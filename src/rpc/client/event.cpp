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
 * @file json/rpc/client/event.cpp
 *
 * @brief JSON client message interface
 * */

#include <json/rpc/client/event.hpp>

#include <json/rpc/client/call_method.hpp>
#include <json/rpc/client/destroy_context.hpp>
#include <json/rpc/client/send_notification.hpp>

using json::rpc::Error;
using json::rpc::client::Event;
using json::rpc::client::EventDeleter;
using json::rpc::client::CallMethod;
using json::rpc::client::DestroyContext;
using json::rpc::client::SendNotification;

Event::Event(EventType type, Client* client, Miliseconds time_live_ms) :
    m_type(type), m_client(client)
{
    if (0_ms != time_live_ms) {
        m_time_live = std::chrono::steady_clock::now() + time_live_ms;
    }
}

Event::~Event() { }

static inline
void call_method(Event* _event, const Error& error) {
    CallMethod* event = static_cast<CallMethod*>(_event);
    if (!error) {
        event->m_result.set_value(event->m_value);
    }
    else {
        event->m_result.set_exception(std::make_exception_ptr(error));
    }
}

static inline
void call_method_async(const Event* _event, const Error& error) {
    const CallMethod* event = static_cast<const CallMethod*>(_event);
    if (nullptr == event->m_callback) { return; }
    if (!error) {
        event->m_callback(event->m_value, Error::OK);
    }
    else {
        event->m_callback(json::Value::Type::NIL, error);
    }
}

static inline
void send_notification(Event* _event, const Error& error) {
    SendNotification* event = static_cast<SendNotification*>(_event);
    if (!error) {
        event->m_result.set_value();
    }
    else {
        event->m_result.set_exception(std::make_exception_ptr(error));
    }
}

static inline
void send_notification_async(const Event* _event, const Error& error) {
    const SendNotification* event = static_cast<const SendNotification*>(_event);
    if (nullptr == event->m_callback) { return; }
    if (!error) {
        event->m_callback(Error::OK);
    }
    else {
        event->m_callback(error);
    }
}

static inline
void destroy_context(Event* _event, const Error& error) {
    DestroyContext* event = static_cast<DestroyContext*>(_event);
    if (!error) {
        event->m_result.set_value();
    }
    else {
        event->m_result.set_exception(std::make_exception_ptr(error));
    }
}

void Event::complete(const Error& error) {
    m_completed = true;

    switch (m_type) {
    case EventType::CALL_METHOD:
        call_method(this, error);
        break;
    case EventType::CALL_METHOD_ASYNC:
        call_method_async(this, error);
        break;
    case EventType::SEND_NOTIFICATION:
        send_notification(this, error);
        break;
    case EventType::SEND_NOTIFICATION_ASYNC:
        send_notification(this, error);
        break;
    case EventType::DESTROY_CONTEXT:
        destroy_context(this, error);
        break;
    case EventType::CREATE_CONTEXT:
    case EventType::UNDEFINED:
    default:
        break;
    }
}

void EventDeleter::operator()(Event* event) {
    if (!event->is_complete()) {
        event->complete(Error{Error::INTERNAL_ERROR,
                "Event deleted without completetion"});
    }
    delete event;
}
