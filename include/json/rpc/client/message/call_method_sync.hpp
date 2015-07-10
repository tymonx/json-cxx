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
 * @file json/rpc/client/call_method_sync.hpp
 *
 * @brief JSON client call method event
 * */

#ifndef JSON_CXX_RPC_CLIENT_CALL_METHOD_SYNC_HPP
#define JSON_CXX_RPC_CLIENT_CALL_METHOD_SYNC_HPP

#include <json/json.hpp>
#include <json/rpc/error.hpp>
#include <json/rpc/client/message.hpp>

#include <string>
#include <future>

namespace json {
namespace rpc {
namespace client {
namespace message {

class CallMethodSync : public Message {
public:
    CallMethodSync(Client* client,
            const std::string& name, const Value& params);

    virtual ~CallMethodSync() final;

    void set_id(const Value& id) { m_id = id; }

    const Value& get_id() const { return m_id; }

    std::string& get_response() { return m_response; }

    const std::string& get_response() const { return m_response; }

    const std::string& get_name() const { return m_name; }

    const Value& get_params() const { return m_params; }

    void set_result(const Value& result) { m_result.set_value(result); }

    void set_exception(const std::exception_ptr& ptr) {
        m_result.set_exception(ptr);
    }

    std::future<Value> get_result() { return m_result.get_future(); }
private:
    std::string m_name{};
    Value m_params{};
    Value m_id{};
    std::string m_response{};
    std::promise<Value> m_result{};
};

} /* message */
} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_CALL_METHOD_SYNC_HPP */
