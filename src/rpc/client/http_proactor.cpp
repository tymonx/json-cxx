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

#include <json/rpc/client/create_context.hpp>
#include <json/rpc/client/destroy_context.hpp>

#include <iostream>
#include <exception>
#include <algorithm>
#include <curl/curl.h>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <unistd.h>

using json::rpc::Error;
using json::rpc::client::HttpContext;
using json::rpc::client::HttpProactor;

void HttpProactor::CurlMultiDeleter::operator ()(void* curl_multi) {
    curl_multi_cleanup(curl_multi);
}

HttpProactor::HttpProactor() {
    m_curl_multi = CurlMultiPtr{curl_multi_init()};
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
}

void HttpProactor::notify() {
    std::uint64_t notify{1};
    write(m_eventfd, &notify, sizeof(notify));
}

void HttpProactor::push_event(EventPtr event) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_events_background.push_back(event);
    lock.unlock();
    notify();
}

void HttpProactor::get_events() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.splice(m_events.end(), m_events_background);
}

void HttpProactor::setup_context(HttpContext& context) {
    context.m_curl_multi = m_curl_multi.get();
}

void HttpProactor::waiting_for_events() {
    long curl_timeout_ms{0};
    std::uint64_t notify{0};
    struct timeval timeout;

    FD_ZERO(&m_fds_read);
    FD_ZERO(&m_fds_write);
    FD_ZERO(&m_fds_except);
    m_fds_max = -1;

    curl_multi_fdset(m_curl_multi.get(),
            &m_fds_read, &m_fds_write, &m_fds_except, &m_fds_max);

    FD_SET(m_eventfd, &m_fds_read);

    curl_multi_timeout(m_curl_multi.get(), &curl_timeout_ms);
    if (curl_timeout_ms < 0) {
        curl_timeout_ms = 1000;
    }

    timeout.tv_sec = curl_timeout_ms / 1000;
    timeout.tv_usec = (curl_timeout_ms % 1000) * 1000;

    select(m_fds_max + 1, &m_fds_read, &m_fds_write, &m_fds_except, &timeout);
    read(m_eventfd, &notify, sizeof(notify));
}

void HttpProactor::handle_create_context(EventList::iterator& it) {
    m_contexts.emplace_back(new HttpContext{it->get()->get_client(),
            static_cast<CreateContext*>(it->get())->get_http_protocol()});
    it = m_events.erase(it);
}

void HttpProactor::handle_destroy_context(EventList::iterator& it) {
    auto context = std::find(m_contexts.begin(), m_contexts.end(),
        [&it] (const HttpContextPtr& ctx) {
            return ctx->get_client() == it->get()->get_client();
        }
    );
    if (m_contexts.end() != context) {
        if (!context->get()->active()) {
            m_contexts.erase(context);
            it->get()->complete();
            it = m_events.erase(it);
        }
        else {
            ++it;
        }
    }
    else {
        it->get()->complete();
        it = m_events.erase(it);
    }
}

void HttpProactor::handle_events_context(EventList::iterator& it) {
    auto context = std::find(m_contexts.begin(), m_contexts.end(),
        [&it] (const HttpContextPtr& ctx) {
            return ctx->get_client() == it->get()->get_client();
        }
    );
    if (m_contexts.end() != context) {
        context->get()->push_event(std::move(*it));
    }
    else {
        it->get()->complete(Error{Error::INTERNAL_ERROR,
                "Client context doesn't exist"});
    }
    it = m_events.erase(it);
}

void HttpProactor::demultiplexing_events() {
    for (auto it = m_events.begin(); it != m_events.end();) {
        switch (it->get()->get_type()) {
        case EventType::CREATE_CONTEXT:
            handle_create_context(it);
            break;
        case EventType::DESTROY_CONTEXT:
            handle_destroy_context(it);
            break;
        case EventType::CALL_METHOD:
        case EventType::CALL_METHOD_ASYNC:
        case EventType::SEND_NOTIFICATION:
        case EventType::SEND_NOTIFICATION_ASYNC:
            handle_events_context(it);
            break;
        case EventType::UNDEFINED:
        default:
            it = m_events.erase(it);
            break;
        }
    }
}

void HttpProactor::task() {
    int running_handles;

    while (!m_task_done) {
        waiting_for_events();

        get_events();

        demultiplexing_events();

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
                if (context->read_complete(curl_easy)) { break; }
            }
        }
    } while (message);
}

/*
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
*/

