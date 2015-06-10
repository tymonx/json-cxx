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

using namespace json::rpc::client;

Proactor* Proactor::g_instance = nullptr;

void Proactor::task() {
    std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);

    while (!m_task_done) {
        lock.lock();
        if (m_events_background.empty()) {
            m_cond_variable.wait(lock);
        }
        m_events.splice(m_events_background);
        lock.unlock();

        while (!m_events.empty()) {
            event_handling(static_cast<Event*>(m_events.pop()));
        }
    }
}

void Proactor::event_handling(Event* event) {
    if (Event::Type::CREATE_CONTEXT == event->type) {
        m_contexts.push(new Context(event->client));
        event_complete(event);
    }
    else if (Event::Type::DESTROY_CONTEXT == event->type) {
        delete m_contexts.remove(find_context(event->client));
        event_complete(event);
    }
    else {
        auto context = find_context(event->client);
        if (nullptr != context) {
            context->dispatch_event(event);
        }
        else {
            event_complete(event, -1);
        }
    }
}
