/*!
 * @copyright
 * Copyright (c) 2015, Tymoteusz Blazejczyk
 *
 * @copyright
 * All rights reserved.
 *
 * @copyright
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * @copyright
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * @copyright
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * @copyright
 * * Neither the name of json-cxx nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * @copyright
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file json/rpc/client/executor.cpp
 *
 * @brief JSON RPC client executor implementation
 * */

#include <json/rpc/client/executor.hpp>

#include <json/json.hpp>

#include <json/rpc/client/message/call_method_sync.hpp>
#include <json/rpc/client/message/call_method_async.hpp>
#include <json/rpc/client/message/send_notification_sync.hpp>
#include <json/rpc/client/message/send_notification_async.hpp>
#include <json/rpc/client/message/set_error_to_exception.hpp>

using namespace json::rpc::client::message;

using json::Value;
using json::Deserializer;
using json::rpc::Error;
using json::rpc::client::Executor;

Executor::Executor(size_t thread_pool_size) {
    m_thread_pool.resize(thread_pool_size);
    for (auto it = m_thread_pool.begin(); it != m_thread_pool.end(); ++it) {
        *it = std::thread{&Executor::task, this};
    }
}

Executor::~Executor() {
    m_stop = true;
    m_cond_variable.notify_all();
    for (auto it = m_thread_pool.begin(); it != m_thread_pool.end(); ++it) {
        if (it->joinable()) { it->join(); }
    }

    while (!m_messages.empty()) {
        auto& message = m_messages.front();
        message_processing(message.first, message.second);
        m_messages.pop();
    }
}

void Executor::execute(MessagePtr&& message, const Error& error) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_messages.emplace(std::move(message), error);
    lock.unlock();
    m_cond_variable.notify_all();
}

void Executor::resize(size_t size) {
    if (0 == size) { size = 1; }

    m_stop = true;
    m_cond_variable.notify_all();
    for (auto it = m_thread_pool.begin(); it != m_thread_pool.end(); ++it) {
        if (it->joinable()) { it->join(); }
    }

    m_stop = false;
    m_thread_pool.resize(size);
    for (auto it = m_thread_pool.begin(); it != m_thread_pool.end(); ++it) {
        *it = std::thread{&Executor::task, this};
    }
}

static bool
valid_response(const Value& value, const Value& id) {
    if (!value.is_object()) { return false; }
    if (3 != value.size()) { return false; }
    if (value["jsonrpc"] != "2.0") { return false; }
    if (!value.is_member("id")) { return false; }

    const auto& vid = value["id"];
    if (!vid.is_number() && !vid.is_string() && !vid.is_null()) {
        return false;
    }

    if (value.is_member("result")) {
        if (vid != id) { return false; }
    }
    else if (value.is_member("error")) {
        auto& error = value["error"];
        if (!error.is_object()) { return false; }
        if (!error["code"].is_int()) { return false; }
        if (!error["message"].is_string()) { return false; }
        if (3 == error.size()) {
            if (!error.is_member("data")) { return false; }
        }
        else if (2 != error.size()) { return false; }
        if ((vid != id) && !vid.is_null()) { return false; }
    }
    else { return false; }

    return true;
}

static Error
processing_method(const std::string& response, const Value& id, Value& result) {
    Value value;
    Deserializer deserializer(response);
    if (deserializer.is_invalid()) {
        return {Error::PARSE_ERROR};
    }
    deserializer >> value;
    if (!valid_response(value, id)) {
        return {Error::PARSE_ERROR};
    }
    if (value.is_member("result")) {
        result = value["result"];
    }
    else {
        return {
            value["error"]["code"].as_int(),
            value["error"]["message"].as_string(),
            value["error"]["data"]
        };
    }

    return {Error::OK};
}

static Error
processing_notification(const std::string& response) {
    if (!response.empty()) { return {Error::INTERNAL_ERROR}; }
    return {Error::OK};
}

void Executor::call_method_sync(MessagePtr& message, Error& error) {
    auto& msg = static_cast<CallMethodSync&>(*message);
    Value result(Value::Type::NIL);

    if (!error) {
        error = processing_method(msg.get_response(), msg.get_id(), result);
    }

    if (!error) {
        msg.set_result(result);
    }
    else {
        std::unique_lock<std::mutex> lock(m_mutex);
        ErrorToException callback = m_error_to_exception;
        lock.unlock();

        if (callback) {
            msg.set_exception(callback(error));
        }
        else {
            msg.set_exception(std::make_exception_ptr(error));
        }
    }
}

void Executor::call_method_async(MessagePtr& message, Error& error) {
    auto& msg = static_cast<CallMethodAsync&>(*message);
    if (nullptr == msg.get_callback()) { return; }
    Value result(Value::Type::NIL);

    if (!error) {
        error = processing_method(msg.get_response(), msg.get_id(), result);
    }

    try {
        msg.get_callback()(msg.get_client(), result, error);
    }
    catch (...) { }
}

void Executor::send_notification_sync(MessagePtr& message, Error& error) {
    auto& msg = static_cast<SendNotificationSync&>(*message);
    if (!error) {
        error = processing_notification(msg.get_response());
    }

    if (!error) {
        msg.set_result();
    }
    else {
        std::unique_lock<std::mutex> lock(m_mutex);
        ErrorToException callback = m_error_to_exception;
        lock.unlock();

        if (callback) {
            msg.set_exception(callback(error));
        }
        else {
            msg.set_exception(std::make_exception_ptr(error));
        }
    }
}

void Executor::send_notification_async(MessagePtr& message, Error& error) {
    auto& msg = static_cast<SendNotificationAsync&>(*message);
    if (nullptr == msg.get_callback()) { return; }
    if (!error) {
        error = processing_notification(msg.get_response());
    }

    try {
        msg.get_callback()(msg.get_client(), error);
    }
    catch (...) { }
}

void Executor::task() {
    std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
    Message message{};

    do {
        message.first = nullptr;
        lock.lock();
        m_cond_variable.wait(lock, [this] {
            return !m_messages.empty() || m_stop;
        });
        if (!m_messages.empty()) {
            message = std::move(m_messages.front());
            m_messages.pop();
        }
        lock.unlock();

        if (nullptr != message.first) {
            message_processing(message.first, message.second);
        }
    } while (!m_stop);
}

void Executor::set_error_to_exception(MessagePtr& message, Error&) {
    auto& msg = static_cast<SetErrorToException&>(*message);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_error_to_exception = msg.get_callback();
}

void Executor::message_processing(MessagePtr& message, Error& error) {
    switch (message->get_type()) {
    case MessageType::CALL_METHOD_SYNC:
        call_method_sync(message, error);
        break;
    case MessageType::CALL_METHOD_ASYNC:
        call_method_async(message, error);
        break;
    case MessageType::SEND_NOTIFICATION_SYNC:
        send_notification_sync(message, error);
        break;
    case MessageType::SEND_NOTIFICATION_ASYNC:
        send_notification_async(message, error);
        break;
    case MessageType::SET_ERROR_TO_EXCEPTION:
        set_error_to_exception(message, error);
        break;
    case MessageType::CONNECT:
    case MessageType::DISCONNECT:
    case MessageType::SET_ID_BUILDER:
    case MessageType::SET_HTTP_SETTINGS:
    case MessageType::CREATE_CONTEXT:
    case MessageType::DESTROY_CONTEXT:
    case MessageType::UNDEFINED:
    default:
        break;
    }
}
