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
 * @file deserializer.cpp
 *
 * @brief JSON deserializer implementation
 * */

#include "json/deserializer_error.hpp"

#include <array>

using json::DeserializerError;

static const std::array<const char*, 17> g_error_codes{{
    "No error",
    "End of file reached",
    "Stack limit reached. Increase limit",
    "Missing value in array/member",
    "Missing quote '\"' for string",
    "Missing colon ':' in member pair",
    "Missing comma ',' or closing curly '}' for object",
    "Missing comma ',' or closing square ']' for array",
    "Did you mean 'null'?",
    "Did you mean 'true'?",
    "Did you mean 'false'?",
    "Invalid whitespace character",
    "Invalid escape character",
    "Invalid unicode",
    "Invalid number integer part",
    "Invalid number fractional part",
    "Invalid number exponent part"
}};

DeserializerError::DeserializerError(Code code, std::size_t offset) :
    m_code{code}, m_offset{offset}  { }

DeserializerError::~DeserializerError() { }

const char* DeserializerError::what() const noexcept {
    return g_error_codes[m_code];
}
