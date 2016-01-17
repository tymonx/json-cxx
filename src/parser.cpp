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
 * @file parser.cpp
 *
 * @brief JSON parser interface
 * */

#include <json/parser.hpp>
#include <json/parser_error.hpp>

#include "parser_number.hpp"
#include "parser_string.hpp"

#include <memory>
#include <cstring>

using json::Char;
using json::Size;
using json::Value;
using json::Parser;
using json::ParserNumber;
using json::ParserString;
using Error = json::ParserError;

static constexpr Char JSON_NULL[] = "null";
static constexpr Char JSON_TRUE[] = "true";
static constexpr Char JSON_FALSE[] = "false";

static constexpr Size JSON_NULL_LENGTH = 4;
static constexpr Size JSON_TRUE_LENGTH = 4;
static constexpr Size JSON_FALSE_LENGTH = 5;

const Parser::ParseFunction Parser::m_parse_functions[]{
    {'{', &Parser::read_object},
    {'[', &Parser::read_array},
    {'"', &Parser::read_string},
    {'t', &Parser::read_true},
    {'f', &Parser::read_false},
    {'n', &Parser::read_null},
    {'-', &Parser::read_number},
    {'0', &Parser::read_number},
    {'1', &Parser::read_number},
    {'2', &Parser::read_number},
    {'3', &Parser::read_number},
    {'4', &Parser::read_number},
    {'5', &Parser::read_number},
    {'6', &Parser::read_number},
    {'7', &Parser::read_number},
    {'8', &Parser::read_number},
    {'9', &Parser::read_number},
    {-1, nullptr}
};

static inline
bool is_whitespace(int ch) {
    return ((' '  == ch) || ('\n' == ch) || ('\r' == ch) || ('\t' == ch));
}

Parser::Parser(const Char* str) :
    Parser(str, str + std::strlen(str)) { }

void Parser::parsing(const Char* str, Value& value) {
    parsing(str, str + std::strlen(str), value);
}

void Parser::parsing(Value& value) {
    value.~Value();
    value.m_type = Value::NIL;

    read_whitespaces();
    if (m_pos < m_end) {
        read_value(value);
        read_whitespaces();

        if ((m_pos < m_end) && !m_stream_mode) {
            throw Error{Error::EXTRA_CHARACTER, m_pos};
        }
    }
    else { throw Error{Error::EMPTY_DOCUMENT, m_pos}; }
}

void Parser::read_value(Value& value) {
    std::memset(&value, 0, sizeof(0));

    read_whitespaces();
    if (m_pos < m_end) {
        int ch = *m_pos;
        const auto* p = m_parse_functions;
        while ((p->code != ch) && (p->parse != nullptr)) { ++p; }

        if (nullptr != p->parse) {
            (this->*(p->parse))(value);
        }
        else { throw Error{Error::MISS_VALUE, m_pos}; }
    }
    else { throw Error{Error::END_OF_FILE, m_end}; }
}

void Parser::read_array(Value& value) {
    ++m_pos;
    read_whitespaces();

    if (m_pos < m_end) {
        if (']' == *m_pos) {
            ++m_pos;
            value.m_type = Value::ARRAY;
        }
        else {
            Size count = 0;
            read_array_element(value, count);
        }
    }
    else { throw Error{Error::END_OF_FILE, m_end}; }
}

void Parser::read_array_element(Value& value, Size& count)  {
    if (m_limit) { stack_guard(); }

    char tmp_buffer[sizeof(Value)];
    Value* tmp = new (tmp_buffer) Value;

    read_value(*tmp);
    read_whitespaces();
    ++count;

    if (',' == *m_pos) {
        ++m_pos;
        read_array_element(value, count);
    }
    else if (']' == *m_pos) {
        ++m_pos;
        value.m_array.m_begin = new Value[count];
        value.m_array.m_end = value.m_array.m_begin + count;
        value.m_type = Value::ARRAY;
    }
    else { throw Error{Error::MISS_SQUARE_CLOSE, m_pos}; }

    std::memcpy(&value.m_array[--count], tmp, sizeof(Value));
}

void Parser::read_object(Value& value) {
    ++m_pos;
    read_whitespaces();

    if (m_pos < m_end) {
        if ('}' == *m_pos) {
            ++m_pos;
            value.m_type = Value::OBJECT;
        }
        else {
            Size count = 0;
            read_object_member(value, count);
        }
    }
    else { throw Error{Error::END_OF_FILE, m_end}; }
}

void Parser::read_object_member(Value& value, Size& count) {
    if (m_limit) { stack_guard(); }

    char key_buffer[sizeof(Value)];
    char tmp_buffer[sizeof(Value)];

    Value* key = new (key_buffer) Value;
    Value* tmp = new (tmp_buffer) Value;

    read_quote();
    read_string(*key);
    read_colon();
    read_value(*tmp);
    read_whitespaces();
    ++count;

    if (',' == *m_pos) {
        ++m_pos;
        read_object_member(value, count);
    }
    else if ('}' == *m_pos) {
        ++m_pos;
        value.m_object.m_begin = new Pair[count];
        value.m_object.m_end = value.m_object.m_begin + count;
        value.m_type = Value::OBJECT;
    }
    else { throw Error{Error::MISS_CURLY_CLOSE, m_pos}; }

    std::memcpy(&value.m_object[--count].key, &key->m_string, sizeof(String));
    std::memcpy(&value.m_object[count].value, tmp, sizeof(Value));
}

void Parser::read_string(Value& value) {
    Size count = ParserString::count_string_chars(++m_pos, m_end);
    std::unique_ptr<Char []> str{new Char[count + 1]};
    str[count] = '\0';

    ParserString parser(m_pos, m_end);
    parser.parsing(str.get());
    m_pos = parser.get_position();

    value.m_type = Value::STRING;
    value.m_string.m_begin = str.release();
    value.m_string.m_end = value.m_string.m_begin + count;
}

void Parser::read_number(Value& value) {
    ParserNumber parser(m_pos, m_end);
    parser.parsing(value.m_number);
    m_pos = parser.get_position();

    value.m_type = Value::NUMBER;
}

void Parser::read_true(Value& value) {
    if (m_pos + JSON_TRUE_LENGTH <= m_end) {
        if (!std::memcmp(m_pos, JSON_TRUE, JSON_TRUE_LENGTH)) {
            value.m_type = Value::BOOL;
            value.m_bool = true;
            m_pos += JSON_TRUE_LENGTH;
        }
        else { throw Error{ParserError::NOT_MATCH_TRUE, m_pos}; }
    }
    else { throw Error{ParserError::END_OF_FILE, m_end}; }
}

void Parser::read_false(Value& value) {
    if (m_pos + JSON_FALSE_LENGTH <= m_end) {
        if (!std::memcmp(m_pos, JSON_FALSE, JSON_FALSE_LENGTH)) {
            value.m_type = Value::BOOL;
            value.m_bool = false;
            m_pos += JSON_FALSE_LENGTH;
        }
        else { throw Error{Error::NOT_MATCH_FALSE, m_pos}; }
    }
    else { throw Error{ParserError::END_OF_FILE, m_end}; }
}

void Parser::read_null(Value& value) {
    if (m_pos + JSON_NULL_LENGTH <= m_end) {
        if (!std::memcmp(m_pos, JSON_NULL, JSON_NULL_LENGTH)) {
            value.m_type = Value::NIL;
            m_pos += JSON_NULL_LENGTH;
        }
        else { throw Error{ParserError::NOT_MATCH_NULL, m_pos}; }
    }
    else { throw Error{ParserError::END_OF_FILE, m_end}; }
}

void Parser::read_whitespaces() {
    while (m_pos < m_end) {
        if (is_whitespace(*m_pos)) { ++m_pos; }
        else { break; }
    }
}

void Parser::read_colon() {
    read_whitespaces();
    if (m_pos < m_end) {
        if (':' == *m_pos) { ++m_pos; }
        else { throw Error{ParserError::MISS_COLON, m_pos}; }
    }
    else { throw Error{Error::END_OF_FILE, m_end}; }
}

void Parser::read_quote() {
    read_whitespaces();
    if (m_pos < m_end) {
        if ('"' == *m_pos) { ++m_pos; }
        else { throw Error{ParserError::MISS_QUOTE, m_pos}; }
    }
    else { throw Error{Error::END_OF_FILE, m_end}; }
}

void Parser::stack_guard() {
    if (0 == --m_limit) { throw Error{Error::STACK_LIMIT_REACHED, m_pos}; }
}
