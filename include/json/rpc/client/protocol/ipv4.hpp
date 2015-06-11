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
 * @file json/rpc/client/protocol/ipv4.hpp
 *
 * @brief JSON client protocol IPv4 protocol
 * */

#ifndef JSON_CXX_RPC_CLIENT_PROTOCOL_IPv4_HPP
#define JSON_CXX_RPC_CLIENT_PROTOCOL_IPv4_HPP

#include <json/rpc/client/protocol.hpp>

#include <string>

namespace json {
namespace rpc {
namespace client {
namespace protocol {

class IPv4 : public json::rpc::client::Protocol {
public:
    static constexpr const char DEFAULT_ADDRESS[] = "127.0.0.1";

    static constexpr const std::uint16_t DEFAULT_PORT = 80;

    IPv4(const std::string& address = DEFAULT_ADDRESS,
            std::uint16_t port = DEFAULT_PORT)
        : Protocol(ProtocolType::IPv4), m_address{address}, m_port{port} { }
private:
    std::string m_address{DEFAULT_ADDRESS};
    std::uint16_t m_port{DEFAULT_PORT};
};

} /* protocol */
} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_PROTOCOL_IPv4_HPP */
