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
#include <json/rpc/time.hpp>
#include <json/rpc/error.hpp>

#include <string>
#include <functional>
#include <future>

namespace json {
namespace rpc {

/* Forward declarations */
namespace client {
    class HttpProtocol;
    class Proactor;
}

/*!
 * JSON Client class
 * */
class Client {
public:
    using NotificationCallback = std::function<void(Client*, const Error&)>;
    using NotificationFuture = std::future<void>;
    using MethodCallback = std::function<void(Client*, const Value&, const Error&)>;
    using MethodFuture = std::future<json::Value>;

    Client(const client::HttpProtocol& protocol);

    Client(const Client&) = delete;
    Client(Client&&) = delete;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&) = delete;

    ~Client();

    /*!
     * @brief Call JSON-RPC method
     *
     * @param[in]   name        Method name
     * @param[in]   params      Method input parameter
     * @param[out]  result      Method output parameter
     * */
    void method(const std::string& name, const json::Value& params,
            json::Value& result) { result = method(name, params).get(); }

    /*!
     * @brief Call JSON-RPC method
     *
     * @param[in]   name        Method name
     * @param[in]   params      Method input parameter
     * @param[out]  result      Method output parameter
     * */
    void method(const std::string& name, const json::Value& params,
            MethodFuture& result) { result = method(name, params); }

    /*!
     * @brief Call JSON-RPC method
     *
     * @param[in]   name        Method name
     * @param[in]   params      Method input parameter
     *
     * @return  JSON-RPC result from method by future
     * */
    MethodFuture method(const std::string& name, const json::Value& params);

    /*!
     * @brief Call JSON-RPC method
     *
     * @param[in]   name        Method name
     * @param[in]   params      Method input parameter
     * */
    void method(const std::string& name, const json::Value& params,
            MethodCallback result);

    /*!
     * @brief Call JSON-RPC notification
     *
     * @param[in]   name        Method name
     * @param[in]   params      Method input parameter
     * */
    void notification(const std::string& name, const json::Value& params,
            NotificationFuture& result) {
        result = notification(name, params);
    }

    NotificationFuture notification(const std::string& name,
            const json::Value& params);

    void notification(const std::string& name, const json::Value& params,
            NotificationCallback result);

private:
    Client* const m_id{nullptr};
    client::Proactor& m_proactor;
};

}
}

#endif /* JSON_CXX_RPC_CLIENT_HPP */
