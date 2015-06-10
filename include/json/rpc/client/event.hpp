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

/* Client events */
#include "event/call_method.hpp"

#include <future>

namespace json {
namespace rpc {

class Client;

namespace client {

struct Event : public json::rpc::ListItem {
    using PromiseStatus = std::promise<int>;
    using FutureStatus = std::future<int>;

    enum class Type {
        UNDEFINED = 0,
        CALL_METHOD,
        CALL_METHOD_ASYNC,
        CREATE_CONTEXT,
        DESTROY_CONTEXT,
        OPEN_CONNECTION,
        CLOSE_CONNECTION
    };

    union Request {
        struct RequestCallMethod call_method;
        struct RequestCallMethodAsync call_method_async;
    };

    union Response {
        struct ResponseCallMethod call_method;
    };

    union Data {
        union Request request;
        union Response response;

        ~Data() { }
    };

    Client* client;
    Type type;
    Data data;
    PromiseStatus status;
    struct {
        unsigned auto_remove : 1;
    } flag;

    ~Event() {
    
    }
};

static inline
Event::FutureStatus get_future_status(struct Event& event) {
    return event.status.get_future();
}

} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_EVENT_HPP */
