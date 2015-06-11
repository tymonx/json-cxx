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
 * @file json/rpc/client.hpp
 *
 * @brief JSON client interface
 * */

#ifndef JSON_CXX_RPC_CLIENT_HPP
#define JSON_CXX_RPC_CLIENT_HPP

#include <json/json.hpp>

#include <string>
#include <functional>

namespace json {
namespace rpc {

/* Forward declarations */
namespace client {
    class Protocol;
    class Proactor;
}

/*!
 * JSON Client class
 * */
class Client {
public:
    using ResultCallback = std::function<void(const json::Value&)>;

    Client(const client::Protocol& protocol);

    ~Client();

    /*!
     * @brief Call JSON-RPC method
     *
     * @param[in]   name        Method name
     * @param[in]   params      Method input parameter
     * @param[out]  result      Method output parameter
     * */
    void method(const std::string& name, const json::Value& params,
            json::Value& result) { result = method(name, params); }

    /*!
     * @brief Call JSON-RPC method
     *
     * @param[in]   name        Method name
     * @param[in]   params      Method input parameter
     *
     * @return  JSON-RPC result from method by future
     * */
    json::Value method(const std::string& name, const json::Value& params);

    /*!
     * @brief Call JSON-RPC method
     *
     * @param[in]   name        Method name
     * @param[in]   params      Method input parameter
     * */
    void method(const std::string& name, const json::Value& params,
            ResultCallback result);

    /*!
     * @brief Call JSON-RPC notification
     *
     * @param[in]   name        Method name
     * @param[in]   params      Method input parameter
     * */
    void notification(const std::string& name, const json::Value& params);

private:
    Client() = delete;

    client::Proactor& m_proactor;
};

}
}

#endif /* JSON_CXX_RPC_CLIENT_HPP */
