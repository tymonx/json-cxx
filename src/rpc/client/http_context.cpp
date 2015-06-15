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

#include <json/rpc/client/call_method.hpp>
#include <json/rpc/client/call_method_async.hpp>
#include <json/rpc/client/send_notification.hpp>

#include <curl/curl.h>
#include <exception>

using json::rpc::client::CallMethod;
using json::rpc::client::CallMethodAsync;
using json::rpc::client::SendNotification;
using json::rpc::client::HttpContext;

HttpContext::HttpContext(Client* client, HttpProactor& proactor,
        const HttpProtocol& protocol) : Context{client},
    m_proactor{proactor}, m_protocol{protocol}
{
    std::string http_header{};
    struct ::curl_slist* headers{nullptr};
    unsigned pipeline_size{m_protocol.get_pipeline_length()};
    (void)m_proactor;

    for (const auto& header : m_protocol.get_headers()) {
        http_header = header.first + ": " + header.second;
        headers = curl_slist_append(headers, http_header.c_str());
    }

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charset: utf-8");
    m_headers = std::move(CurlSlistPtr{headers});

    pipeline_size = (pipeline_size < m_proactor.get_max_pipeline_length()) ?
        pipeline_size : m_proactor.get_max_pipeline_length();

    m_pipelines.resize(pipeline_size);
    for (auto& pipe : m_pipelines) {
        CURL* curl_easy = curl_easy_init();
        if (nullptr == curl_easy) {
            throw std::bad_alloc();
        }
        curl_easy_setopt(curl_easy, CURLOPT_URL, m_protocol.get_url().c_str());
        curl_easy_setopt(curl_easy, CURLOPT_WRITEFUNCTION, write_function);
        curl_easy_setopt(curl_easy, CURLOPT_WRITEDATA, &pipe);
        curl_easy_setopt(curl_easy, CURLOPT_READFUNCTION, read_function);
        curl_easy_setopt(curl_easy, CURLOPT_READDATA, &pipe);
        curl_easy_setopt(curl_easy, CURLOPT_POSTFIELDS, nullptr);
        curl_easy_setopt(curl_easy, CURLOPT_POSTFIELDSIZE, 0);
        curl_easy_setopt(curl_easy, CURLOPT_HTTPHEADER, m_headers.get());
        curl_easy_setopt(curl_easy, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
        curl_easy_setopt(curl_easy, CURLOPT_TIMEOUT_MS,
                m_protocol.get_timeout().count());
        pipe.curl_easy = std::move(CurlEasyPtr{curl_easy});

    }
}

HttpContext::~HttpContext() {

}

size_t HttpContext::write_function(char* buffer, size_t size, size_t nmemb,
        void* userdata)
{
    struct pipeline* pipe = static_cast<struct pipeline*>(userdata);
    size_t copy = size * nmemb;
    pipe->response.append(buffer, copy, pipe->response_pos);
    pipe->response_pos += copy;
    return copy;
}

size_t HttpContext::read_function(char* buffer, size_t size, size_t nmemb,
        void* userdata)
{
    struct pipeline* pipe = static_cast<struct pipeline*>(userdata);
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

bool HttpContext::event_to_message(Event* event) {
    for (Pipelines::size_type i = 0; i < m_pipelines.size(); ++i) {
        if (nullptr == m_pipelines[i].event) {
            return event_to_pipeline(event, m_pipelines[i], unsigned(i));
        }
    }
    return false;
}

static
void build_params(json::Value& params, json::Value& message) {
    if (params.is_object() || params.is_array()) {
        message["params"] = params;
    }
    else {
        message["params"].push_back(params);
    }
}

static inline
void build_message(CallMethod* event, json::Value& message, unsigned id) {
    message["method"] = event->m_name;
    build_params(event->m_value, message);
    message["id"] = id;
}

static inline
void build_message(CallMethodAsync* event, json::Value& message, unsigned id) {
    message["method"] = event->m_name;
    build_params(event->m_value, message);
    message["id"] = id;
}

static inline
void build_message(SendNotification* event, json::Value& message) {
    message["method"] = event->m_name;
    build_params(event->m_value, message);
}

bool HttpContext::build_message(Event* event, json::Value& message,
        unsigned id)
{
    bool valid{true};
    message["jsonrpc"] = "2.0";
    if (EventType::CALL_METHOD == event->get_type()) {
        ::build_message(static_cast<CallMethod*>(event), message, id);
    }
    else if (EventType::CALL_METHOD_ASYNC == event->get_type()) {
        ::build_message(static_cast<CallMethodAsync*>(event), message, id);
    }
    else if (EventType::SEND_NOTIFICATION == event->get_type()) {
        ::build_message(static_cast<SendNotification*>(event), message);
    }
    else {
        valid = false;
    }
    return valid;
}

bool HttpContext::event_to_pipeline(Event* event, struct pipeline& pipe,
        unsigned id)
{
    json::Value message{json::Value::Type::OBJECT};
    if (build_message(event, message, id)) {
        pipe.event = event;
        pipe.request.clear();
        pipe.request_pos = 0;
        pipe.response.clear();
        pipe.response_pos = 0;
        pipe.request << message;
        return true;
    }
    return false;
}

void HttpContext::dispatch_event(Event* event) {
    if (!event_to_message(event)) {
        m_events.push(event);
    }
}
