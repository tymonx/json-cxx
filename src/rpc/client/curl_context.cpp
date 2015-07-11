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
 * @file json/rpc/client/curl_context.cpp
 *
 * @brief HTTP JSON RPC client context implementation for proactor based on
 *        CURL library
 * */

#include <json/rpc/client/curl_context.hpp>
#include <json/rpc/client/curl_proactor.hpp>
#include <json/rpc/client/http_settings.hpp>
#include <json/rpc/client/http_client.hpp>

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
#include <json/rpc/client/message/set_http_settings.hpp>

#include <curl/curl.h>
#include <algorithm>

using namespace json::rpc::client;
using namespace json::rpc::client::message;

using json::rpc::Error;
using json::rpc::client::CurlContext;
using json::rpc::time::operator "" _ms;
using json::rpc::time::TimePoint;

void CurlContext::CurlSlistDeleter::operator()(struct curl_slist* curl_slist) {
    curl_slist_free_all(curl_slist);
}

void CurlContext::CurlEasyDeleter::operator()(void* curl_easy) {
    curl_easy_cleanup(curl_easy);
}

CurlContext::CurlContext(HttpClient* client, void* curl_multi) :
    m_client{client}, m_curl_multi{curl_multi} {
    m_headers.reset(curl_slist_append(m_headers.release(),
            "Content-Type: application/json"));
    m_headers.reset(curl_slist_append(m_headers.release(),
            "charset: utf-8"));
}

CurlContext::~CurlContext() { }

void CurlContext::splice_message(MessageList& other,
        MessageList::const_iterator it) {
    if ((0_ms != m_time_live_ms)
            && (TimePoint(0_ms) == (*it)->get_time_live())) {
        (*it)->set_time_live(m_time_live_ms);
    }

    m_messages.splice(m_messages.end(), other, it);
}

void CurlContext::info_read(CurlContext* context, InfoReadPtr info_read,
        unsigned curl_code) {
    if (CURLE_OK == curl_code) {
        context->m_executor.execute(std::move(info_read->message));
    }
    else if (CURLE_OPERATION_TIMEDOUT == curl_code) {
        context->m_executor.execute(std::move(info_read->message),
                    {Error::SERVER_ERROR});
    }
    else {
        context->m_messages.push_back(std::move(info_read->message));
    }
    --context->m_requests;
}

size_t CurlContext::write_function(char* buffer, size_t size, size_t nmemb,
        void* userdata) {
    InfoRead* info = static_cast<InfoRead*>(userdata);
    size_t copy = size * nmemb;
    info->response->append(buffer, copy);
    return copy;
}

size_t CurlContext::read_function(char* buffer, size_t size, size_t nmemb,
        void* userdata) {
    InfoRead* info = static_cast<InfoRead*>(userdata);
    std::string::size_type copied;

    if (!size || !nmemb || (info->request_pos >= info->request.size())) {
        return 0;
    }

    copied = info->request.copy(buffer, size * nmemb, info->request_pos);
    info->request_pos += copied;

    return copied;
}

bool CurlContext::message_expired(MessageList::iterator& it) {
    if (TimePoint(0_ms) != (*it)->get_time_live()) {
        if (std::chrono::steady_clock::now() > (*it)->get_time_live()) {
            m_executor.execute(std::move(*it), {Error::INTERNAL_ERROR});
            it = m_messages.erase(it);
            return true;
        }
    }
    return false;
}

static json::Value build_notification(const std::string& method,
        const json::Value& params) {
    json::Value message(json::Value::Type::OBJECT);
    message["jsonrpc"] = "2.0";
    message["method"] = method;
    if (params.is_object() || params.is_array()) {
        message["params"] = params;
    }
    else if (!params.is_null()) {
        message["params"].push_back(params);
    }
    return message;
}

static json::Value build_method(const std::string& method,
        const json::Value& params, const json::Value& id) {
    json::Value message = build_notification(method, params);
    message["id"] = id;
    return message;
}

void CurlContext::add_request(MessagePtr&& message, std::string&& request,
        std::string* response) {
    try {
        InfoReadPtr info{new InfoRead{}};
        CurlEasyPtr curl_easy{curl_easy_init()};
        if (nullptr == curl_easy) { throw std::bad_alloc(); }

        info->context = this;
        info->callback = &CurlContext::info_read;
        info->message = std::move(message);
        info->request = std::move(request);
        info->response = response;
        response->clear();

        curl_easy_setopt(curl_easy.get(),
                CURLOPT_URL, m_url.c_str());
        curl_easy_setopt(curl_easy.get(),
                CURLOPT_WRITEFUNCTION, write_function);
        curl_easy_setopt(curl_easy.get(),
                CURLOPT_WRITEDATA, info.get());
        curl_easy_setopt(curl_easy.get(),
                CURLOPT_READFUNCTION, read_function);
        curl_easy_setopt(curl_easy.get(),
                CURLOPT_READDATA, info.get());
        curl_easy_setopt(curl_easy.get(),
                CURLOPT_POSTFIELDS, nullptr);
        curl_easy_setopt(curl_easy.get(),
                CURLOPT_POSTFIELDSIZE, info->request.size());
        curl_easy_setopt(curl_easy.get(),
                CURLOPT_HTTPHEADER, m_headers.get());
        curl_easy_setopt(curl_easy.get(),
                CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
        curl_easy_setopt(curl_easy.get(),
                CURLOPT_TIMEOUT_MS, m_timeout_ms.count());
        curl_easy_setopt(curl_easy.get(),
                CURLOPT_PRIVATE, info.release());
        curl_multi_add_handle(m_curl_multi, curl_easy.release());
        ++m_requests;
    }
    catch (...) {
        m_executor.execute(std::move(message), {Error::INTERNAL_ERROR});
    }
}

void CurlContext::dispatch_messages() {
    for (auto it = m_messages.begin(); it != m_messages.end();) {
        if (message_expired(it)) { continue; }

        switch ((*it)->get_type()) {
        case MessageType::CALL_METHOD_SYNC:
            call_method_sync(it);
            break;
        case MessageType::CALL_METHOD_ASYNC:
            call_method_async(it);
            break;
        case MessageType::SEND_NOTIFICATION_SYNC:
            send_notification_sync(it);
            break;
        case MessageType::SEND_NOTIFICATION_ASYNC:
            send_notification_async(it);
            break;
        case MessageType::CONNECT:
            connect(it);
            break;
        case MessageType::DISCONNECT:
            disconnect(it);
            break;
        case MessageType::SET_ID_BUILDER:
            set_id_builder(it);
            break;
        case MessageType::SET_HTTP_SETTINGS:
            set_http_settings(it);
            break;
        case MessageType::SET_ERROR_TO_EXCEPTION:
            set_error_to_exception(it);
            break;
        case MessageType::CREATE_CONTEXT:
        case MessageType::DESTROY_CONTEXT:
        case MessageType::UNDEFINED:
        default:
            it = m_messages.erase(it);
            break;
        }
    }
}

void CurlContext::call_method_sync(MessageList::iterator& it) {
    if (!m_is_connected) { ++it; return; }
    auto& message = static_cast<CallMethodSync&>(**it);

    if (nullptr != m_id_builder) {
        message.set_id(m_id_builder(++m_id));
    }
    else {
        message.set_id(++m_id);
    }

    std::string request{};
    request << build_method(message.get_name(), message.get_params(),
            message.get_id());
    add_request(std::move(*it), std::move(request), &message.get_response());
    it = m_messages.erase(it);
}

void CurlContext::call_method_async(MessageList::iterator& it) {
    if (!m_is_connected) { ++it; return; }
    auto& message = static_cast<CallMethodAsync&>(**it);

    if (nullptr != m_id_builder) {
        message.set_id(m_id_builder(++m_id));
    }
    else {
        message.set_id(++m_id);
    }

    std::string request{};
    request << build_method(message.get_name(), message.get_params(),
            message.get_id());
    add_request(std::move(*it), std::move(request), &message.get_response());
    it = m_messages.erase(it);
}

void CurlContext::send_notification_sync(MessageList::iterator& it) {
    if (!m_is_connected) { ++it; return; }
    auto& message = static_cast<SendNotificationSync&>(**it);

    std::string request{};
    request << build_notification(message.get_name(), message.get_params());
    add_request(std::move(*it), std::move(request), &message.get_response());
    it = m_messages.erase(it);
}

void CurlContext::send_notification_async(MessageList::iterator& it) {
    if (!m_is_connected) { ++it; return; }
    auto& message = static_cast<SendNotificationAsync&>(**it);

    std::string request{};
    request << build_notification(message.get_name(), message.get_params());
    add_request(std::move(*it), std::move(request), &message.get_response());
    it = m_messages.erase(it);
}

void CurlContext::connect(MessageList::iterator& it) {
    m_is_connected = true;
    it = m_messages.erase(it);
}

void CurlContext::disconnect(MessageList::iterator& it) {
    m_is_connected = false;
    it = m_messages.erase(it);
}

void CurlContext::set_error_to_exception(MessageList::iterator& it) {
    const auto& message = static_cast<const SetErrorToException&>(**it);
    m_executor.set_error_to_exception(message.get_callback());
    it = m_messages.erase(it);
}

void CurlContext::set_http_settings(MessageList::iterator& it) {
    const auto& message = static_cast<const SetHttpSettings&>(**it);
    const auto& settings = message.get_http_settings();

    if (!settings.get_headers().empty()) {
        std::string header;
        for (const auto& header_pair : settings.get_headers()) {
            header = header_pair.first + ": " + header_pair.second;
            m_headers.reset(curl_slist_append(m_headers.release(),
                        header.c_str()));
        }
    }

    if (!settings.get_url().empty()) {
        m_url = settings.get_url();
    }

    if (HttpSettings::UNKNOWN_TIMEOUT_MS != settings.get_timeout()) {
        m_timeout_ms = settings.get_timeout();
    }

    if (HttpSettings::UNKNOWN_TIME_LIVE_MS != settings.get_time_live()) {
        m_time_live_ms = settings.get_time_live();
    }

    it = m_messages.erase(it);
}

void CurlContext::set_id_builder(MessageList::iterator& it) {
    const auto& message = static_cast<const SetIdBuilder&>(**it);
    m_id_builder = message.get_callback();
    it = m_messages.erase(it);
}
