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
    m_pipelines.resize(16);
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
            curl_multi_remove_handle(m_curl_multi, it->curl_easy.get());
        }
    }
}

void CurlContext::handle_pipe(struct InfoRead* info_read,
        unsigned curl_code) {
    auto pipe = static_cast<Pipeline*>(info_read);

    if (CURLE_OK == curl_code) {
        //m_proactor.get_executor().execute(std::move(pipe->event));
    }
    else {
        /*
        if (!m_proactor.task_done()) {
            m_events.push_back(std::move(pipe->event));
        }
        else {
            //m_proactor.get_executor().execute(std::move(pipe->event),
            //        {Error::INTERNAL_ERROR});;
        }
        */
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
    pipe->response.append(buffer, copy);
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

bool CurlContext::handle_event_timeout(MessageList::iterator& it) {
    if (TimePoint(0_ms) != (*it)->get_time_live()) {
        if (std::chrono::steady_clock::now() > (*it)->get_time_live()) {
            //m_proactor.get_executor().execute(std::move(*it),
            //        {Error::INTERNAL_ERROR});
            it = m_messages.erase(it);
            return true;
        }
    }
    return false;
}

json::Value CurlContext::build_message(Request& request, Id id) {
    json::Value message;
    message["jsonrpc"] = "2.0";
    message["method"] = request.get_name();
    if (request.get_value().is_object() || request.get_value().is_array()) {
        message["params"] = request.get_value();
    }
    else if (!request.get_value().is_null()) {
        message["params"].push_back(request.get_value());
    }
    if (MessageTypeU((MessageType::CALL_METHOD
        | MessageType::CALL_METHOD_ASYNC) & request.get_type()))
    {
        CallMethod& call_method = static_cast<CallMethod&>(request);
        if (nullptr == m_id_builder) {
            call_method.set_id(id);
        }
        else {
            call_method.set_id(m_id_builder(id));
        }
        message["id"] = call_method.get_id();
    }
    return message;
}

void CurlContext::handle_event_request(MessageList::iterator& it) {
    if (m_pipes_active < m_pipelines.size()) {
        Id id{0};
        for (auto& pipe : m_pipelines) {
            if (nullptr == pipe.event) {
                pipe.event = std::move(*it);
                Request& request = static_cast<Request&>(*pipe.event);
                pipe.request.clear();
                pipe.request_pos = 0;
                request.get_response().clear();
                pipe.request << build_message(request, id);
                curl_easy_setopt(pipe.curl_easy.get(), CURLOPT_POSTFIELDSIZE,
                        pipe.request.size());
                curl_multi_add_handle(m_proactor.get_curl_multi(),
                        pipe.curl_easy.get());
                it = m_events.erase(it);
                ++m_pipes_active;
                return;
            }
            ++id;
        }
    }
    ++it;
}

void CurlContext::dispatch_events() {
    for (auto it = m_events.begin(); it != m_events.end();) {
        if (handle_event_timeout(it)) { continue; }

        if (MessageTypeU((MessageType::SEND_NOTIFICATION
            | MessageType::SEND_NOTIFICATION_ASYNC
            | MessageType::CALL_METHOD
            | MessageType::CALL_METHOD_ASYNC) & (*it)->get_type()))
        {
            handle_event_request(it);
        }
        else {
            m_proactor.get_executor().execute(std::move(*it),
                    {Error::INTERNAL_ERROR});
            it = m_events.erase(it);
        }
    }
}
