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
#include <json/rpc/client/event_type.hpp>

#include <future>

namespace json {
namespace rpc {

class Client;

namespace client {

class Event : public json::rpc::ListItem {
public:
    using Flags = std::uint16_t;

    enum Option : Flags {
        AUTO_REMOVE         = 0x0001,
        NOTIFY              = 0x0002
    };

    Event(Event&& other) :
        m_type(other.m_type),
        m_client(other.m_client),
        m_flags(other.m_flags),
        m_notify(std::move(other.m_notify))
    {
        other.m_type = EventType::UNDEFINED;
        other.m_client = nullptr;
        other.m_flags = 0;
    }

    EventType get_type() const { return m_type; }

    const Client* get_client() const { return m_client; }

    std::future<void> get_notify() { return m_notify.get_future(); }

    Flags get_flags() const { return m_flags; }
    void set_flags(Flags flags) { m_flags |= flags; }
    void clear_flags() { m_flags = 0; }
    void clear_flags(Flags flags) { m_flags &= Flags(~flags); }
    bool check_flags(Flags flags) { return (m_flags & flags) == flags; }
    bool check_flag(Flags flag) { return (m_flags & flag); }

    static void event_complete(Event* event);
protected:
    Event(EventType type, Client* client, const Flags& flags = {})
        : m_type(type), m_client(client), m_flags(flags) { }
private:
    Event() = delete;
    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    EventType m_type{EventType::UNDEFINED};
    Client* m_client{nullptr};
    Flags m_flags{0};
    std::promise<void> m_notify{};

    friend void event_complete(Event* event);
};

} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_EVENT_HPP */
