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
 * @brief JSON client protocol IPv4 protocol
 * */

#include <json/rpc/client/executor.hpp>

#include <json/json.hpp>

#include <json/rpc/client/request.hpp>
#include <json/rpc/client/call_method.hpp>
#include <json/rpc/client/destroy_context.hpp>
#include <json/rpc/client/send_notification.hpp>

#include <iostream>

using namespace std;

using json::Value;
using json::Deserializer;
using json::rpc::Error;
using json::rpc::client::Event;
using json::rpc::client::Request;
using json::rpc::client::Executor;
using json::rpc::client::CallMethod;
using json::rpc::client::DestroyContext;
using json::rpc::client::SendNotification;

Executor::Executor() : m_thread{&Executor::task, this} { }

Executor::~Executor() {
    m_stop = true;
    m_cond_variable.notify_one();
    m_thread.join();
    /* Finish all tasks */
    task();
}

void Executor::task() {
    std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);

    do {
        lock.lock();
        m_cond_variable.wait(lock,
            [this] { return !m_events_background.empty() || m_stop; });
        m_events.splice(m_events.end(), m_events_background);
        lock.unlock();

        while (!m_events.empty()) {
            event_dispatcher(std::move(m_events.front()));
            m_events.pop_front();
        }
    } while(!m_stop);
}

void Executor::push_event(EventPtr event) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_events_background.push_back(std::move(event));
    lock.unlock();
    m_cond_variable.notify_one();
}

static Error check_response(const Value& value) {
    if (!value.is_object()) {
        return {Error::PARSE_ERROR, "Invalid JSON object"};
    }
    if (3 != value.size()) {
        return {Error::PARSE_ERROR, "Invalid params number"};
    }
    if (value["jsonrpc"] != "2.0") {
        return {Error::PARSE_ERROR, "Invalid/missing 'jsonrpc'"};
    }
    if (!value.is_member("id")) {
        return {Error::PARSE_ERROR, "Missing 'id'"};
    }
    else {
        if (!value["id"].is_number() && !value["id"].is_string()
         && !value["id"].is_null()) {
            return {Error::PARSE_ERROR, "Invalid 'id'"};
        }
    }

    if (value.is_member("result")) {
        if (value["id"].is_null()) {
            return {Error::PARSE_ERROR, "Invalid 'id'"};
        }
    }
    else if (value.is_member("error")) {
        auto& error = value["error"];
        if (!error.is_object()) {
            return {Error::PARSE_ERROR, "Invalid JSON object 'error'"};
        }
        if (!error["code"].is_int()) {
            return {Error::PARSE_ERROR, "Invalid/missing 'code' in 'error'"};
        }
        if (!error["message"].is_string()) {
            return {Error::PARSE_ERROR, "Invalid/missing 'message' in 'error'"};
        }
        if (2 == error.size()) {
            /* Do nothing */
        }
        else if (3 == error.size()) {
            if (!error.is_member("data")) {
                return {Error::PARSE_ERROR, "Missing 'data' member in 'error'"};
            }
        }
        else {
            return {Error::PARSE_ERROR, "Invalid params number in 'error'"};
        }
    }
    else {
        return {Error::PARSE_ERROR, "Missing 'result' or 'error'"};
    }
    return {Error::OK};
}

static
void call_method_response(Request* request) {
    Value value;
    Deserializer deserializer(request->get_response());
    if (deserializer.is_invalid()) {
        auto error = deserializer.get_error();
        return request->set_error({Error::PARSE_ERROR, "Invalid response: \'" +
                request->get_response() + "\' " + error.decode() + " at " +
                std::to_string(error.offset)});
    }
    deserializer >> value;
    if (auto error = check_response(value)) {
        return request->set_error({Error::PARSE_ERROR, "Invalid response: \'" +
                request->get_response() + "\' " + error.what()});
    }
    if (value.is_member("result")) {
        request->m_value = value["result"];
    }
    else {
        request->set_error({
            value["error"]["code"].as_int(),
            value["error"]["message"].as_string(),
            value["error"]["data"]
        });
    }
}

static
void send_notification_response(Request* request) {
    if (0 != request->get_response().size()) {
        request->set_error({Error::PARSE_ERROR, "Notification response: \'" +
            request->get_response() + "\'"});
    }
}

static
void call_method(Event* _event) {
    CallMethod* event = static_cast<CallMethod*>(_event);
    if (!event->get_error()) {
        call_method_response(static_cast<Request*>(event));
    }

    if (!event->get_error()) {
        event->m_result.set_value(event->m_value);
    }
    else {
        event->m_result.set_exception(
                std::make_exception_ptr(event->get_error()));
    }
}

static
void call_method_async(Event* _event) {
    CallMethod* event = static_cast<CallMethod*>(_event);
    if (nullptr != event->m_callback) {
        call_method_response(static_cast<Request*>(event));
        try {
            event->m_callback(event->m_value, event->get_error());
        }
        catch (...) { }
    }
}

static
void send_notification(Event* _event) {
    SendNotification* event = static_cast<SendNotification*>(_event);
    if (!event->get_error()) {
        send_notification_response(static_cast<Request*>(event));
    }

    if (!event->get_error()) {
        event->m_result.set_value();
    }
    else {
        event->m_result.set_exception(
                std::make_exception_ptr(event->get_error()));
    }
}

static
void send_notification_async(Event* _event) {
    SendNotification* event = static_cast<SendNotification*>(_event);
    if (nullptr != event->m_callback) {
        send_notification_response(static_cast<Request*>(event));
        try {
            event->m_callback(event->get_error());
        }
        catch (...) { }
    }
}

static
void destroy_context(Event* _event) {
    DestroyContext* event = static_cast<DestroyContext*>(_event);
    if (!event->get_error()) {
        event->m_result.set_value();
    }
    else {
        event->m_result.set_exception(
                std::make_exception_ptr(event->get_error()));
    }
}

void Executor::event_dispatcher(EventPtr event) {
    switch (event->get_type()) {
    case EventType::CALL_METHOD:
        call_method(event.get());
        break;
    case EventType::CALL_METHOD_ASYNC:
        call_method_async(event.get());
        break;
    case EventType::SEND_NOTIFICATION:
        send_notification(event.get());
        break;
    case EventType::SEND_NOTIFICATION_ASYNC:
        send_notification_async(event.get());
        break;
    case EventType::DESTROY_CONTEXT:
        destroy_context(event.get());
        break;
    case EventType::CREATE_CONTEXT:
    case EventType::UNDEFINED:
    default:
        break;
    }
}
