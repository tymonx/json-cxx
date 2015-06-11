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

#ifndef JSON_CXX_RPC_CLIENT_PROACTOR_HPP
#define JSON_CXX_RPC_CLIENT_PROACTOR_HPP

#include <json/rpc/list.hpp>
#include <json/rpc/client/event.hpp>
#include <json/rpc/client/event/context.hpp>

#include <mutex>
#include <atomic>
#include <thread>
#include <algorithm>
#include <condition_variable>

namespace json {
namespace rpc {
namespace client {

class Proactor {
public:
    using Context = event::Context;

    static Proactor& get_instance() {
        static Proactor proactor{};
        return proactor;
    }

    Proactor() : m_thread(std::thread{&Proactor::task, this}) { }

    ~Proactor();

    void push_event(Event* pevent) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_events_background.push(pevent);
        lock.unlock();
        m_cond_variable.notify_one();
    }
private:
    static Proactor* g_instance;

    void task();

    Context* find_context(const Client* client) {
        return static_cast<Context*>(std::find_if(
            m_contexts.begin(),
            m_contexts.end(),
            [&client] (const ListItem& item) {
                return static_cast<const Context&>(item).check(client);
            }
        ).operator->());
    }

    void event_loop();

    void inline event_handling(Event* event);

    volatile std::atomic<bool> m_task_done{false};

    json::rpc::List m_events{};
    json::rpc::List m_events_background{};
    json::rpc::List m_contexts{};

    std::condition_variable m_cond_variable{};
    std::thread m_thread{};
    std::mutex m_mutex{};
};

}
}
}

#endif /* JSON_CXX_RPC_CLIENT_PROACTOR_HPP */
