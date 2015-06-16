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
 * @file json/rpc/client/event.hpp
 *
 * @brief JSON client message interface
 *
 * Message used for communication between clients and proactor
 * */

#ifndef JSON_CXX_RPC_CLIENT_EVENT_HPP
#define JSON_CXX_RPC_CLIENT_EVENT_HPP

#include <json/rpc/list.hpp>
#include <json/rpc/time.hpp>
#include <json/rpc/error.hpp>
#include <json/rpc/client/event_type.hpp>

namespace json {
namespace rpc {

class Client;

namespace client {

class Event : public json::rpc::ListItem {
public:
    using Options = std::uint16_t;

    enum Option : Options {
        NO_OPTIONS          = 0x0000,
        AUTO_REMOVE         = 0x0001,
        NOTIFY              = 0x0002
    };

    EventType get_type() const { return m_type; }

    const Client* get_client() const { return m_client; }

    void set_live_time_duration(Miliseconds miliseconds) {
        m_time_live = m_time_created + miliseconds;
    }

    const TimePoint& get_live_time_point() const { return m_time_live; }

    static void event_complete(Event* event, const Error& error = {Error::OK});

    virtual ~Event();

    Options options() { return m_options; }
protected:
    Event(EventType type, Client* client, Options options = NO_OPTIONS) :
        m_type(type), m_client(client), m_options(options),
        m_time_created(std::chrono::steady_clock::now()) { }
private:
    Event(const Event&) = delete;
    Event(Event&&) = delete;
    Event& operator=(const Event&) = delete;
    Event& operator=(Event&&) = delete;

    EventType m_type{EventType::UNDEFINED};
    Client* m_client{nullptr};
    Options m_options{NO_OPTIONS};

    /* Event time managment */
    TimePoint m_time_live{0_ms};
    TimePoint m_time_created{};

    friend void event_complete(Event* event);
};

} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_EVENT_HPP */
