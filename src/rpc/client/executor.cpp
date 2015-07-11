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
#include <thread>

#include <json/rpc/client/message/call_method_sync.hpp>
#include <json/rpc/client/message/call_method_async.hpp>
#include <json/rpc/client/message/send_notification_sync.hpp>
#include <json/rpc/client/message/send_notification_async.hpp>

using namespace json::rpc::client::message;

using json::Value;
using json::Deserializer;
using json::rpc::Error;
using json::rpc::client::Executor;

Executor::~Executor() {
    while (m_tasks.load() > 0) { }
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

void Executor::call_method_sync(MessagePtr& message, const Error& error) {
    auto& msg = static_cast<CallMethodSync&>(*message);
    Error err{error};
    Value result(Value::Type::NIL);

    if (!err) {
        err = processing_method(msg.get_response(), msg.get_id(), result);
    }

    if (!err) {
        msg.set_result(result);
    }
    else {
        if (m_error_to_exception) {
            msg.set_exception(m_error_to_exception(err));
        }
        else {
            msg.set_exception(std::make_exception_ptr(err));
        }
    }
}

void Executor::call_method_async(MessagePtr&& message, Error error) {
    auto& msg = static_cast<CallMethodAsync&>(*message);
    Value result(Value::Type::NIL);

    if (!error) {
        error = processing_method(msg.get_response(), msg.get_id(), result);
    }

    const auto& call = msg.get_callback();
    try {
        if (nullptr != call) {
            call(msg.get_client(), result, error);
        }
    }
    catch (...) { }
    --m_tasks;
}

void Executor::send_notification_sync(MessagePtr& message, const Error& error) {
    auto& msg = static_cast<SendNotificationSync&>(*message);
    Error err{error};

    if (!err) {
        err = processing_notification(msg.get_response());
    }

    if (!err) {
        msg.set_result();
    }
    else {
        if (m_error_to_exception) {
            msg.set_exception(m_error_to_exception(err));
        }
        else {
            msg.set_exception(std::make_exception_ptr(err));
        }
    }
}

void Executor::send_notification_async(MessagePtr&& message, Error error) {
    auto& msg = static_cast<SendNotificationAsync&>(*message);
    if (!error) {
        error = processing_notification(msg.get_response());
    }

    const auto& call = msg.get_callback();
    try {
        if (nullptr != call) {
            call(msg.get_client(), error);
        }
    }
    catch (...) { }
    --m_tasks;
}

void Executor::execute(MessagePtr&& message, const Error& error) {
    switch (message->get_type()) {
    case MessageType::CALL_METHOD_SYNC:
        call_method_sync(message, error);
        break;
    case MessageType::CALL_METHOD_ASYNC:
        try {
            ++m_tasks;
            std::thread t{&Executor::call_method_async, this,
                 std::move(message), error};
            t.detach();
        }
        catch (...) {
            --m_tasks;
        }
        break;
    case MessageType::SEND_NOTIFICATION_SYNC:
        send_notification_sync(message, error);
        break;
    case MessageType::SEND_NOTIFICATION_ASYNC:
        try {
            ++m_tasks;
            std::thread t{&Executor::send_notification_async, this,
                std::move(message), error};
            t.detach();
        }
        catch (...) {
            --m_tasks;
        }
        break;
    case MessageType::CONNECT:
    case MessageType::DISCONNECT:
    case MessageType::SET_ID_BUILDER:
    case MessageType::SET_HTTP_SETTINGS:
    case MessageType::SET_ERROR_TO_EXCEPTION:
    case MessageType::CREATE_CONTEXT:
    case MessageType::DESTROY_CONTEXT:
    case MessageType::UNDEFINED:
    default:
        break;
    }
}
