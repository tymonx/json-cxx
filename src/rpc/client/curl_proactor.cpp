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

#include <json/rpc/client/curl_proactor.hpp>
#include <json/rpc/client/curl_context.hpp>
#include <json/rpc/client/http_settings.hpp>
#include <json/rpc/client/http_client.hpp>

#include <json/rpc/error.hpp>
#include <json/rpc/client/create_context.hpp>
#include <json/rpc/client/destroy_context.hpp>

#include <exception>
#include <algorithm>
#include <curl/curl.h>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <unistd.h>

using json::rpc::Error;
using json::rpc::client::Event;
using json::rpc::client::CurlContext;
using json::rpc::client::HttpSettings;
using json::rpc::client::CurlProactor;
using json::rpc::client::DestroyContext;

void CurlProactor::CurlMultiDeleter::operator ()(void* curl_multi) {
    curl_multi_cleanup(curl_multi);
}

CurlProactor::CurlProactor() {
    curl_global_init(CURL_GLOBAL_ALL);

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

    m_thread = std::thread{&CurlProactor::task, this};
}

CurlProactor::~CurlProactor() {
    m_task_done = true;
    notify();
    m_thread.join();
    /* Finish job */
    task();
    close(m_eventfd);

    m_contexts.clear();
    m_curl_multi.reset(nullptr);

    curl_global_cleanup();
}

void CurlProactor::notify() {
    std::uint64_t event{1};
    write(m_eventfd, &event, sizeof(event));
}

void CurlProactor::push_event(EventPtr&& event) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_events_background.push_back(std::move(event));
    lock.unlock();
    notify();
}

void CurlProactor::get_events() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.splice(m_events.end(), m_events_background);
}

void CurlProactor::waiting_for_events() {
    std::uint64_t event{0};
    struct curl_waitfd waitfd[1]{};
    waitfd[0].fd = m_eventfd;
    waitfd[0].events = CURL_WAIT_POLLIN | CURL_WAIT_POLLPRI;

    curl_multi_wait(m_curl_multi.get(), waitfd, 1, 1000, nullptr);
    read(m_eventfd, &event, sizeof(event));
}

void CurlProactor::handle_create_context(EventList::iterator& it) {
    m_contexts.emplace_back(new CurlContext{
            static_cast<const HttpClient*>((*it)->get_client()), *this});
    it = m_events.erase(it);
}

static inline
void destroy_context_event(Event* event) {
    static_cast<DestroyContext*>(event)->m_result.set_value();
}

void CurlProactor::handle_destroy_context(EventList::iterator& it) {
    auto context = std::find_if(m_contexts.begin(), m_contexts.end(),
        [&it] (const CurlContextPtr& ctx) {
            return ctx->get_client() == it->get()->get_client();
        }
    );
    if (m_contexts.end() != context) {
        if (!context->get()->active()) {
            m_contexts.erase(context);
            destroy_context_event(it->get());
            it = m_events.erase(it);
        }
        else {
            ++it;
        }
    }
    else {
        destroy_context_event(it->get());
        it = m_events.erase(it);
    }
}

void CurlProactor::handle_events_context(EventList::iterator& it) {
    auto context = std::find_if(m_contexts.begin(), m_contexts.end(),
        [&it] (const CurlContextPtr& ctx) {
            return ctx->get_client() == (*it)->get_client();
        }
    );
    if (m_contexts.end() != context) {
        if (0_ms != (*context)->get_time_live()) {
            (*it)->set_time_live((*context)->get_time_live());
        }
        (*context)->splice_event(m_events, it++);
    }
    else {
        m_executor.execute(std::move(*it), {Error::INTERNAL_ERROR});
        it = m_events.erase(it);
    }
}

void CurlProactor::dispatch_events() {
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

void CurlProactor::task() {
    do {
        waiting_for_events();

        get_events();

        dispatch_events();

        context_processing();

        m_running_handles = 0;
        curl_multi_perform(m_curl_multi.get(), &m_running_handles);

        read_processing();
    } while (!m_task_done || !m_events.empty());
}

void CurlProactor::context_processing() {
    for (auto& context : m_contexts) {
        context->dispatch_events();
    }
}

void CurlProactor::read_processing() {
    CURLMsg* message;
    struct CurlContext::InfoRead* info_read;
    int msgs_in_queue;
    do {
        msgs_in_queue = 0;
        message = curl_multi_info_read(m_curl_multi.get(), &msgs_in_queue);
        if (message && (CURLMSG_DONE == message->msg)) {
            info_read = nullptr;
            curl_easy_getinfo(message->easy_handle,
                    CURLINFO_PRIVATE, &info_read);
            if (nullptr != info_read) {
                info_read->callback(info_read->context, info_read,
                        message->data.result);
            }
            else {
                curl_multi_remove_handle(m_curl_multi.get(),
                        message->easy_handle);
            }
        }
    } while (nullptr != message);
}
