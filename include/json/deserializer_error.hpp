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
 * @file deserializer_error.hpp
 *
 * @brief JSON deserializer error interface
 * */

#ifndef JSON_CXX_DESERIALIZER_ERROR_HPP
#define JSON_CXX_DESERIALIZER_ERROR_HPP

#include <exception>
#include <cstdint>

namespace json {

/*! JSON error parsing */
class DeserializerError : public std::exception {
public:
    /*! Error parsing codes */
    enum Code {
        NONE,
        END_OF_FILE,
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

    DeserializerError(Code code, std::size_t offset);

    DeserializerError(const DeserializerError&) = default;
    DeserializerError(DeserializerError&&) = default;
    DeserializerError& operator=(const DeserializerError&) = default;
    DeserializerError& operator=(DeserializerError&&) = default;

    /*!
     * @brief Return error explanatory string
     *
     * @return  When success return decoded error code as a human readable
     *          message, otherwise return empty string ""
     * */
    virtual const char* what() const noexcept;

    Code get_code() const { return m_code; }

    std::size_t get_offset() const { return m_offset; }

    virtual ~DeserializerError();
private:
    /*! Error parsing code */
    Code m_code{NONE};
    /*! Column number indicative error */
    std::size_t m_offset{0};
};

}

#endif /* JSON_CXX_DESERIALIZER_ERROR_HPP */
