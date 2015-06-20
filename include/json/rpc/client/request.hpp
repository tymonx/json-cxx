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
 * @file json/rpc/client/message.hpp
 *
 * @brief JSON client message interface
 * */

#ifndef JSON_CXX_RPC_CLIENT_REQUEST_HPP
#define JSON_CXX_RPC_CLIENT_REQUEST_HPP

#include <json/json.hpp>
#include <json/rpc/client/event.hpp>

#include <string>

namespace json {
namespace rpc {
namespace client {

class Request : public Event {
public:
    Request(EventType type, Client* client,
            const std::string& name, const Value& value) :
        Event{type, client}, m_response{}, m_name{name}, m_value{value} { }

    virtual ~Request();

    void set_response(const std::string& response) { m_response = response; }

    const std::string& get_response() const { return m_response; }

    std::string& get_response() { return m_response; }

    void set_value(const Value& value) { m_value = value; }

    const Value& get_value() const { return m_value; }

    const std::string& get_name() const { return m_name; }
private:
    std::string m_response;
    std::string m_name;
    Value m_value;
};

} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_REQUEST_HPP */
