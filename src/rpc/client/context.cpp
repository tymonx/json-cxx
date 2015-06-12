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
 * @file json/rpc/client/event/context.cpp
 *
 * @brief JSON client protocol IPv4 protocol
 * */

#include <json/rpc/client/context.hpp>

#include <curl/curl.h>
#include <iostream>

using namespace json::rpc::client;
using json::rpc::client::event::Context;
using json::rpc::client::event::DestroyContext;

Context::Context(Client* client, const Protocol& protocol) :
    Event(EventType::CONTEXT, client, AUTO_REMOVE),
    m_protocol_type{protocol.get_type()}
{
    switch (m_protocol_type) {
    case ProtocolType::IPv4:
        new (&m_ipv4) protocol::IPv4(static_cast<const protocol::IPv4&>(protocol));
        break;
    case ProtocolType::IPv6:
    case ProtocolType::UDP:
    case ProtocolType::SERIAL:
    case ProtocolType::UNDEFINED:
    default:
        break;
    }
}

Context::~Context() {
    switch (m_protocol_type) {
    case ProtocolType::IPv4:
        m_ipv4.~IPv4();
        break;
    case ProtocolType::IPv6:
    case ProtocolType::UDP:
    case ProtocolType::SERIAL:
    case ProtocolType::UNDEFINED:
    default:
        break;
    }

    while (!m_events.empty()) {
        Event::event_complete(static_cast<Event*>(m_events.pop()),
                Error{Error::INTERNAL_ERROR, "Client context destroyed"});
    }
}

void Context::dispatch_event(Event* event) {
    switch (event->get_type()) {
    case EventType::CALL_METHOD:
    case EventType::CALL_METHOD_ASYNC:
    case EventType::SEND_NOTIFICATION:
        event->context = curl_easy_init();
        if (nullptr == event->context) {
            return Event::event_complete(event, Error{Error::INTERNAL_ERROR,
                    "Cannot create context"});
        }
        curl_easy_setopt(event->context, CURLOPT_URL, m_ipv4.get_address().c_str());
        curl_easy_setopt(event->context, CURLOPT_PORT, m_ipv4.get_port());
        curl_easy_setopt(event->context, CURLOPT_POSTFIELDS, "Dupa!!!");
        curl_multi_add_handle(context, event->context);
        break;
    case EventType::OPEN_CONNECTION:
    case EventType::CLOSE_CONNECTION:
    case EventType::CONTEXT:
    case EventType::DESTROY_CONTEXT:
    case EventType::UNDEFINED:
        break;
    default:
        break;
    }

    Event::event_complete(event);
}
