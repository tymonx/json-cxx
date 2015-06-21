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
 * @file json/rpc/client.cpp
 *
 * @brief JSON client implementation
 * */

#include <json/rpc/error.hpp>

using json::rpc::Error;

constexpr const char Error::MSG_UNKNOWN_ERROR[];
constexpr const char Error::MSG_PARSE_ERROR[];
constexpr const char Error::MSG_INVALID_REQUEST[];
constexpr const char Error::MSG_METHOD_NOT_FOUND[];
constexpr const char Error::MSG_INVALID_PARAMS[];
constexpr const char Error::MSG_INTERNAL_ERROR[];
constexpr const char Error::MSG_SERVER_ERROR[];

Error::Error(Code code) : m_code{code} {
    switch (m_code) {
    case OK:
        break;
    case PARSE_ERROR:
        m_message = MSG_PARSE_ERROR;
        break;
    case INVALID_REQUEST:
        m_message = MSG_INVALID_REQUEST;
        break;
    case METHOD_NOT_FOUND:
        m_message = MSG_METHOD_NOT_FOUND;
        break;
    case INVALID_PARAMS:
        m_message = MSG_INVALID_PARAMS;
        break;
    case INTERNAL_ERROR:
        m_message = MSG_INTERNAL_ERROR;
        break;
    default:
        if ((-SERVER_ERROR >= m_code) && (m_code <= -SERVER_ERROR_MAX)) {
            m_message = MSG_SERVER_ERROR;
        }
        else {
            m_message = MSG_UNKNOWN_ERROR;
        }
        break;
    }
}

Error::Error(Code code, const Message& message, const Data& data) :
        m_code{code}, m_message{message}, m_data{data} { }

Error::Error(Code code, const char* message, const Data& data) :
        m_code{code}, m_message{message}, m_data{data} { }

Error::~Error() { }
