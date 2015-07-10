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
 * @brief JSON RPC client implementation
 * */

#include <json/rpc/client.hpp>

#include <json/rpc/client/proactor.hpp>

#include <json/rpc/client/message/call_method_sync.hpp>
#include <json/rpc/client/message/call_method_async.hpp>
#include <json/rpc/client/message/send_notification_sync.hpp>
#include <json/rpc/client/message/send_notification_async.hpp>
#include <json/rpc/client/message/create_context.hpp>
#include <json/rpc/client/message/destroy_context.hpp>
#include <json/rpc/client/message/connect.hpp>
#include <json/rpc/client/message/disconnect.hpp>
#include <json/rpc/client/message/set_id_builder.hpp>
#include <json/rpc/client/message/set_error_to_exception.hpp>

using namespace json::rpc;
using namespace json::rpc::client;
using namespace json::rpc::client::message;

Client::Client(client::Proactor& proactor) : m_id{this}, m_proactor{proactor} {
    m_proactor.push_event(MessagePtr{new CreateContext{m_id}});
}

Client::~Client() {
    MessagePtr event{new DestroyContext{m_id}};
    auto result = static_cast<DestroyContext&>(*event).get_result();
    m_proactor.push_event(std::move(event));
    result.get();
}

std::future<void> Client::connect() {
    MessagePtr event{new Connect{m_id}};
    auto result = static_cast<Connect&>(*event).get_result();
    m_proactor.push_event(std::move(event));
    return result;
}

std::future<void> Client::disconnect() {
    MessagePtr event{new Disconnect{m_id}};
    auto result = static_cast<Disconnect&>(*event).get_result();
    m_proactor.push_event(std::move(event));
    return result;
}

void Client::set_id_builder(const IdBuilder& id_builder) {
    m_proactor.push_event(MessagePtr{new SetIdBuilder{m_id, id_builder}});
}

void Client::set_error_to_exception(const ErrorToException& error_to_exception) {
    m_proactor.push_event(MessagePtr{new SetErrorToException{m_id,
            error_to_exception}});
}

Client::MethodFuture Client::method(const std::string& name,
        const json::Value& params)
{
    MessagePtr event{new CallMethodSync{m_id, name, params}};
    auto result = static_cast<CallMethodSync&>(*event).get_result();
    m_proactor.push_event(std::move(event));
    return result;
}

void Client::method(const std::string& name, const json::Value& params,
        MethodCallback result)
{
    m_proactor.push_event(MessagePtr{new CallMethodAsync{m_id,
                name, params, result}});
}

Client::NotificationFuture Client::notification(const std::string& name,
        const json::Value& params)
{
    MessagePtr event{new SendNotificationSync{m_id, name, params}};
    auto result = static_cast<SendNotificationSync&>(*event).get_result();
    m_proactor.push_event(std::move(event));
    return result;
}

void Client::notification(const std::string& name, const json::Value& params,
        NotificationCallback result)
{
    m_proactor.push_event(MessagePtr{new SendNotificationAsync{m_id,
            name, params, result}});
}
