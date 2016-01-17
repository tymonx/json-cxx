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
#include <limits>
#include <string>
#include <cstring>
#include <utility>

namespace json {

class Parser {
public:
    static constexpr Size DEFAULT_LIMIT_PER_OBJECT =
        std::numeric_limits<Size>::max();

    Parser() { }

    Parser(const Char* begin, const Char* end) :
        m_begin{begin},
        m_end{end},
        m_pos{begin}
    { }

    Parser(const Char* begin, const Char* end, Value& value) :
        Parser(begin, end) { parsing(value); }

    Parser(const Char* str) :
        Parser(str, str + std::strlen(str)) { }

    Parser(const Char* str, Value& value) :
        Parser(str, str + std::strlen(str)) { parsing(value); }

    Parser(const Char* str, Size size) :
        Parser(str, str + size) { }

    Parser(const Char* str, Size size, Value& value) :
        Parser(str, str + size) { parsing(value); }

    template<Size N>
    Parser(const Char str[N]) :
        Parser(str, str + N - 1) { }

    template<Size N>
    Parser(const Char str[N], Value& value) :
        Parser(str, str + N - 1) { parsing(value); }

    Parser(const std::string& str) :
        Parser(str.data(), str.data() + str.length()) { }

    Parser(const std::string& str, Value& value) :
        Parser(str.data(), str.data() + str.length()) { parsing(value); }

    void parsing(Value& value);

    void parsing(const Char* begin, const Char* end, Value& value) {
        m_pos = m_begin = begin;
        m_end = end;
        parsing(value);
    }

    void parsing(const Char* str, Value& value) {
        parsing(str, str + std::strlen(str), value);
    }

    void parsing(const Char* str, Size size, Value& value) {
        parsing(str, str + size, value);
    }

    template<Size N>
    void parsing(const Char str[N], Value& value) {
        parsing(str, str + N - 1, value);
    }

    void parsing(const std::string& str, Value& value) {
        parsing(str.data(), str.data() + str.length(), value);
    }

    Value parsing() {
        Value value;
        parsing(value);
        return value;
    }

    void set_limit(Size limit = DEFAULT_LIMIT_PER_OBJECT) {
        m_limit = limit;
    }

    Size get_limit() const {
        return m_limit;
    }

    void enable_stream_mode(bool enable) {
        m_stream_mode = enable;
    }
private:
    template<Size N>
    using ParseFunctions = std::array<
        std::pair<int, void(Parser::*)(Value&)>, N>;

    static constexpr Size NUM_PARSE_FUNCTIONS = 17;
    static const ParseFunctions<NUM_PARSE_FUNCTIONS> m_parse_functions;

    Size m_limit{DEFAULT_LIMIT_PER_OBJECT};
    bool m_stream_mode{false};
    const Char* m_begin{nullptr};
    const Char* m_end{nullptr};
    const Char* m_pos{nullptr};

    void read_whitespaces();
    void read_value(Value& value);
    void read_array(Value& value);
    void read_array_element(Value& value, Size& count);
    void read_object(Value& value);
    void read_object_member(Value& value, Size& count);
    void read_string(Value& value);
    void read_number(Value& value);
    void read_true(Value& value);
    void read_false(Value& value);
    void read_null(Value& value);
    void read_colon();
    void read_quote();
};

}

static inline
json::Parser& operator>>(json::Parser& parser, json::Value& value) {
    parser.parsing(value);
    return parser;
}

static inline
json::Parser operator>>(const json::Char* str, json::Value& value) {
    return json::Parser(str, str + std::strlen(str), value);
}

template<json::Size N>
json::Parser operator>>(const json::Char str[N], json::Value& value) {
    return json::Parser(str, str + N - 1, value);
}

static inline
json::Parser operator>>(const std::string& str, json::Value& value) {
    return json::Parser(str.data(), str.data() + str.length(), value);
}

#endif /* JSON_CXX_PARSER_HPP */
