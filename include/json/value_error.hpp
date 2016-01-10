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

#ifndef JSON_CXX_VALUE_ERROR_HPP
#define JSON_CXX_VALUE_ERROR_HPP

#include <exception>

namespace json {

/*! JSON error parsing */
class ValueError : public std::exception {
public:
    /*! Error parsing codes */
    enum Code {
        NONE,
        NOT_NULL,
        NOT_STRING,
        NOT_NUMBER,
        NOT_BOOLEAN,
        NOT_ARRAY,
        NOT_OBJECT
    };

    ValueError(Code code);

    ValueError(const ValueError&) = default;
    ValueError(ValueError&&) = default;
    ValueError& operator=(const ValueError&) = default;
    ValueError& operator=(ValueError&&) = default;

    /*!
     * @brief Return error explanatory string
     *
     * @return  When success return decoded error code as a human readable
     *          message, otherwise return empty string ""
     * */
    virtual const char* what() const noexcept;

    Code get_code() const { return m_code; }

    virtual ~ValueError();
private:
    /*! Error parsing code */
    Code m_code{NONE};
};

}

#endif /* JSON_CXX_VALUE_ERROR_HPP */
