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
 * @file json/rpc/client/http_client.hpp
 *
 * @brief HTTP JSON RPC client interface
 * */

#ifndef JSON_CXX_RPC_HTTP_CLIENT_HPP
#define JSON_CXX_RPC_HTTP_CLIENT_HPP

#include <json/rpc/client.hpp>
#include <json/rpc/client/http_settings.hpp>

namespace json {
namespace rpc {
namespace client {

/*!
 * JSON Client class
 * */
class HttpClient : public json::rpc::Client {
public:
    using Url = std::string;

    HttpClient(const Url& url, const HttpSettings& settings,
            Proactor& proactor) : Client{proactor}, m_url{url},
        m_settings{settings} { }

    virtual ~HttpClient();

    const Url& get_url() const { return m_url; }

    HttpSettings& get_settings() { return m_settings; }

    const HttpSettings& get_settings() const { return m_settings; }
private:
    Url m_url{};
    HttpSettings m_settings{};
};

}
}
}

#endif /* JSON_CXX_RPC_HTTP_CLIENT_HPP */
