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
#include <json/rpc/client/event_type.hpp>
#include <json/rpc/client/destroy_context.hpp>
#include <json/rpc/client/call_method.hpp>
#include <json/rpc/client/call_method_async.hpp>

using json::rpc::Error;
using namespace json::rpc::client;

static inline void call_method(Event* _event, const Error& error) {
    CallMethod* event = static_cast<CallMethod*>(_event);
    if (!error) {
        event->m_result.set_value(event->m_value);
    }
    else {
        event->m_result.set_exception(std::make_exception_ptr(error));
    }
}

static inline void call_method_async(const Event* _event, const Error& error) {
    const CallMethodAsync* event = static_cast<const CallMethodAsync*>(_event);
    if (nullptr == event->m_callback) { return; }
    if (!error) {
        event->m_callback(event->m_value, Error::OK);
    }
    else {
        event->m_callback(json::Value::Type::NIL, error);
    }
}

void Event::event_complete(Event* event, const Error& error) {
    switch (event->get_type()) {
    case EventType::CALL_METHOD:
        call_method(event, error);
        break;
    case EventType::CALL_METHOD_ASYNC:
        call_method_async(event, error);
        break;
    case EventType::SEND_NOTIFICATION:
    case EventType::CONTEXT:
    case EventType::DESTROY_CONTEXT:
    case EventType::UNDEFINED:
        break;
    default:
        break;
    }

    if (event->check_flag(Event::AUTO_REMOVE)) { delete event; }
}

Event::~Event() { }
