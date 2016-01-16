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

#include <array>
#include <limits>
#include <utility>
#include <cstring>

using json::Char;
using json::Size;
using json::Value;
using json::Parser;
using Error = json::ParserError;

using Uint16 = std::uint_fast16_t;
using Uint32 = std::uint_fast32_t;

static constexpr const Char JSON_NULL[] = "null";
static constexpr const Char JSON_TRUE[] = "true";
static constexpr const Char JSON_FALSE[] = "false";
static constexpr const Size UNICODE_LENGTH = 4;

static constexpr Uint16 SURROGATE_HIGH_MIN = 0xD800;
static constexpr Uint16 SURROGATE_HIGH_MAX = 0xDBFF;

static constexpr Uint16 SURROGATE_LOW_MIN = 0xDC00;
static constexpr Uint16 SURROGATE_LOW_MAX = 0xDFFF;

const Size Parser::DEFAULT_LIMIT_PER_OBJECT =
    std::numeric_limits<Size>::max();

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
    {'9', &Parser::read_number}
}};

static bool is_whitespace(int ch) {
    return ((' '  == ch) || ('\n' == ch) || ('\r' == ch) || ('\t' == ch));
}

static inline
int is_utf16_surrogate_pair(Uint16 high, Uint16 low) {
    return (high >= SURROGATE_HIGH_MIN) && (high <= SURROGATE_HIGH_MAX) &&
        (low >= SURROGATE_LOW_MIN) && (low <= SURROGATE_LOW_MAX);
}

static inline
Uint32 decode_utf16_surrogate_pair(Uint32 high, Uint32 low) {
    return 0x10000 | ((0x3F & high) << 10) | (0x3FF & low);
}

static
Uint16 read_unicode(const Char* pos, const Char* end) {
    if (pos + UNICODE_LENGTH > end) { throw Error{Error::END_OF_FILE, pos}; }

    end = pos + UNICODE_LENGTH;
    Uint16 code{0};

    while (pos < end) {
        Uint16 ch = Uint16(*pos);
        if ((ch >= '0') && (ch <= '9')) {
            ch = ch - '0';
        }
        else if ((ch >= 'A') && (ch <= 'F')) {
            ch = (ch + 0xA - 'A');
        }
        else if ((ch >= 'a') && (ch <= 'f')) {
            ch = (ch + 0xA - 'a');
        }
        else {
            throw Error{Error::INVALID_UNICODE, pos};
        }
        code = (code << 4) | ch;
        ++pos;
    }

    return code;
}

static
Size count_string_chars(const Char* pos, const Char* end) {
    Size count{0};

    for (; (pos < end) && ('"' != *pos); ++pos, ++count) {
        if ('\\' == *pos) {
            if ('u' == *(++pos)) {
                Uint16 code = read_unicode(++pos, end);
                if (code >= 0x80) {
                    if (code < 0x800) {
                        count += 1;
                    }
                    else if ((code < SURROGATE_HIGH_MIN) ||
                             (code > SURROGATE_HIGH_MAX)) {
                        count += 2;
                    }
                    else {
                        count += 3;
                    }
                }
                pos += 3;
            }
        }
    }

    if (pos >= end) { throw Error{Error::END_OF_FILE, pos}; }

    return count;
}

Parser::Parser() :
    m_limit{DEFAULT_LIMIT_PER_OBJECT},
    m_begin{nullptr},
    m_end{nullptr},
    m_pos{nullptr}
{ }

void Parser::parsing(const Char* begin, const Char* end, Value& value) {
    m_begin = begin;
    m_end = end;
    m_pos = begin;

    value.~Value();
    value.m_type = Value::NIL;

    read_whitespaces();
    if (m_pos >= m_end) {
        throw Error{Error::EMPTY_DOCUMENT, m_pos};
    }

    read_value(value);
    read_whitespaces();

    if (m_pos < m_end) {
        throw Error{Error::EXTRA_CHARACTER, m_pos};
    }
}

void Parser::read_value(Value& value) {
    read_whitespaces();
    if (m_pos >= m_end) {
        throw Error{Error::END_OF_FILE, m_pos};
    }

    int ch = *m_pos;
    for (const auto& p : m_parse_functions) {
        if (p.first == ch) {
            return (this->*(p.second))(value);
        }
    }

    throw Error{Error::MISS_VALUE, m_pos};
}

void Parser::read_array(Value& value) {
    ++m_pos;
    read_whitespaces();

    value.m_type = Value::ARRAY;
    std::memset(&value.m_array, 0, sizeof(Array));

    if (m_pos < m_end) {
        if (']' == *m_pos) {
            ++m_pos;
        }
        else {
            Size count = 0;
            read_array_element(value, count);
        }
    }
    else {
        throw Error{Error::END_OF_FILE, m_pos};
    }
}

void Parser::read_array_element(Value& value, Size& count)  {
    if (0 == m_limit--) { throw Error{Error::STACK_LIMIT_REACHED, m_pos}; }

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
        value.m_array.m_end = value.m_array.m_end + count;
    }
    else {
        throw Error{Error::MISS_SQUARE_CLOSE, m_pos};
    }
    std::memcpy(&value.m_array[--count], tmp, sizeof(Value));
}

void Parser::read_object(Value& value) {
    ++m_pos;
    read_whitespaces();

    value.m_type = Value::OBJECT;
    std::memset(&value.m_object, 0, sizeof(Object));

    if (m_pos < m_end) {
        if ('}' == *m_pos) {
            ++m_pos;
        }
        else {
            Size count = 0;
            read_object_member(value, count);
        }
    }
    else {
        throw Error{Error::END_OF_FILE, m_pos};
    }
}

void Parser::read_object_member(Value& value, Size& count) {
    if (0 == m_limit--) { throw Error{Error::STACK_LIMIT_REACHED, m_pos}; }

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
        value.m_object.m_end = value.m_object.m_begin + count;;
    }
    else {
        throw Error{Error::MISS_CURLY_CLOSE, m_pos};
    }
    --count;
    std::memcpy(&value.m_object[--count].name, &key->m_string, sizeof(String));
    std::memcpy(&value.m_object[count].value, tmp, sizeof(Value));
}

void Parser::read_string(Value& value) {
    Size count = count_string_chars(++m_pos, m_end);

    Char* str = new Char[count + 1];
    str[count] = '\0';

    value.m_type = Value::STRING;
    value.m_string.m_begin = str;
    value.m_string.m_end = str + count;

    int ch = *(m_pos++);
    while ('"' != ch) {
        if ('\\' == ch) {
            ch = *(m_pos++);
            switch (ch) {
            case '"':
            case '\\':
            case '/':
                break;
            case 'n':
                ch = '\n';
                break;
            case 'r':
                ch = '\r';
                break;
            case 't':
                ch = '\t';
                break;
            case 'b':
                ch = '\b';
                break;
            case 'f':
                ch = '\f';
                break;
            case 'u':
                ++m_pos;
                str = read_string_unicode(str, ch);
                continue;
            default:
                throw Error{Error::INVALID_ESCAPE, m_pos};
            }
        }
        *(str++) = Char(ch);
        ch = *(m_pos++);
    }
}

Char* Parser::read_string_unicode(Char* str, int& ch) {
    Uint32 unicode = read_unicode(m_pos, m_end);
    m_pos += UNICODE_LENGTH;

    if ((m_pos < m_end) && ('\\' == m_pos[0]) && ('u' == m_pos[1])) {
        auto surrogate_high = unicode;
        Uint16 surrogate_low = read_unicode(m_pos + 2, m_end);
        if (is_utf16_surrogate_pair(surrogate_high, surrogate_low)) {
            unicode = decode_utf16_surrogate_pair(
                    surrogate_high, surrogate_low);
            m_pos += (2 + UNICODE_LENGTH);
        }
    }

    if (unicode < 0x80) {
        *(str++) = Char(unicode);
    }
    else if (unicode < 0x800) {
        *(str++) = Char(0xC0 | (0x1F & (unicode >>  6)));
        *(str++) = Char(0x80 | (0x3F & unicode));
    }
    else if (unicode < 0x10000) {
        *(str++) = Char(0xE0 | (0x0F & (unicode >> 12)));
        *(str++) = Char(0x80 | (0x3F & (unicode >>  6)));
        *(str++) = Char(0x80 | (0x3F & unicode));
    }
    else {
        *(str++) = Char(0xF0 | (0x07 & (unicode >> 18)));
        *(str++) = Char(0x80 | (0x3F & (unicode >> 12)));
        *(str++) = Char(0x80 | (0x3F & (unicode >>  6)));
        *(str++) = Char(0x80 | (0x3F & unicode));
    }

    ch = *m_pos;
    return str;
}

void Parser::read_true(Value& value) {
    if (m_pos + string_length(JSON_TRUE) > m_end) {
        throw Error{ParserError::END_OF_FILE, m_pos};
    }

    if (std::memcmp(m_pos, JSON_TRUE, string_length(JSON_TRUE))) {
        throw Error{ParserError::NOT_MATCH_TRUE, m_pos};
    }

    value.m_type = Value::BOOL;
    value.m_bool = true;

    m_pos += string_length(JSON_TRUE);
}

void Parser::read_false(Value& value) {
    if (m_pos + string_length(JSON_FALSE) > m_end) {
        throw Error{ParserError::END_OF_FILE, m_pos};
    }

    if (std::memcmp(m_pos, JSON_FALSE, string_length(JSON_FALSE))) {
        throw Error{Error::NOT_MATCH_FALSE, m_pos};
    }

    value.m_type = Value::BOOL;
    value.m_bool = false;

    m_pos += string_length(JSON_FALSE);
}

void Parser::read_null(Value& value) {
    if (m_pos + string_length(JSON_NULL) > m_end) {
        throw Error{ParserError::END_OF_FILE, m_pos};
    }

    if (std::memcmp(m_pos, JSON_NULL, string_length(JSON_NULL))) {
        throw Error{ParserError::NOT_MATCH_NULL, m_pos};
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

void Parser::read_colon() {
    read_whitespaces();
    if (m_pos < m_end) {
        if (':' == *m_pos) { ++m_pos; }
        else { throw Error{ParserError::MISS_COLON, m_pos}; }
    }
    else { throw Error{Error::END_OF_FILE, m_pos}; }
}

void Parser::read_quote() {
    read_whitespaces();
    if (m_pos < m_end) {
        if ('"' == *m_pos) { ++m_pos; }
        else { throw Error{ParserError::MISS_QUOTE, m_pos}; }
    }
    else { throw Error{Error::END_OF_FILE, m_pos}; }
}
