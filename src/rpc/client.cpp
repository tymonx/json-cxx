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
 * @file json/rpc/client.cpp
 *
 * @brief JSON client implementation
 * */

#include <json/rpc/client.hpp>

#include <json/rpc/client/proactor.hpp>
#include <json/rpc/client/protocol.hpp>
#include <json/rpc/client/event/context.hpp>
#include <json/rpc/client/event/call_method.hpp>
#include <json/rpc/client/event/send_notification.hpp>

using namespace json::rpc;
using namespace json::rpc::client;

Client::Client(const Protocol& protocol)
    : m_proactor{Proactor::get_instance()}
{
    m_proactor.push_event(new event::Context(this, protocol));
}

Client::~Client() {
    m_proactor.push_event(new event::DestroyContext(this));
}

void Client::method(const std::string& name, const json::Value& params,
        ResultCallback result)
{
    m_proactor.push_event(
            new event::CallMethodAsync(this, name, params, result));
}

json::Value Client::method(const std::string& name, const json::Value& params) {
    event::CallMethod event(this, name, params);
    auto notify = event.get_notify();
    m_proactor.push_event(&event);
    notify.get();
    return event.m_value;
}

void Client::notification(const std::string& name, const json::Value& params) {
    m_proactor.push_event(new event::SendNotification(this, name, params));
}
