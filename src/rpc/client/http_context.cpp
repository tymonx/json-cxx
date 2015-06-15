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

#include <json/rpc/client/request.hpp>
#include <json/rpc/client/call_method.hpp>
#include <json/rpc/client/call_method_async.hpp>
#include <json/rpc/client/send_notification.hpp>

#include <iostream>
#include <curl/curl.h>
#include <exception>

using json::rpc::client::CallMethod;
using json::rpc::client::CallMethodAsync;
using json::rpc::client::SendNotification;
using json::rpc::client::HttpContext;

HttpContext::HttpContext(Client* client, const HttpProtocol& protocol)
    : Context{client}, m_protocol{protocol}
{
    std::string http_header{};
    struct ::curl_slist* headers{nullptr};
    unsigned pipeline_size{m_protocol.get_pipeline_length()};

    for (const auto& header : m_protocol.get_headers()) {
        http_header = header.first + ": " + header.second;
        headers = curl_slist_append(headers, http_header.c_str());
    }

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charset: utf-8");
    m_headers = std::move(CurlSlistPtr{headers});

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
    for (auto& pipe : m_pipelines) {
        if (nullptr != pipe.event) {
            Event::event_complete(pipe.event, Error{Error::INTERNAL_ERROR});
            curl_multi_remove_handle(m_curl_multi, pipe.curl_easy.get());
        }
    }

    while (!m_events.empty()) {
        Event::event_complete(static_cast<Event*>(m_events.pop()),
                Error{Error::INTERNAL_ERROR});
    }
}

size_t HttpContext::write_function(char* buffer, size_t size, size_t nmemb,
        void* userdata)
{
    Pipeline* pipe = static_cast<Pipeline*>(userdata);
    size_t copy = size * nmemb;
    pipe->response.append(buffer, copy);
    std::cout << "Write function: " << pipe->response
        << " size: " << size << "," << pipe->response.size()
        << " nmemb: " << nmemb
        << " pos: " << pipe->response_pos
        << "" << buffer[0] << buffer[1] << std::endl;
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

    std::cout << "Read function: " << pipe->request << " size: " << pipe->request.size() << std::endl;

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

bool HttpContext::add_event_to_processing(Event* event) {
    Id id{0};
    CURLMcode code;
    for (auto& pipe : m_pipelines) {
        if (nullptr == pipe.event) {
            pipe.event = event;
            pipe.request.clear();
            pipe.request_pos = 0;
            pipe.response.clear();
            pipe.response_pos = 0;
            pipe.request << build_message(
                    static_cast<const Request&>(*event), id);
            curl_easy_setopt(pipe.curl_easy.get(), CURLOPT_POSTFIELDSIZE, pipe.request.size());
            code = curl_multi_add_handle(m_curl_multi, pipe.curl_easy.get());
            std::cout << "HttpContext: add_event " << int(event->get_type())
                << " status: " << code
                << " Avent: " << event << std::endl;
            return true;
        }
        ++id;
    }
    return false;
}

bool HttpContext::read_complete(void* curl_easy) {
    for (auto& pipe : m_pipelines) {
        if (curl_easy == pipe.curl_easy.get()) {
            pipe.response >> static_cast<Request*>(pipe.event)->m_value;
            curl_multi_remove_handle(m_curl_multi, curl_easy);
            Event::event_complete(pipe.event);
            pipe.event = nullptr;
            return true;
        }
    }
    return false;
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
    if (EventType::SEND_NOTIFICATION != request.get_type()) {
        message["id"] = id;
    }
    return message;
}

void HttpContext::dispatch_event(Event* event) {
    std::cout << "DispatchEvent" << std::endl;
    if (!add_event_to_processing(event)) {
        m_events.push(event);
    }
}
