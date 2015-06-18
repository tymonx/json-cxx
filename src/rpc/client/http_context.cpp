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

#include <iostream>
#include <curl/curl.h>
#include <exception>

using json::Deserializer;
using json::rpc::Error;
using json::rpc::client::CallMethod;
using json::rpc::client::SendNotification;
using json::rpc::client::HttpContext;

HttpContext::HttpContext(const Client* client, const HttpProtocol& protocol,
        void* curl_multi) :
    m_client{client}, m_curl_multi{curl_multi}, m_protocol{protocol}
{
    using std::placeholders::_1;
    using std::placeholders::_2;
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

    /* Keep alive */
    m_keep_alive.curl_easy.reset(curl_easy_init());
    if (nullptr == m_keep_alive.curl_easy) {
        throw std::bad_alloc();
    }
    m_keep_alive.context = this;
    m_keep_alive.callback = &HttpContext::handle_keep_alive;
    curl_easy = m_keep_alive.curl_easy.get();
    curl_easy_setopt(curl_easy, CURLOPT_URL, m_protocol.get_url().c_str());
    curl_easy_setopt(curl_easy, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl_easy, CURLOPT_TCP_KEEPIDLE, 2L);
    curl_easy_setopt(curl_easy, CURLOPT_TCP_KEEPINTVL, 1L);
    curl_easy_setopt(curl_easy, CURLOPT_PRIVATE,
            static_cast<InfoRead*>(&m_keep_alive));
    curl_multi_add_handle(m_curl_multi, curl_easy);
}

HttpContext::~HttpContext() {
    for (auto it = m_pipelines.begin(); it != m_pipelines.end(); ++it) {
        if (nullptr != it->event) {
            curl_multi_remove_handle(m_curl_multi, it->curl_easy.get());
        }
    }
    curl_multi_remove_handle(m_curl_multi, m_keep_alive.curl_easy.get());
}

void HttpContext::handle_keep_alive(struct InfoRead* info_read,
        unsigned curl_code)
{
    auto keep_alive = static_cast<KeepAlive*>(info_read);

    (void)keep_alive;
    curl_multi_remove_handle(m_curl_multi, keep_alive->curl_easy.get());
    curl_multi_add_handle(m_curl_multi, keep_alive->curl_easy.get());

    if (curl_code != 7) {
        std::cout << "KeepAlive: " << curl_code << std::endl;
    }
}

Error HttpContext::check_response(const Value& value) {
    if (!value.is_object()) {
        return {Error::PARSE_ERROR, "Is not a JSON object"};
    }
    if (3 != value.size()) {
        return {Error::PARSE_ERROR, "Invalid params number"};
    }
    if (value["jsonrpc"] != "2.0") {
        return {Error::PARSE_ERROR, "Invalid/missing 'jsonrpc' member"};
    }
    if (value.is_member("result")) {
        if (!value["id"].is_uint()) {
            return {Error::PARSE_ERROR, "Invalid/missing 'id' member"};
        }
    }
    else if (value.is_member("error")) {
        auto& error = value["error"];
        if (!error.is_object()) {
            return {Error::PARSE_ERROR, "Invalid 'error' member"};
        }
        if (!error["code"].is_int()) {
            return {Error::PARSE_ERROR, "Invalid/missing 'code' in 'error'"};
        }
        if (!error["message"].is_string()) {
            return {Error::PARSE_ERROR, "Invalid/missing 'message' in 'error'"};
        }
        if (2 == error.size()) {
            /* Do nothing */
        }
        else if (3 == error.size()) {
            if (!error.is_member("data")) {
                return {Error::PARSE_ERROR, "Missing 'data' member in 'error'"};
            }
        }
        else {
            return {Error::PARSE_ERROR, "Invalid params number in 'error'"};
        }
        if (!value["id"].is_int() && !value["id"].is_null()) {
            return {Error::PARSE_ERROR, "Invalid/missing 'id' member"};
        }
    }
    else {
        return {Error::PARSE_ERROR, "Missing 'result' or 'error' member"};
    }
    return {Error::OK};
}

Error HttpContext::handle_pipe_response(Pipeline& pipe) {
    Value value;
    Deserializer deserializer(pipe.response);
    if (deserializer.is_invalid()) {
        auto error = deserializer.get_error();
        return {Error::PARSE_ERROR, "Invalid response: \'" +
                pipe.response + "\' " + error.decode() + " at " +
                std::to_string(error.offset)};
    }
    deserializer >> value;
    if (auto error = check_response(value)) { return error; }
    if (value.is_member("result")) {
        static_cast<Request*>(pipe.event.get())->m_value = value["result"];
    }
    else {
        return {
            value["error"]["code"].as_int(),
            value["error"]["message"].as_string(),
            value["error"]["data"]
        };
    }
    return {Error::OK};
}

void HttpContext::handle_pipe(struct InfoRead* info_read,
        unsigned curl_code) {
    auto pipe = static_cast<Pipeline*>(info_read);

    switch (curl_code) {
    case CURLE_OK:
        pipe->event->complete(handle_pipe_response(*pipe));
        break;
    case CURLE_COULDNT_CONNECT:
        m_events.push_back(std::move(pipe->event));
        break;
    default:
        pipe->event->complete({Error::Code(curl_code),
                curl_easy_strerror(CURLcode(curl_code))});
        break;
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
    pipe->response.append(buffer, copy);
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
    if (TimePoint(0_ms) != it->get()->get_time_live()) {
        if (std::chrono::steady_clock::now() > it->get()->get_time_live()) {
            it->get()->complete(Error{Error::INTERNAL_ERROR,
                    "Timeout occur"});
            it = m_events.erase(it);
            return true;
        }
    }
    return false;
}

void HttpContext::handle_event_request(EventList::iterator& it) {
    Id id{0};
    for (auto& pipe : m_pipelines) {
        if (nullptr == pipe.event) {
            pipe.event = std::move(*it);
            pipe.request.clear();
            pipe.request_pos = 0;
            pipe.response.clear();
            pipe.request << build_message(static_cast<const Request&>(*pipe.event), id);
            curl_easy_setopt(pipe.curl_easy.get(), CURLOPT_POSTFIELDSIZE,
                    pipe.request.size());
            curl_multi_add_handle(m_curl_multi, pipe.curl_easy.get());
            it = m_events.erase(it);
            ++m_pipes_active;
            return;
        }
        ++id;
    }
    ++it;
}

void HttpContext::dispatch_events() {
    for (auto it = m_events.begin(); it != m_events.end();) {
        if (handle_event_timeout(it)) { continue; }

        switch (it->get()->get_type()) {
        case EventType::CALL_METHOD:
        case EventType::CALL_METHOD_ASYNC:
        case EventType::SEND_NOTIFICATION:
        case EventType::SEND_NOTIFICATION_ASYNC:
            handle_event_request(it);
            break;
        case EventType::CREATE_CONTEXT:
        case EventType::DESTROY_CONTEXT:
        case EventType::UNDEFINED:
        default:
            it->get()->complete(Error{Error::INTERNAL_ERROR,
                    "Client context cannot handle event object"});
            it = m_events.erase(it);
            break;
        }
    }
}

bool HttpContext::active() const {
    return !m_events.empty() || m_pipes_active;
}

bool HttpContext::read_complete(void* curl_easy) {
    for (auto& pipe : m_pipelines) {
        if (curl_easy == pipe.curl_easy.get()) {
            pipe.response >> static_cast<Request*>(pipe.event.get())->m_value;
            curl_multi_remove_handle(m_curl_multi, curl_easy);
            pipe.event->complete();
            pipe.event.reset(nullptr);
            --m_pipes_active;
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
    if (EventType::CALL_METHOD == request.get_type() ||
        EventType::CALL_METHOD_ASYNC == request.get_type()) {
        message["id"] = id;
    }
    return message;
}
