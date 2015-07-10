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

CurlContext::CurlContext(HttpClient* client, void* curl_multi) :
    m_client{client}, m_curl_multi{curl_multi}
{
    CURL* curl_easy;
    struct ::curl_slist* headers{nullptr};
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charset: utf-8");
    m_headers.reset(headers);

    /* Pipelines */
    m_pipelines.resize(CurlProactor::DEFAULT_MAX_PIPELINE_LENGTH);
    for (auto& pipe : m_pipelines) {
        pipe.curl_easy.reset(curl_easy_init());
        if (nullptr == pipe.curl_easy) {
            throw std::bad_alloc();
        }
        pipe.context = this;
        pipe.callback = &CurlContext::handle_pipe;
        curl_easy = pipe.curl_easy.get();
        curl_easy_setopt(curl_easy, CURLOPT_URL, HttpClient::DEFAULT_URL);
        curl_easy_setopt(curl_easy, CURLOPT_WRITEFUNCTION, write_function);
        curl_easy_setopt(curl_easy, CURLOPT_WRITEDATA, &pipe);
        curl_easy_setopt(curl_easy, CURLOPT_READFUNCTION, read_function);
        curl_easy_setopt(curl_easy, CURLOPT_READDATA, &pipe);
        curl_easy_setopt(curl_easy, CURLOPT_POSTFIELDS, nullptr);
        curl_easy_setopt(curl_easy, CURLOPT_POSTFIELDSIZE, 0);
        curl_easy_setopt(curl_easy, CURLOPT_HTTPHEADER, m_headers.get());
        curl_easy_setopt(curl_easy, CURLOPT_PROTOCOLS,
                CURLPROTO_HTTP | CURLPROTO_HTTPS);
        curl_easy_setopt(curl_easy, CURLOPT_PRIVATE,
                static_cast<InfoRead*>(&pipe));
    }
}

CurlContext::~CurlContext() {
    for (auto it = m_pipelines.begin(); it != m_pipelines.end(); ++it) {
        if (nullptr != it->message) {
            m_executor.execute(it->message, {Error::INTERNAL_ERROR});
            curl_multi_remove_handle(m_curl_multi, it->curl_easy.get());
        }
    }
}

void CurlContext::handle_pipe(struct InfoRead* info_read,
        unsigned curl_code) {
    auto pipe = static_cast<Pipeline*>(info_read);

    if (CURLE_OK == curl_code) {
        m_executor.execute(pipe->message);
    }
    else {
        m_messages.push_back(std::move(pipe->message));
    }

    pipe->message = nullptr;
    curl_multi_remove_handle(m_curl_multi, pipe->curl_easy.get());
    --m_pipes_active;
}

size_t CurlContext::write_function(char* buffer, size_t size, size_t nmemb,
        void* userdata)
{
    Pipeline* pipe = static_cast<Pipeline*>(userdata);
    size_t copy = size * nmemb;
    pipe->response->append(buffer, copy);
    return copy;
}

size_t CurlContext::read_function(char* buffer, size_t size, size_t nmemb,
        void* userdata)
{

    Pipeline* pipe = static_cast<Pipeline*>(userdata);
    std::string::size_type copied;

    if (!size || !nmemb || (pipe->request_pos >= pipe->request.size())) {
        return 0;
    }

    copied = pipe->request.copy(buffer, size * nmemb, pipe->request_pos);
    pipe->request_pos += copied;

    return copied;
}

void CurlContext::CurlEasyDeleter::operator()(void* curl_easy) {
    curl_easy_cleanup(curl_easy);
}

void CurlContext::CurlSlistDeleter::operator()(struct curl_slist* curl_slist) {
    curl_slist_free_all(curl_slist);
}

bool CurlContext::message_expired(MessageList::iterator& it) {
    if ((0_ms != m_time_live_ms)
            && (TimePoint(0_ms) == (*it)->get_time_live())) {
        (*it)->set_time_live(m_time_live_ms);
    }

    if (TimePoint(0_ms) != (*it)->get_time_live()) {
        if (std::chrono::steady_clock::now() > (*it)->get_time_live()) {
            m_executor.execute(*it, {Error::INTERNAL_ERROR});
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

void CurlContext::add_request_to_pipe(MessagePtr&& message,
        std::string&& request, std::string* response) {
    for (auto& pipe : m_pipelines) {
        if (nullptr == pipe.message) {
            pipe.message = std::move(message);
            pipe.request = std::move(request);
            pipe.request_pos = 0;
            pipe.response = response;
            pipe.response->clear();
            curl_easy_setopt(pipe.curl_easy.get(), CURLOPT_POSTFIELDSIZE,
                    pipe.request.size());
            curl_multi_add_handle(m_curl_multi,
                    pipe.curl_easy.get());
            ++m_pipes_active;
            return;
        }
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
    auto& message = static_cast<CallMethodSync&>(**it);

    if ((m_pipes_active < m_pipelines.size()) && m_is_connected) {
        std::string request{};

        if (nullptr != m_id_builder) {
            message.set_id(m_id_builder(++m_id));
        }
        else {
            message.set_id(++m_id);
        }

        request << build_method(message.get_name(), message.get_params(),
                message.get_id());
        add_request_to_pipe(std::move(*it), std::move(request),
                &message.get_response());
        it = m_messages.erase(it);
    }
    else {
        ++it;
    }
}

void CurlContext::call_method_async(MessageList::iterator& it) {
    auto& message = static_cast<CallMethodAsync&>(**it);

    if ((m_pipes_active < m_pipelines.size()) && m_is_connected) {
        std::string request{};

        if (nullptr != m_id_builder) {
            message.set_id(m_id_builder(++m_id));
        }
        else {
            message.set_id(++m_id);
        }

        request << build_method(message.get_name(), message.get_params(),
                message.get_id());
        add_request_to_pipe(std::move(*it), std::move(request),
                &message.get_response());
        it = m_messages.erase(it);
    }
    else {
        ++it;
    }
}

void CurlContext::send_notification_sync(MessageList::iterator& it) {
    auto& message = static_cast<SendNotificationSync&>(**it);

    if ((m_pipes_active < m_pipelines.size()) && m_is_connected) {
        std::string request{};

        request << build_notification(message.get_name(), message.get_params());
        add_request_to_pipe(std::move(*it), std::move(request),
                &message.get_response());
        it = m_messages.erase(it);
    }
    else {
        ++it;
    }
}

void CurlContext::send_notification_async(MessageList::iterator& it) {
    auto& message = static_cast<SendNotificationAsync&>(**it);

    if ((m_pipes_active < m_pipelines.size()) && m_is_connected) {
        std::string request{};

        request << build_notification(message.get_name(), message.get_params());
        add_request_to_pipe(std::move(*it), std::move(request),
                &message.get_response());
        it = m_messages.erase(it);
    }
    else {
        ++it;
    }
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

    CURL* curl_easy;
    for (auto pipe = m_pipelines.begin(); pipe != m_pipelines.end(); ++pipe) {
        curl_easy = pipe->curl_easy.get();

        if (!settings.get_url().empty()) {
            curl_easy_setopt(curl_easy, CURLOPT_URL,
                    settings.get_url().c_str());
        }

        if (HttpSettings::UNKNOWN_TIME_TIMEOUT_MS != settings.get_timeout()) {
            curl_easy_setopt(curl_easy, CURLOPT_TIMEOUT_MS,
                    settings.get_timeout().count());
        }

        curl_easy_setopt(curl_easy, CURLOPT_HTTPHEADER, m_headers.get());
    }
    it = m_messages.erase(it);
}

void CurlContext::set_id_builder(MessageList::iterator& it) {
    const auto& message = static_cast<const SetIdBuilder&>(**it);
    m_id_builder = message.get_callback();
    it = m_messages.erase(it);
}
