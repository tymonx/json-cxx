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

#include <json/rpc/client/proactor.hpp>
#include <json/rpc/error.hpp>

#include <iostream>
#include <curl/curl.h>
#include <exception>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <unistd.h>

using json::rpc::Error;
using namespace json::rpc::client;

void Proactor::task() {
    std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
    struct timeval timeout{};
    long curl_timeout{-1};
    int err;

    while (!m_task_done) {
        curl_multi_timeout(m_context, &curl_timeout);
        if (curl_timeout < 0) {
            curl_timeout = 980;
        }

        timeout.tv_sec = curl_timeout / 1000;
        timeout.tv_usec = (curl_timeout % 1000) * 1000;

        err = select(m_maxfd + 1, &m_fdread, &m_fdwrite, &m_fdexcep, &timeout);
        if (err < 0) {
            std::cout << "Error!!!" << std::endl;
            continue;
        }

        lock.lock();
        m_events.splice(m_events_background);
        lock.unlock();

        event_loop();

        curl_multi_perform(m_context, nullptr);
    }
}

void Proactor::event_loop() {
    while (!m_events.empty()) {
        event_handling(static_cast<Event*>(m_events.pop()));
    }
}

void Proactor::event_handling(Event* event) {
    if (EventType::CONTEXT == event->get_type()) {
        event->context = m_context;
        m_contexts.push(event);
    }
    else if (EventType::DESTROY_CONTEXT == event->get_type()) {
        delete m_contexts.remove(find_context(event->get_client()));
        Event::event_complete(event);
    }
    else {
        auto context = find_context(event->get_client());
        if (nullptr != context) {
            context->dispatch_event(event);
            FD_ZERO(&m_fdread);
            FD_ZERO(&m_fdwrite);
            FD_ZERO(&m_fdexcep);
            m_maxfd = -1;
            curl_multi_fdset(m_context, &m_fdread, &m_fdwrite, &m_fdexcep, &m_maxfd);
        }
        else {
            Event::event_complete(event, Error{Error::INTERNAL_ERROR,
                    "Client context doesn't exist"});
        }
    }
}

Proactor::Proactor() {
    m_eventfd = eventfd(0, EFD_NONBLOCK);
    if (m_eventfd < 0) {
        throw std::exception();
    }

    m_context = curl_multi_init();
    if (nullptr == m_context) {
        throw std::exception();
    }
    curl_multi_setopt(m_context, CURLMOPT_PIPELINING, 1UL);

    curl_multi_fdset(m_context, &m_fdread, &m_fdwrite, &m_fdexcep, &m_maxfd);

    m_thread = std::thread{&Proactor::task, this};
}

Proactor::~Proactor() {
    m_task_done = true;
    write(m_eventfd, &m_event, sizeof(m_event));
    m_thread.join();
    close(m_eventfd);

    std::unique_lock<std::mutex> lock(m_mutex);
    m_events.splice(m_events_background);
    lock.unlock();

    event_loop();

    while (!m_contexts.empty()) {
        auto context = static_cast<Context*>(m_contexts.pop());
        delete context;
    }

    curl_multi_cleanup(m_context);
}

void Proactor::push_event(Event* pevent) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_events_background.push(pevent);
    lock.unlock();
    write(m_eventfd, &m_event, sizeof(m_event));
}
