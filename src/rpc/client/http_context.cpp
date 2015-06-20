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
 * @file json/rpc/client/http_context.cpp
 *
 * @brief JSON client message interface
 * */

#include <json/rpc/client/http_context.hpp>

#include <json/deserializer.hpp>

#include <json/rpc/client/request.hpp>
#include <json/rpc/client/call_method.hpp>
#include <json/rpc/client/send_notification.hpp>

#include <curl/curl.h>
#include <exception>
#include <algorithm>

using json::Deserializer;
using json::rpc::Error;
using json::rpc::client::CallMethod;
using json::rpc::client::SendNotification;
using json::rpc::client::HttpContext;

HttpContext::HttpContext(const Client* client, const HttpProtocol& protocol,
        void* curl_multi, Executor& executor) :
    m_client{client}, m_curl_multi{curl_multi}, m_protocol{protocol},
    m_executor{executor}
{
    std::string http_header{};
    struct ::curl_slist* headers{nullptr};
    unsigned pipeline_size{m_protocol.get_pipeline_length()};
    CURL* curl_easy;

    for (const auto& header : m_protocol.get_headers()) {
        http_header = header.first + ": " + header.second;
        headers = curl_slist_append(headers, http_header.c_str());
    }

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charset: utf-8");
    m_headers = std::move(CurlSlistPtr{headers});

    /* Pipelines */
    Id id{0};
    m_pipelines.resize(pipeline_size);
    for (auto& pipe : m_pipelines) {
        pipe.id = id++;
        pipe.curl_easy.reset(curl_easy_init());
        if (nullptr == pipe.curl_easy) {
            throw std::bad_alloc();
        }
        pipe.context = this;
        pipe.callback = &HttpContext::handle_pipe;
        curl_easy = pipe.curl_easy.get();
        curl_easy_setopt(curl_easy, CURLOPT_URL, m_protocol.get_url().c_str());
        curl_easy_setopt(curl_easy, CURLOPT_WRITEFUNCTION, write_function);
        curl_easy_setopt(curl_easy, CURLOPT_WRITEDATA, &pipe);
        curl_easy_setopt(curl_easy, CURLOPT_READFUNCTION, read_function);
        curl_easy_setopt(curl_easy, CURLOPT_READDATA, &pipe);
        curl_easy_setopt(curl_easy, CURLOPT_POSTFIELDS, nullptr);
        curl_easy_setopt(curl_easy, CURLOPT_POSTFIELDSIZE, 0);
        curl_easy_setopt(curl_easy, CURLOPT_HTTPHEADER, m_headers.get());
        curl_easy_setopt(curl_easy, CURLOPT_PROTOCOLS,
                CURLPROTO_HTTP | CURLPROTO_HTTPS);
        curl_easy_setopt(curl_easy, CURLOPT_TIMEOUT_MS,
                m_protocol.get_timeout().count());
        curl_easy_setopt(curl_easy, CURLOPT_PRIVATE,
                static_cast<InfoRead*>(&pipe));
    }
}

HttpContext::~HttpContext() {
    for (auto it = m_pipelines.begin(); it != m_pipelines.end(); ++it) {
        if (nullptr != it->event) {
            curl_multi_remove_handle(m_curl_multi, it->curl_easy.get());
        }
    }
}

void HttpContext::handle_pipe(struct InfoRead* info_read,
        unsigned curl_code) {
    auto pipe = static_cast<Pipeline*>(info_read);

    if (CURLE_OK == curl_code) {
        m_executor.push_event(std::move(pipe->event));
    }
    else {
        m_events.push_back(std::move(pipe->event));
    }

    pipe->event = nullptr;
    curl_multi_remove_handle(m_curl_multi, pipe->curl_easy.get());
    --m_pipes_active;
}

size_t HttpContext::write_function(char* buffer, size_t size, size_t nmemb,
        void* userdata)
{
    Pipeline* pipe = static_cast<Pipeline*>(userdata);
    size_t copy = size * nmemb;
    static_cast<Request*>(&*pipe->event)->m_response.append(buffer, copy);
    return copy;
}

size_t HttpContext::read_function(char* buffer, size_t size, size_t nmemb,
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

void HttpContext::CurlEasyDeleter::operator()(void* curl_easy) {
    curl_easy_cleanup(curl_easy);
}

void HttpContext::CurlSlistDeleter::operator()(struct curl_slist* curl_slist) {
    curl_slist_free_all(curl_slist);
}

bool HttpContext::handle_event_timeout(EventList::iterator& it) {
    if (TimePoint(0_ms) != (*it)->get_time_live()) {
        if (std::chrono::steady_clock::now() > (*it)->get_time_live()) {
            m_executor.push_event(std::move(*it), {Error::INTERNAL_ERROR,
                    "Timeout occur"});
            it = m_events.erase(it);
            return true;
        }
    }
    return false;
}

void HttpContext::handle_event_request(EventList::iterator& it) {
    if (m_pipes_active < m_pipelines.size()) {
        auto pipe = std::find_if(
            m_pipelines.begin(),
            m_pipelines.end(),
            [] (const Pipeline& p) { return nullptr == p.event; }
        );
        if (m_pipelines.end() != pipe) {
            pipe->event = std::move(*it);
            pipe->request.clear();
            pipe->request_pos = 0;
            static_cast<Request&>(*pipe->event).m_response.clear();
            pipe->request << build_message(
                    static_cast<const Request&>(*pipe->event), pipe->id);
            curl_easy_setopt(pipe->curl_easy.get(), CURLOPT_POSTFIELDSIZE,
                    pipe->request.size());
            curl_multi_add_handle(m_curl_multi, pipe->curl_easy.get());
            it = m_events.erase(it);
            ++m_pipes_active;
            return;
        }
    }
    ++it;
}

void HttpContext::dispatch_events() {
    for (auto it = m_events.begin(); it != m_events.end();) {
        if (handle_event_timeout(it)) { continue; }

        if (EventTypeU((EventType::SEND_NOTIFICATION
            | EventType::SEND_NOTIFICATION_ASYNC
            | EventType::CALL_METHOD
            | EventType::CALL_METHOD_ASYNC) & (*it)->get_type()))
        {
            handle_event_request(it);
        }
        else {
            m_executor.push_event(std::move(*it), {Error::INTERNAL_ERROR,
                    "Client context cannot handle event object"});
            it = m_events.erase(it);
        }
    }
}

json::Value HttpContext::build_message(const Request& request, Id id) {
    json::Value message;
    message["jsonrpc"] = "2.0";
    message["method"] = request.m_name;
    if (request.m_value.is_object() || request.m_value.is_array()) {
        message["params"] = request.m_value;
    }
    else {
        message["params"].push_back(request.m_value);
    }
    if (EventTypeU((EventType::CALL_METHOD
        | EventType::CALL_METHOD_ASYNC) & request.get_type()))
    {
        message["id"] = id;
    }
    return message;
}
