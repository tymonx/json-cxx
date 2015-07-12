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
 * @file json/rpc/client/curl_proactor.hpp
 *
 * @brief CURL proactor
 * */

#ifndef JSON_CXX_RPC_CLIENT_CURL_PROACTOR_HPP
#define JSON_CXX_RPC_CLIENT_CURL_PROACTOR_HPP

#include <json/rpc/client/proactor.hpp>
#include <json/rpc/client/curl_context.hpp>
#include <json/rpc/client/executor.hpp>

#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <sys/select.h>

namespace json {
namespace rpc {
namespace client {

class CurlProactor : public Proactor {
public:
    static CurlProactor& get_instance() {
        static CurlProactor proactor{};
        return proactor;
    }

    CurlProactor();

    virtual ~CurlProactor() final;

    virtual void push_message(MessagePtr&& message) final;

    virtual void set_max_total_connections(MaxTotalConnections amount);
private:
    struct CurlMultiDeleter {
        void operator ()(void*);
    };

    using CurlMultiPtr = std::unique_ptr<void, CurlMultiDeleter>;

    inline void notify();
    void waiting_for_messages();
    void dispatch_messages();
    void create_context(MessageList::iterator& it);
    void destroy_context(MessageList::iterator& it);
    void context_message(MessageList::iterator& it);
    void context_processing();
    void read_processing();
    void task();

    CurlMultiPtr m_curl_multi{nullptr};

    volatile std::atomic<bool> m_task_done{false};
    std::thread m_thread{};

    int m_eventfd{0};
    int m_running_handles{0};

    MessageList m_messages{};
    MessageList m_messages_background{};
    CurlContextList m_contexts{};

    std::mutex m_mutex{};
};

}
}
}

#endif /* JSON_CXX_RPC_CLIENT_CURL_PROACTOR_HPP */
