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
#include <json/rpc/list.hpp>

#include <iostream>
#include <algorithm>

using json::rpc::Error;
using json::rpc::client::Context;
using json::rpc::client::Proactor;

void Proactor::event_loop() {
    get_events();
    while (!m_events.empty()) {
        std::cout << "Proactor: event_loop" << std::endl;
        event_handling(static_cast<Event*>(m_events.pop()));
    }
}

void Proactor::get_events() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.splice(m_events_background);
}

void Proactor::event_handling(Event* event) {
    std::cout << "Proactor: Event handling: " << event << std::endl;
    if (EventType::CONTEXT == event->get_type()) {
        std::cout << "Proactor: add context" << std::endl;
        setup_context(static_cast<Context&>(*event));
        m_contexts.push(event);
    }
    else if (EventType::DESTROY_CONTEXT == event->get_type()) {
        std::cout << "Proactor: destroy context" << std::endl;
        delete m_contexts.remove(find_context(event->get_client()));
        Event::event_complete(static_cast<Event*>(event));
    }
    else {
        std::cout << "Proactor: event " << unsigned(event->get_type()) << std::endl;
        auto context = find_context(event->get_client());
        if (nullptr != context) {
            context->dispatch_event(event);
        }
        else {
            Event::event_complete(event, Error{Error::INTERNAL_ERROR,
                    "Client context doesn't exist"});
        }
    }
}

Context* Proactor::find_context(const Client* client) {
    return static_cast<Context*>(std::find_if(
        m_contexts.begin(),
        m_contexts.end(),
        [&client] (const ListItem& item) {
            return static_cast<const Context&>(item).check(client);
        }
    ).operator->());
}

Proactor::~Proactor() {
    event_loop();
    while (!m_contexts.empty()) {
        delete m_contexts.pop();
    }
}

void Proactor::push_event(Event* pevent) {
    std::cout << "Proactor: push_event " << pevent << std::endl;
    std::unique_lock<std::mutex> lock(m_mutex);
    m_events_background.push(pevent);
    lock.unlock();
    notify();
}
