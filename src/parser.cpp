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

#include "parser.hpp"

#include <array>
#include <utility>
#include <cstring>

using json::Char;
using json::Size;
using json::Value;
using json::Parser;
using json::ParserError;

static constexpr const Char JSON_NULL[] = "null";
static constexpr const Char JSON_TRUE[] = "true";
static constexpr const Char JSON_FALSE[] = "false";
static constexpr const Size UNICODE_LENGTH = 4;

static constexpr std::uint32_t HEX_0_9 = '0' - 0x0;
static constexpr std::uint32_t HEX_A_F = 'A' - 0xA;
static constexpr std::uint32_t HEX_a_f = 'a' - 0xA;

/*!
 * @brief   Get string length without null termination '\0'
 * @return  String length
 * */
template<Size N>
constexpr Size string_length(const Char (&)[N]) { return (N - 1); }

const Parser::ParseFunctions<Parser::NUM_PARSE_FUNCTIONS>
Parser::m_parse_functions{{
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
    {'\0', &Parser::read_end_of_file}
}};

static bool is_whitespace(int ch) {
    return ((' '  == ch) || ('\n' == ch) || ('\r' == ch) || ('\t' == ch));
}

void Parser::parsing(Value& value) {
    value = nullptr;

    read_whitespaces();
    if (m_pos >= m_end) {
        throw_error(ParserError::EMPTY_DOCUMENT);
    }

    read_value(value);
    read_whitespaces();

    if (m_pos < m_end) {
        throw_error(ParserError::EXTRA_CHARACTER);
    }
}

void Parser::read_value(Value& value) {
    read_whitespaces();

    int ch = *m_pos;
    for (const auto& p : m_parse_functions) {
        if (p.code == ch) {
            return (this->*(p.parse))(value);
        }
    }

    throw_error(ParserError::MISS_VALUE);
}

void Parser::read_true(Value& value) {
    if (m_pos + string_length(JSON_TRUE) > m_end) {
        throw_error(ParserError::END_OF_FILE);
    }

    if (std::memcmp(m_pos, JSON_TRUE, string_length(JSON_TRUE))) {
        throw_error(ParserError::NOT_MATCH_TRUE);
    }

    value.m_type = Value::BOOL;
    value.m_bool = true;

    m_pos += string_length(JSON_TRUE);
}

void Parser::read_false(Value& value) {
    if (m_pos + string_length(JSON_FALSE) > m_end) {
        throw_error(ParserError::END_OF_FILE);
    }

    if (std::memcmp(m_pos, JSON_FALSE, string_length(JSON_FALSE))) {
        throw_error(ParserError::NOT_MATCH_FALSE);
    }

    value.m_type = Value::BOOL;
    value.m_bool = false;

    m_pos += string_length(JSON_FALSE);
}

void Parser::read_null(Value& value) {
    if (m_pos + string_length(JSON_NULL) > m_end) {
        throw_error(ParserError::END_OF_FILE);
    }

    if (std::memcmp(m_pos, JSON_NULL, string_length(JSON_NULL))) {
        throw_error(ParserError::NOT_MATCH_NULL);
    }

    value.m_type = Value::NIL;

    m_pos += string_length(JSON_NULL);
}

void Parser::read_whitespaces() {
    while (m_pos < m_end) {
        if (is_whitespace(*m_pos)) { ++m_pos; }
        else { break; }
    }
}

[[noreturn]]
void Parser::read_end_of_file(Value&) {
    throw_error(ParserError::END_OF_FILE);
}

[[noreturn]]
void Parser::throw_error(ParserError::Code code) {
    throw ParserError(code, Size(m_pos - m_begin));
}
