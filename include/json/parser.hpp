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
#include <json/allocator.hpp>
#include <json/parser_error.hpp>

#include <csetjmp>

namespace json {

class Parser {
public:
    static constexpr Size DEFAULT_LIMIT_PER_OBJECT{0};

    Parser() { }

    Parser(const Char* begin, const Char* end) :
        m_pos{begin},
        m_end{end}
    { }

    Parser(const Char* begin, const Char* end, Value& value) :
        Parser(begin, end) { parsing(value); }

    Parser(const Char* str);

    Parser(const Char* str, Value& value) : Parser(str)
        { parsing(value); }

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

    Parser(const Parser&) = default;
    Parser(Parser&&) = default;
    Parser& operator=(const Parser&) = default;
    Parser& operator=(Parser&&) = default;

    void parsing(Value& value);

    void parsing(const Char* begin, const Char* end, Value& value) {
        m_pos = begin;
        m_end = end;
        parsing(value);
    }

    void parsing(const Char* str, Value& value);

    void parsing(const Char* str, Size size, Value& value) {
        parsing(str, str + size, value);
    }

    template<Size N>
    void parsing(const Char str[N], Value& value) {
        parsing(str, str + N - 1, value);
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

    bool is_error() const {
        return ParserError::NONE != m_error_code;
    }

    ParserError get_error() const {
        return {m_error_code, const_cast<const Char*>(m_error_position)};
    }
private:
    struct ParseFunction {
        int code;
        void (Parser::*parse)(Value&);
    };

    static const ParseFunction m_parse_functions[];

    /* Modified locals in setjmp scope must be volatile */
    std::jmp_buf m_jump_buffer{};
    volatile const Char* m_error_position{};
    volatile ParserError::Code m_error_code{};

    /* Parser configuration */
    bool m_stream_mode{false};
    Size m_limit{DEFAULT_LIMIT_PER_OBJECT};

    Allocator* m_allocator{nullptr};
    const Char* m_pos{nullptr};
    const Char* m_end{nullptr};

    /* Parser processing members */
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

    inline void read_whitespaces();
    inline void read_colon();
    inline void read_quote();

    /* Number processing methods */
    struct NumberInfo;

    inline void read_number_integral(NumberInfo&);
    inline void read_number_fractional(NumberInfo&);
    inline void read_number_exponent(NumberInfo&);
    inline void read_number_digits(NumberInfo&);
    inline bool write_number_integer(const NumberInfo&, Number& number);
    inline void write_number_double(const NumberInfo&, Number& number);

    /* String processing */
    inline Char* read_string_unicode(Char* str, int& ch);
    inline unsigned read_string_unicode();
    inline Size read_string_count();

    inline void stack_guard();

    [[noreturn]]
    void throw_error(ParserError::Code code);
};

}

static inline
json::Parser& operator>>(json::Parser& parser, json::Value& value) {
    parser.parsing(value);
    return parser;
}

static inline
json::Parser operator>>(const char* str, json::Value& value) {
    return json::Parser(str, value);
}

template<json::Size N>
json::Parser operator>>(const char str[N], json::Value& value) {
    return json::Parser(str, str + N - 1, value);
}

#endif /* JSON_CXX_PARSER_HPP */
