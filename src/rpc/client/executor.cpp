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
 * @file json/rpc/client/executor.cpp
 *
 * @brief JSON client protocol IPv4 protocol
 * */

#include <json/rpc/client/executor.hpp>

#include <json/json.hpp>

#include <json/rpc/client/request.hpp>
#include <json/rpc/client/call_method.hpp>
#include <json/rpc/client/send_notification.hpp>

using json::Value;
using json::Deserializer;
using json::rpc::Error;
using json::rpc::client::Event;
using json::rpc::client::Request;
using json::rpc::client::Executor;
using json::rpc::client::CallMethod;
using json::rpc::client::SendNotification;

Executor::Executor(size_t threads) {
    m_threads.resize(threads);
    for (auto& thread : m_threads) {
        thread = std::thread{&Executor::task, this};
    }
}

Executor::~Executor() {
    m_stop = true;
    m_cond_variable.notify_all();
    for (auto& thread : m_threads) {
        if (thread.joinable()) { thread.join(); }
    }
    /* Finish all events */
    while (!m_events.empty()) { task(); }
}

void Executor::push_event(EventPtr&& event) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_events.push_back(std::move(event));
    lock.unlock();
    m_cond_variable.notify_all();
}

void Executor::task() {
    std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
    EventPtr event{nullptr};

    do {
        lock.lock();
        m_cond_variable.wait(lock,
            [this] { return !m_events.empty() || m_stop; });
        if (!m_events.empty()) {
            event = std::move(m_events.front());
            m_events.pop_front();
        }
        lock.unlock();

        if (nullptr != event) {
            event_dispatcher(event.get());
            event = nullptr;
        }
    } while(!m_stop);
}

static
void call_method(Event* _event) {
    CallMethod* event = static_cast<CallMethod*>(_event);
    event->processing();
    if (!event->get_error()) {
        event->m_result.set_value(event->get_value());
    }
    else {
        event->m_result.set_exception(
                std::make_exception_ptr(event->get_error()));
    }
}

static
void call_method_async(Event* _event) {
    CallMethod* event = static_cast<CallMethod*>(_event);
    event->processing();
    try {
        event->m_callback(event->get_client(), event->get_value(),
                event->get_error());
    }
    catch (...) { }
}

static
void send_notification(Event* _event) {
    SendNotification* event = static_cast<SendNotification*>(_event);
    event->processing();
    if (!event->get_error()) {
        event->m_result.set_value();
    }
    else {
        event->m_result.set_exception(
                std::make_exception_ptr(event->get_error()));
    }
}

static
void send_notification_async(Event* _event) {
    SendNotification* event = static_cast<SendNotification*>(_event);
    event->processing();
    try {
        event->m_callback(event->get_client(), event->get_error());
    }
    catch (...) { }
}

void Executor::execute(EventPtr&& event) {
    switch (event->get_type()) {
    case EventType::CALL_METHOD:
        call_method(event.get());
        break;
    case EventType::SEND_NOTIFICATION:
        send_notification(event.get());
        break;
    case EventType::CALL_METHOD_ASYNC:
        if (nullptr != static_cast<CallMethod&>(*event).m_callback) {
            push_event(std::move(event));
        }
        break;
    case EventType::SEND_NOTIFICATION_ASYNC:
        if (nullptr != static_cast<SendNotification&>(*event).m_callback) {
            push_event(std::move(event));
        }
        break;
    case EventType::DESTROY_CONTEXT:
    case EventType::CREATE_CONTEXT:
    case EventType::UNDEFINED:
    default:
        break;
    }
}

void Executor::event_dispatcher(Event* event) {
    switch (event->get_type()) {
    case EventType::CALL_METHOD_ASYNC:
        call_method_async(event);
        break;
    case EventType::SEND_NOTIFICATION_ASYNC:
        send_notification_async(event);
        break;
    case EventType::CALL_METHOD:
    case EventType::SEND_NOTIFICATION:
    case EventType::DESTROY_CONTEXT:
    case EventType::CREATE_CONTEXT:
    case EventType::UNDEFINED:
    default:
        break;
    }
}
