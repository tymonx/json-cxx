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
 * @file json/rpc/client/call_method.hpp
 *
 * @brief JSON client message interface
 *
 * Message used for communication between clients and proactor
 * */

#ifndef JSON_CXX_RPC_CLIENT_CALL_METHOD_HPP
#define JSON_CXX_RPC_CLIENT_CALL_METHOD_HPP

#include <json/json.hpp>
#include <json/rpc/error.hpp>
#include <json/rpc/client/request.hpp>

#include <string>
#include <future>

namespace json {
namespace rpc {
namespace client {

class CallMethod : public Request {
public:
    using Callback = std::function<void(Client*, const json::Value&, const Error&)>;

    CallMethod(Client* client,
            const std::string& name, const Value& value) :
        Request{EventType::CALL_METHOD, client, name, value} { }

    CallMethod(Client* client,
            const std::string& name, const Value& value, Callback callback) :
        Request{EventType::CALL_METHOD_ASYNC, client, name, value},
        m_callback{callback} { }

    virtual ~CallMethod() final;

    Callback m_callback{nullptr};
    std::promise<json::Value> m_result{};

    void set_id(const Value& id) { m_id = id; }

    const Value& get_id() const { return m_id; }

    void processing();

    bool valid_response(const Value& value);
private:
    Value m_id{};
};

} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_CALL_METHOD_HPP */
