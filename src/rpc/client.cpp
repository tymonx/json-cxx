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

#include <json/rpc/client/http_context.hpp>
#include <json/rpc/client/http_protocol.hpp>
#include <json/rpc/client/http_proactor.hpp>

#include <json/rpc/client/call_method.hpp>
#include <json/rpc/client/call_method_async.hpp>
#include <json/rpc/client/send_notification.hpp>
#include <json/rpc/client/destroy_context.hpp>

#include <json/rpc/client/call_method.hpp>
#include <json/rpc/client/send_notification.hpp>

using namespace json::rpc;
using namespace json::rpc::client;

Client::Client(const Protocol& protocol) : m_id{this} {
    switch (protocol.get_type()) {
    case ProtocolType::HTTP:
        m_proactor = &HttpProactor::get_instance();
        m_proactor->push_event(new HttpContext(m_id,
            static_cast<HttpProactor&>(*m_proactor),
            static_cast<const HttpProtocol&>(protocol)));
        break;
    case ProtocolType::SERIAL:
    case ProtocolType::UDP:
    case ProtocolType::UNDEFINED:
    default:
        break;
    }
}

Client::~Client() {
    m_proactor->push_event(new DestroyContext(m_id));
}

void Client::method(const std::string& name, const json::Value& params,
        ResultCallback result)
{
    m_proactor->push_event(new CallMethodAsync(m_id, name, params, result));
}

Client::ResultFuture Client::method(const std::string& name,
        const json::Value& params)
{
    auto event = new CallMethod(m_id, name, params);
    auto result = event->m_result.get_future();
    m_proactor->push_event(event);
    return result;
}

void Client::notification(const std::string& name, const json::Value& params) {
    m_proactor->push_event(new SendNotification(this, name, params));
}
