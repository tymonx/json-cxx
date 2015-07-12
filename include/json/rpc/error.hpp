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
 * @file json/rpc/error.hpp
 *
 * @brief JSON RPC error interface
 * */

#ifndef JSON_CXX_RPC_ERROR_HPP
#define JSON_CXX_RPC_ERROR_HPP

#include <json/json.hpp>
#include <string>
#include <exception>

namespace json {
namespace rpc {

class Error : public std::exception {
public:
    using Message = std::string;
    using Data = json::Value;
    using Code = std::int32_t;

    static const Code OK = 0;
    static const Code PARSE_ERROR       = -32700;
    static const Code INVALID_REQUEST   = -32600;
    static const Code METHOD_NOT_FOUND  = -32601;
    static const Code INVALID_PARAMS    = -32602;
    static const Code INTERNAL_ERROR    = -32603;
    static const Code SERVER_ERROR      = -32000;
    static const Code SERVER_ERROR_MAX  = -32099;

    static constexpr const char MSG_UNKNOWN_ERROR[] = "Unknown error";
    static constexpr const char MSG_PARSE_ERROR[] = "Parse error";
    static constexpr const char MSG_INVALID_REQUEST[] = "Invalid Request";
    static constexpr const char MSG_METHOD_NOT_FOUND[] = "Method not found";
    static constexpr const char MSG_INVALID_PARAMS[] = "Invalid params";
    static constexpr const char MSG_INTERNAL_ERROR[] = "Internal error";
    static constexpr const char MSG_SERVER_ERROR[] = "Server error";

    Error(Code code = OK);
    Error(Code code, const Message& message,
            const Data& data = Value::Type::NIL);
    Error(Code code, const char* message,
            const Data& data = Value::Type::NIL);

    Error(const Error&) = default;
    Error(Error&&) = default;
    Error& operator=(const Error&) = default;
    Error& operator=(Error&&) = default;

    virtual ~Error();

    virtual const char* what() const noexcept { return m_message.c_str(); }

    Code get_code() const { return m_code; }
    const Message& get_message() const { return m_message; }
    const Data& get_data() const { return m_data; }

    bool operator!() const { return OK == m_code; }
    operator bool() const { return OK != m_code; }

    bool operator==(Code code) const { return m_code == code; }
    bool operator!=(Code code) const { return m_code != code; }
private:
    Code m_code{OK};
    Message m_message{""};
    Data m_data = Value::Type::NIL;
};

}
}

static inline bool operator==(json::rpc::Error::Code code,
        const json::rpc::Error& error) { return error == code; }

static inline bool operator!=(json::rpc::Error::Code code,
        const json::rpc::Error& error) { return error != code; }

#endif /* JSON_CXX_RPC_ERROR_HPP */
