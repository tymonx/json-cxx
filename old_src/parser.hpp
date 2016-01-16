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
 * @file parser.hpp
 *
 * @brief JSON parser interface
 * */

#ifndef JSON_CXX_PARSER_HPP
#define JSON_CXX_PARSER_HPP

#include <json/value.hpp>
#include <json/deserializer_error.hpp>

#include <cstdint>

namespace json {

class Parser {
public:
    Parser(const char* str, std::size_t length, std::size_t limit);

    void parsing(Value& value);
private:
    const char* m_begin;
    const char* m_current;
    const char* m_end;
    std::size_t m_limit;

    void read_object(Value& value);
    void read_object_member(Value& value, std::size_t& count);
    void read_string(String& str);
    void read_string_unicode(String& str);
    void read_string_escape(String& str);
    void read_value(Value& value);
    void read_array(Value& value);
    void read_array_element(Value& value, std::size_t& count);
    void read_colon();
    void read_quote();
    void read_true(Value& value);
    void read_false(Value& value);
    void read_null(Value& value);
    void read_number(Value& value);
    void read_number_digit(Uint64& str);
    void read_number_integer(Number& number);
    void read_number_fractional(Number& number);
    void read_number_exponent(Number& number);
    void read_unicode(const char** pos, std::uint32_t& code);
    void read_whitespaces(bool enable_error = true);

    Size count_string_chars() const;

    [[noreturn]] void throw_error(DeserializerError::Code code);
};

}

#endif /* JSON_CXX_PARSER_HPP */
