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
 * @file json/rpc/client/curl_proactor.cpp
 *
 * @brief HTTP JSON RPC client proactor implementation based on CURL library
 * */

#include <json/rpc/client/curl_proactor.hpp>
#include <json/rpc/client/curl_context.hpp>
#include <json/rpc/client/http_settings.hpp>
#include <json/rpc/client/http_client.hpp>

#include <json/rpc/error.hpp>
#include <json/rpc/client/message/create_context.hpp>
#include <json/rpc/client/message/destroy_context.hpp>

#include <exception>
#include <algorithm>
#include <curl/curl.h>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <unistd.h>

using json::rpc::Error;
using json::rpc::client::Message;
using json::rpc::client::CurlContext;
using json::rpc::client::CurlProactor;
using json::rpc::client::message::CreateContext;
using json::rpc::client::message::DestroyContext;

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

    m_thread = std::thread{&CurlProactor::task, this};
}

CurlProactor::~CurlProactor() {
    m_task_done = true;
    notify();
    if (m_thread.joinable()) {
        m_thread.join();
    }
    /* Finish job */
    task();
    close(m_eventfd);

    m_contexts.clear();
    m_curl_multi.reset(nullptr);

    curl_global_cleanup();
}

void CurlProactor::set_max_total_connections(MaxTotalConnections amount) {
     curl_multi_setopt(m_curl_multi.get(),
            CURLMOPT_MAX_TOTAL_CONNECTIONS, amount);
}

void CurlProactor::notify() {
    std::uint64_t message{1};
    ssize_t err = write(m_eventfd, &message, sizeof(message));
    (void)err;
}

void CurlProactor::push_message(MessagePtr&& message) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_messages_background.push_back(std::move(message));
    lock.unlock();
    notify();
}

void CurlProactor::waiting_for_messages() {
    struct curl_waitfd waitfd[1]{};
    waitfd[0].fd = m_eventfd;
    waitfd[0].events = CURL_WAIT_POLLIN | CURL_WAIT_POLLPRI;

    curl_multi_wait(m_curl_multi.get(), waitfd, 1, 1000, nullptr);
    if ((CURL_WAIT_POLLIN | CURL_WAIT_POLLPRI) & waitfd[0].revents) {
        std::uint64_t messages{0};
        ssize_t err = read(m_eventfd, &messages, sizeof(messages));
        if ((messages > 0) && (EAGAIN != err)) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_messages.splice(m_messages.end(), m_messages_background);
        }
    }
}

void CurlProactor::create_context(MessageList::iterator& it) {
    m_contexts.emplace_back(new CurlContext{
            static_cast<HttpClient*>((*it)->get_client()), m_curl_multi.get()});
    it = m_messages.erase(it);
}

void CurlProactor::destroy_context(MessageList::iterator& it) {
    auto context = std::find_if(m_contexts.begin(), m_contexts.end(),
        [&it] (const CurlContextPtr& ctx) {
            return ctx->get_client() == it->get()->get_client();
        }
    );
    if (m_contexts.end() != context) {
        if (!context->get()->active()) {
            m_contexts.erase(context);
            static_cast<DestroyContext&>(**it).set_result();
            it = m_messages.erase(it);
        }
        else {
            ++it;
        }
    }
    else {
        static_cast<DestroyContext&>(**it).set_result();
        it = m_messages.erase(it);
    }
}

void CurlProactor::context_message(MessageList::iterator& it) {
    auto context = std::find_if(m_contexts.begin(), m_contexts.end(),
        [&it] (const CurlContextPtr& ctx) {
            return ctx->get_client() == (*it)->get_client();
        }
    );
    if (m_contexts.end() != context) {
        (*context)->splice_message(m_messages, it++);
    }
    else {
        Executor executor{};
        executor.execute(std::move(*it), {Error::INTERNAL_ERROR});
        it = m_messages.erase(it);
    }
}

void CurlProactor::dispatch_messages() {
    for (auto it = m_messages.begin(); it != m_messages.end();) {
        switch (it->get()->get_type()) {
        case MessageType::CREATE_CONTEXT:
            create_context(it);
            break;
        case MessageType::DESTROY_CONTEXT:
            destroy_context(it);
            break;
        case MessageType::CALL_METHOD_SYNC:
        case MessageType::CALL_METHOD_ASYNC:
        case MessageType::SEND_NOTIFICATION_SYNC:
        case MessageType::SEND_NOTIFICATION_ASYNC:
        case MessageType::CONNECT:
        case MessageType::DISCONNECT:
        case MessageType::SET_ERROR_TO_EXCEPTION:
        case MessageType::SET_HTTP_SETTINGS:
        case MessageType::SET_ID_BUILDER:
            context_message(it);
            break;
        case MessageType::UNDEFINED:
        default:
            it = m_messages.erase(it);
            break;
        }
    }
}

void CurlProactor::task() {
    do {
        waiting_for_messages();

        dispatch_messages();

        context_processing();

        m_running_handles = 0;
        curl_multi_perform(m_curl_multi.get(), &m_running_handles);

        read_processing();
    } while (!m_task_done || !m_messages.empty() || m_running_handles);
}

void CurlProactor::context_processing() {
    for (auto& context : m_contexts) {
        context->dispatch_messages();
    }
}

void CurlProactor::read_processing() {
    CURLMsg* message;
    struct CurlContext::InfoRead* info_read;
    int msgs_in_queue;
    CURL* curl_easy;
    do {
        msgs_in_queue = 0;
        message = curl_multi_info_read(m_curl_multi.get(), &msgs_in_queue);
        if (message && (CURLMSG_DONE == message->msg)) {
            info_read = nullptr;
            curl_easy = message->easy_handle;
            curl_easy_getinfo(curl_easy, CURLINFO_PRIVATE, &info_read);
            if (nullptr != info_read) {
                info_read->callback(info_read->context,
                        CurlContext::InfoReadPtr{info_read},
                        message->data.result);
            }
            curl_multi_remove_handle(m_curl_multi.get(), curl_easy);
            curl_easy_cleanup(curl_easy);
        }
    } while (nullptr != message);
}
