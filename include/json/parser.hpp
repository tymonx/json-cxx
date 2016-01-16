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
 * @file parser.hpp
 *
 * @brief JSON parser interface
 * */

#ifndef JSON_CXX_PARSER_HPP
#define JSON_CXX_PARSER_HPP

#include <json/types.hpp>
#include <json/json.hpp>

#include <array>
#include <utility>

namespace json {

class Parser {
public:
    static const Size DEFAULT_LIMIT_PER_OBJECT;

    Parser();

    Parser(const Char* begin, const Char* end, Value& value) :
        Parser() { parsing(begin, end, value); }

    void parsing(const Char* begin, const Char* end, Value& value);
private:
    template<Size N>
    using ParseFunctions = std::array<
        std::pair<int, void(Parser::*)(Value&)>, N>;

    static constexpr Size NUM_PARSE_FUNCTIONS = 17;
    static const ParseFunctions<NUM_PARSE_FUNCTIONS> m_parse_functions;

    Size m_limit;
    const Char* m_begin;
    const Char* m_end;
    const Char* m_pos;

    void read_whitespaces();
    void read_value(Value& value);
    void read_array(Value& value);
    void read_array_element(Value& value, Size& count);
    void read_object(Value& value);
    void read_object_member(Value& value, Size& count);
    void read_string(Value& value);
    Char* read_string_unicode(Char* str, int& ch);
    void read_number(Value& value);
    void read_true(Value& value);
    void read_false(Value& value);
    void read_null(Value& value);
    void read_colon();
    void read_quote();
};

}

#endif /* JSON_CXX_PARSER_HPP */
