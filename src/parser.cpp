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
 * @file parser.cpp
 *
 * @brief JSON parser implementation
 * */

#include "parser.hpp"

#include <array>
#include <cstring>

using json::Value;
using json::Parser;
using Error = json::DeserializerError;

/*! UTF-16 surrogate pair */
using Surrogate = std::pair<unsigned, unsigned>;

static constexpr const char JSON_NULL[] = "null";
static constexpr const char JSON_TRUE[] = "true";
static constexpr const char JSON_FALSE[] = "false";
static constexpr const std::size_t UNICODE_LENGTH = 4;

static constexpr const Surrogate SURROGATE_MIN(0xD800, 0xDC00);
static constexpr const Surrogate SURROGATE_MAX(0xDBFF, 0xDFFF);

static constexpr std::uint32_t HEX_0_9 = '0' - 0x0;
static constexpr std::uint32_t HEX_A_F = 'A' - 0xA;
static constexpr std::uint32_t HEX_a_f = 'a' - 0xA;

static const std::array<json::Uint64, 19> g_pow10{{
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000,
    10000000000,
    100000000000,
    1000000000000,
    10000000000000,
    100000000000000,
    1000000000000000,
    10000000000000000,
    100000000000000000,
    1000000000000000000
}};

template<typename T>
static inline
T pow10(std::uint64_t index) { return T(g_pow10[index]); }

/*!
 * @brief   Get string length without null termination '\0'
 * @return  String length
 * */
template<std::size_t N>
constexpr std::size_t string_length(const char (&)[N]) { return (N - 1); }

Parser::Parser(const char* str, std::size_t length, std::size_t limit) :
    m_begin{str},
    m_current{str},
    m_end{str + length},
    m_limit{limit} { }

void Parser::parsing(Value& value) {
    value = nullptr;
    read_whitespaces(false);
    if (m_current < m_end) {
        read_value(value);
        read_whitespaces(false);
        if (m_current < m_end) {
            throw_error(Error::INVALID_WHITESPACE);
        }
    }
}

void Parser::read_object(Value& value) {
    read_whitespaces();

    value.m_type = Value::Type::OBJECT;
    new (&value.m_object) Object();

    if ('}' == *m_current) {
        ++m_current;
        return;
    }

    std::size_t count = 0;
    read_object_member(value, count);
}

void Parser::read_object_member(Value& value, std::size_t& count) {
    String key;
    Value tmp;

    read_quote();
    read_string(key);
    read_colon();
    read_value(tmp);
    read_whitespaces();

    ++count;
    if (0 == m_limit--) { throw_error(Error::STACK_LIMIT_REACHED); }

    if (',' == *m_current) {
        ++m_current;
        read_object_member(value, count);
        value.m_object[--count].first = std::move(key);
        value.m_object[count].second = std::move(tmp);
    }
    else if ('}' == *m_current) {
        ++m_current;
        value.m_object.resize(count);
        value.m_object[--count].first = std::move(key);
        value.m_object[count].second = std::move(tmp);

    }
    else {
        throw_error(Error::MISS_CURLY_CLOSE);
    }
}

void Parser::read_string(String& str) {
    std::size_t capacity = 1;
    char ch;

    count_string_chars(capacity);
    str.reserve(capacity);

    while (m_current < m_end) {
        ch = *(m_current++);
        switch (ch) {
        case '"':
            return;
        case '\\':
            read_string_escape(str);
            break;
        default:
            str.push_back(ch);
            break;
        }
    }

    throw_error(Error::END_OF_FILE);
}

void Parser::read_string_escape(String& str) {
    char ch = *m_current;

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
        ++m_current;
        return read_string_unicode(str);
    default:
        throw_error(Error::INVALID_ESCAPE);
    }

    ++m_current;
    str.push_back(ch);
}

static inline
uint32_t decode_utf16_surrogate_pair(const Surrogate& surrogate) {
    return 0x10000
        | ((0x03F & surrogate.first) << 10)
        |  (0x3FF & surrogate.second);
}

void Parser::read_string_unicode(String& str) {
    Surrogate surrogate;
    uint32_t code;

    read_unicode(&m_current, code);
    if (m_current + 2 + UNICODE_LENGTH < m_end) {
        if (('\\' == m_current[0]) && ('u' == m_current[1])) {
            m_current += 2;
            read_unicode(&m_current, surrogate.second);
            surrogate.first = code;
            if ((SURROGATE_MIN <= surrogate)
             && (SURROGATE_MAX >= surrogate)) {
                code = decode_utf16_surrogate_pair(surrogate);
            }
            else { m_current -= (2 + UNICODE_LENGTH); }
        }
    }

    std::size_t code_length;
    std::array<String::value_type, UNICODE_LENGTH> codes;
    if (code < 0x80) {
        codes[0] = String::value_type(code);
        code_length = 1;
    }
    else if (code < 0x800) {
        codes[0] = String::value_type(0xC0 | (0x1F & (code >>  6)));
        codes[1] = String::value_type(0x80 | (0x3F & (code >>  0)));
        code_length = 2;
    }
    else if (code < 0x10000) {
        codes[0] = String::value_type(0xE0 | (0x0F & (code >> 12)));
        codes[1] = String::value_type(0x80 | (0x3F & (code >>  6)));
        codes[2] = String::value_type(0x80 | (0x3F & (code >>  0)));
        code_length = 3;
    }
    else {
        codes[0] = String::value_type(0xF0 | (0x07 & (code >> 18)));
        codes[1] = String::value_type(0x80 | (0x3F & (code >> 12)));
        codes[2] = String::value_type(0x80 | (0x3F & (code >>  6)));
        codes[3] = String::value_type(0x80 | (0x3F & (code >>  0)));
        code_length = 4;
    }
    str.append(codes.data(), code_length);
}

void Parser::read_unicode(const char** pos, uint32_t& code) {
    if (*pos + UNICODE_LENGTH >= m_end) {
        throw_error(Error::END_OF_FILE);
    }

    uint32_t ch;
    std::size_t count = UNICODE_LENGTH;
    code = 0;

    while (count--) {
        ch = uint32_t(**pos);
        code <<= 4;
        if (('0' <= ch) && (ch <= '9')) {
            code |= (ch - HEX_0_9);
        }
        else if (('A' <= ch) && (ch <= 'F')) {
            code |= (ch - HEX_A_F);
        }
        else if (('a' <= ch) && (ch <= 'f')) {
            code |= (ch - HEX_a_f);
        }
        else {
            throw_error(Error::INVALID_UNICODE);
        }
        ++(*pos);
    }
}

void Parser::count_string_chars(std::size_t& count) {
    const char* pos = m_current;

    while (pos < m_end) {
        switch (*pos) {
        case '"':
            return;
        case '\\':
            if ('u' == *(++pos)) {
                uint32_t code;
                read_unicode(&(++pos), code);
                if (code < 0x80) { ++count; }
                else if (code < 0x800) { count += 2; }
                else {
                    if ((SURROGATE_MIN.first > code)
                     || (SURROGATE_MAX.first < code)) { count += 3; }
                    else { count += 4; }
                }
            } else {
                ++count;
                ++pos;
            }
            break;
        default:
            ++count;
            ++pos;
            break;
        }
    }

    throw_error(Error::END_OF_FILE);
}

void Parser::read_value(Value& value) {
    read_whitespaces();

    switch (*m_current) {
    case '"': {
        String str;
        ++m_current;
        read_string(str);
        value.m_type = Value::Type::STRING;
        new (&value.m_string) String(std::move(str));
        break;
    }
    case '{':
        ++m_current;
        read_object(value);
        break;
    case '[':
        ++m_current;
        read_array(value);
        break;
    case 't':
        read_true(value);
        break;
    case 'f':
        read_false(value);
        break;
    case 'n':
        read_null(value);
        break;
    case '-':
        read_number(value);
        break;
    default:
        if (std::isdigit(*m_current)) {
            read_number(value);
        } else {
            throw_error(Error::MISS_VALUE);
        }
        break;
    }
}

void Parser::read_array(Value& value) {
    read_whitespaces();

    value.m_type = Value::Type::ARRAY;
    new (&value.m_array) Array();

    if (']' == *m_current) {
        ++m_current;
        return;
    }

    std::size_t count = 0;
    read_array_element(value, count);
}

void Parser::read_array_element(Value& value, std::size_t& count)  {
    Value tmp;

    read_value(tmp);
    read_whitespaces();

    ++count;
    if (0 == m_limit--) { throw_error(Error::STACK_LIMIT_REACHED); }

    if (',' == *m_current) {
        ++m_current;
        read_array_element(value, count);
        value.m_array[--count] = std::move(tmp);
    }
    else if (']' == *m_current) {
        ++m_current;
        value.m_array.resize(count);
        value.m_array[--count] = std::move(tmp);
    }
    else {
        throw_error(Error::MISS_SQUARE_CLOSE);
    }
}

void Parser::read_colon() {
    read_whitespaces();
    if (':' != *m_current) {
        throw_error(Error::MISS_COLON);
    }
    ++m_current;
}

void Parser::read_quote() {
    read_whitespaces();
    if ('"' != *m_current) {
        throw_error(Error::MISS_QUOTE);
    }
    ++m_current;
}

void Parser::read_whitespaces(bool enable_error) {
    char ch;

    while (m_current < m_end) {
        ch = *m_current;
        if ((' '  == ch) || ('\n' == ch) || ('\r' == ch) || ('\t' == ch)) {
            ++m_current;
        } else { return; }
    }

    if (enable_error) {
        throw_error(Error::END_OF_FILE);
    }
}

void Parser::read_number_digit(Uint64& value) {
    if (m_current >= m_end) {
        throw_error(Error::END_OF_FILE);
    }

    if (std::isdigit(*m_current)) {
        value = Uint64(*m_current - '0');
        ++m_current;
    }
    else {
        throw_error(Error::INVALID_NUMBER_INTEGER);
    }

    while (m_current < m_end) {
        if (std::isdigit(*m_current)) {
            value = (10 * value) + Uint64(*m_current - '0');
            ++m_current;
        } else { return; }
    }
}

void Parser::read_number_integer(Number& number) {
    Uint64 value;
    read_number_digit(value);

    if (Number::Type::UINT == number.m_type) {
        number.m_uint = value;
    } else {
        number.m_int = Int64(-value);
    }
}

void Parser::read_number_fractional(Number& number) {
    Double step = 0.1;
    Double fractional = 0;

    while (m_current < m_end) {
        if (std::isdigit(*m_current)) {
            fractional += (step * (*m_current - '0'));
            step = 0.1 * step;
            ++m_current;
        } else {
            if (Number::Type::UINT == number.m_type) {
                Double tmp = Double(number.m_uint);
                number.m_double = tmp + fractional;
            } else {
                Double tmp = Double(number.m_int);
                number.m_double = tmp - fractional;
            }
            number.m_type = Number::Type::DOUBLE;
            return;
        }
    }

    throw_error(Error::END_OF_FILE);
}

void Parser::read_number_exponent(Number& number) {
    bool is_negative = false;
    Uint64 value;

    if ('+' == *m_current) {
        ++m_current;
    } else if ('-' == *m_current) {
        is_negative = true;
        ++m_current;
    }

    read_number_digit(value);

    switch (number.m_type) {
    case Number::Type::INT:
        if (is_negative) {
            number.m_double /= pow10<Double>(value);
            number.m_type = Number::Type::DOUBLE;
        } else {
            number.m_int *= pow10<Int64>(value);
        }
        break;
    case Number::Type::UINT:
        if (is_negative) {
            number.m_double /= pow10<Double>(value);
            number.m_type = Number::Type::DOUBLE;
        } else {
            number.m_uint *= pow10<Uint64>(value);
        }
        break;
    case Number::Type::DOUBLE:
        if (is_negative) {
            number.m_double /= pow10<Double>(value);
        }
        else {
            number.m_double *= pow10<Double>(value);
        }
        break;
    default:
        break;
    }
}

void Parser::read_number(Value& value) {
    /* Prepare JSON number */
    value.m_type = Value::Type::NUMBER;
    new (&value.m_number) Number();

    if ('-' == *m_current) {
        value.m_number.m_type = Number::Type::INT;
        value.m_number.m_int = 0;
        ++m_current;
    }
    else {
        value.m_number.m_type = Number::Type::UINT;
        value.m_number.m_uint = 0;
    }

    if ('0' == *m_current) {
        ++m_current;
    }
    else {
        read_number_integer(value.m_number);
    }

    if ('.' == *m_current) {
        ++m_current;
        read_number_fractional(value.m_number);
    }

    if (('E' == *m_current) || ('e' == *m_current)) {
        ++m_current;
        read_number_exponent(value.m_number);
    }
}

void Parser::read_true(Value& value) {
    if (m_current + string_length(JSON_TRUE) > m_end) {
        throw_error(Error::END_OF_FILE);
    }

    if (0 != std::strncmp(m_current, JSON_TRUE, string_length(JSON_TRUE))) {
        throw_error(Error::NOT_MATCH_TRUE);
    }

    value.m_type = Value::Type::BOOLEAN;
    value.m_boolean = true;

    m_current += string_length(JSON_TRUE);
}

void Parser::read_false(Value& value) {
    if (m_current + string_length(JSON_FALSE) > m_end) {
        throw_error(Error::END_OF_FILE);
    }

    if (0 != std::strncmp(m_current, JSON_FALSE, string_length(JSON_FALSE))) {
        throw_error(Error::NOT_MATCH_FALSE);
    }

    value.m_type = Value::Type::BOOLEAN;
    value.m_boolean = false;

    m_current += string_length(JSON_FALSE);
}

void Parser::read_null(Value& value) {
    if (m_current + string_length(JSON_NULL) > m_end) {
        throw_error(Error::END_OF_FILE);
    }

    if (0 != std::strncmp(m_current, JSON_NULL, string_length(JSON_NULL))) {
        throw_error(Error::NOT_MATCH_NULL);
    }

    value.m_type = Value::Type::NIL;

    m_current += string_length(JSON_NULL);
}

[[noreturn]] void Parser::throw_error(Error::Code code) {
    throw Error(code, std::size_t(m_current - m_begin));
}
