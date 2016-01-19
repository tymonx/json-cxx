/*!
 * @copyright
 * Copyright (c) 2016, Tymoteusz Blazejczyk
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
 * @file parser_error.hpp
 *
 * @brief JSON parser error interface
 * */

#ifndef JSON_CXX_PARSER_ERROR_HPP
#define JSON_CXX_PARSER_ERROR_HPP

#include <json/types.hpp>
#include <json/exception.hpp>

namespace json {

class ParserError : public Exception {
public:
    /*! Error parsing codes */
    enum Code {
        NONE,
        BAD_ALLOCATION,
        EMPTY_DOCUMENT,
        END_OF_FILE,
        EXTRA_CHARACTER,
        STACK_LIMIT_REACHED,
        MISS_VALUE,
        MISS_QUOTE,
        MISS_COLON,
        MISS_CURLY_CLOSE,
        MISS_SQUARE_CLOSE,
        NOT_MATCH_NULL,
        NOT_MATCH_TRUE,
        NOT_MATCH_FALSE,
        INVALID_WHITESPACE,
        INVALID_ESCAPE,
        INVALID_UNICODE,
        INVALID_NUMBER_INTEGER,
        INVALID_NUMBER_FRACTION,
        INVALID_NUMBER_EXPONENT
    };

    ParserError() :
        m_code{Code::NONE},
        m_position{nullptr}
    { }

    ParserError(Code code, const Char* position) :
        m_code{code},
        m_position{position}
    { }

    ParserError(const ParserError&) = default;
    ParserError(ParserError&&) = default;
    ParserError& operator=(const ParserError&) = default;
    ParserError& operator=(ParserError&&) = default;

    virtual const char* what() const noexcept override;

    Code code() const {
        return m_code;
    }

    const Char* position() const {
        return m_position;
    }

    bool operator!() const {
        return Code::NONE == m_code;
    }

    operator bool() const {
        return Code::NONE != m_code;
    }

    void clear() {
        m_code = Code::NONE;
    }

    virtual ~ParserError();
private:
    Code m_code;
    const Char* m_position;
};

}

#endif /* JSON_CXX_PARSER_ERROR_HPP */
