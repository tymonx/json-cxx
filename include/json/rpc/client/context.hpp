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
 * @file json/rpc/client/context.hpp
 *
 * @brief JSON client message interface
 * */

#ifndef JSON_CXX_RPC_CLIENT_CONTEXT_HPP
#define JSON_CXX_RPC_CLIENT_CONTEXT_HPP

#include <json/rpc/client/event.hpp>

#include <future>

namespace json {
namespace rpc {
namespace client {

class Context : public Event {
public:
    using ClosedNotify = std::promise<void>;

    enum State {
        UNKNOWN = 0,
        RUNNING,
        CLOSING
    };

    Context(Client* client);
    virtual ~Context();

    bool check(const Client* client) const { return get_client() == client; }

    virtual void dispatch_event(Event* event) = 0;

    void set_state(State state) { m_state = state; }
    State get_state() const { return m_state; }

    void set_closed_notify(ClosedNotify&& closed_notify) {
        m_closed_notify = std::move(closed_notify);
    }
    void call_closed_notify() {
        m_closed_notify.set_value();
    }
private:
    ClosedNotify m_closed_notify{};
    State m_state{UNKNOWN};
};

} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_CONTEXT_HPP */
