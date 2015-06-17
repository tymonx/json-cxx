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
 * @file json/rpc/client/proactor.hpp
 *
 * @brief JSON client reactor interface
 * */

#ifndef JSON_CXX_RPC_CLIENT_HTTP_PROACTOR_HPP
#define JSON_CXX_RPC_CLIENT_HTTP_PROACTOR_HPP

#include <json/rpc/client/proactor.hpp>
#include <json/rpc/client/http_context.hpp>

#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <sys/select.h>

namespace json {
namespace rpc {
namespace client {

class HttpProactor : public Proactor {
public:
    static const constexpr unsigned DEFAULT_MAX_PIPELINE_LENGTH = 8;

    static Proactor& get_instance() {
        static HttpProactor proactor{};
        return proactor;
    }

    HttpProactor();

    virtual ~HttpProactor() final;

    virtual void push_event(EventPtr event) final;

    void setup_context(HttpContext& context);

    constexpr unsigned get_max_pipeline_length() const {
        return DEFAULT_MAX_PIPELINE_LENGTH;
    }
private:
    struct CurlMultiDeleter {
        void operator ()(void*);
    };

    using CurlMultiPtr = std::unique_ptr<void, CurlMultiDeleter>;

    inline void notify();
    inline void get_events();
    inline void waiting_for_events();
    inline void demultiplexing_events();
    inline void handle_create_context(EventList::iterator& it);
    inline void handle_destroy_context(EventList::iterator& it);
    inline void handle_events_context(EventList::iterator& it);

    void context_processing(HttpContext& context);
    void read_processing();

    void task();

    CurlMultiPtr m_curl_multi{nullptr};

    volatile std::atomic<bool> m_task_done{false};
    std::thread m_thread{};

    uint64_t m_event{0};
    int m_eventfd{0};

    bool m_fds_changed{true};
    fd_set m_fds_read{};
    fd_set m_fds_write{};
    fd_set m_fds_except{};
    int m_fds_max{-1};

    EventList m_events{};
    EventList m_events_background{};
    HttpContextList m_contexts{};

    std::mutex m_mutex{};
};

}
}
}

#endif /* JSON_CXX_RPC_CLIENT_HTTP_PROACTOR_HPP */
