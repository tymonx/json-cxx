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
 * @file json/rpc/client/reactor.cpp
 *
 * @brief JSON client reactor interface
 * */

#include <json/rpc/client/http_proactor.hpp>
#include <json/rpc/client/http_context.hpp>
#include <json/rpc/error.hpp>

#include <iostream>
#include <exception>
#include <curl/curl.h>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <unistd.h>

using json::rpc::Error;
using json::rpc::client::HttpProactor;

HttpProactor::HttpProactor() :
    m_curl_multi{curl_multi_init()}
{
    if (nullptr == m_curl_multi) {
        throw std::exception();
    }

    m_eventfd = eventfd(0, EFD_NONBLOCK);
    if (m_eventfd < 0) {
        throw std::exception();
    }

    curl_multi_setopt(m_curl_multi.get(), CURLMOPT_PIPELINING, 1UL);
    curl_multi_setopt(m_curl_multi.get(), CURLMOPT_MAX_PIPELINE_LENGTH,
            DEFAULT_MAX_PIPELINE_LENGTH);
    curl_multi_setopt(m_curl_multi.get(), CURLMOPT_MAX_HOST_CONNECTIONS, 1UL);

    m_thread = std::thread{&HttpProactor::task, this};
}

HttpProactor::~HttpProactor() {
    m_task_done = true;
    notify();
    m_thread.join();
    close(m_eventfd);

    event_loop();

    while (!m_contexts.empty()) {
        delete static_cast<HttpContext*>(m_contexts.pop());
    }
}

void HttpProactor::setup_context(Context& context) {
    setup_context(static_cast<HttpContext&>(context));
}

void HttpProactor::setup_context(HttpContext& context) {
    context.m_curl_multi = m_curl_multi.get();
}

void HttpProactor::CurlMultiDeleter::operator ()(void* curl_multi) {
    curl_multi_cleanup(curl_multi);
}

void HttpProactor::task() {
    CURLMcode code;
    int running_handles;
    struct curl_waitfd waitfd{};
    waitfd.fd = m_eventfd;
    waitfd.events = CURL_WAIT_POLLIN | CURL_WAIT_POLLOUT;

    while (!m_task_done) {
        code = curl_multi_wait(m_curl_multi.get(), nullptr, 0, 1000, nullptr);

        event_loop();

        for (auto& context : m_contexts) {
            context_processing(static_cast<HttpContext&>(context));
        }

        running_handles = 0;
        curl_multi_perform(m_curl_multi.get(), &running_handles);

        read_processing();
    }
}

void HttpProactor::read_processing() {
    CURLMsg* message;
    int msgs_in_queue;
    do {
        msgs_in_queue = 0;
        message = curl_multi_info_read(m_curl_multi.get(), &msgs_in_queue);
        if (message && (CURLMSG_DONE == message->msg)) {
            CURL* curl_easy = message->easy_handle;
            for (auto& context : m_contexts) {
                if (static_cast<HttpContext&>(context)
                        .read_complete(curl_easy)) { break; }
            }
        }
    } while (message);
}

void HttpProactor::context_processing(HttpContext& context) {
    Event* event;
    for (auto it = context.m_events.begin(); it != context.m_events.end();) {
        std::cout << "Context processing" << std::endl;
        event = static_cast<Event*>(&*it++);
        if (context.add_event_to_processing(event)) {
            context.m_events.remove(event);
        }
    }
}

void HttpProactor::notify() {
    write(m_eventfd, &m_event, sizeof(m_event));
}
