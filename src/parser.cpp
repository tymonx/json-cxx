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

#include <limits>
#include <cctype>
#include <cstring>

#ifndef __EXCEPTIONS
#define THROW_ERROR(_e) do { } while(0)
#else
#define THROW_ERROR(_e) do { throw _e; } while(0)
#endif

using json::Int;
using json::Uint;
using json::Char;
using json::Size;
using json::Value;
using json::Parser;
using json::Difference;
using Error = json::ParserError;
using Uint16 = std::uint_fast16_t;
using Uint32 = std::uint_fast32_t;

static constexpr Char JSON_NULL[] = "null";
static constexpr Char JSON_TRUE[] = "true";
static constexpr Char JSON_FALSE[] = "false";

static constexpr Size JSON_NULL_LENGTH = 4;
static constexpr Size JSON_TRUE_LENGTH = 4;
static constexpr Size JSON_FALSE_LENGTH = 5;

static constexpr Size UNICODE_LENGTH = 4;

struct json::Parser::NumberInfo {
    bool negative;
    Difference exponent;
    Difference nonzero_length;
    const Char* point;
    const Char* nonzero_begin;
    const Char* nonzero_end;
};

struct SurrogatePair {
    Uint16 high;
    Uint16 low;

    bool operator>=(const SurrogatePair& other) const {
        return (high >= other.high) && (low >= other.low);
    }

    bool operator<=(const SurrogatePair& other) const {
        return (high <= other.high) && (low <= other.low);
    }
};

static constexpr SurrogatePair SURROGATE_MIN{0xD800, 0xDC00};
static constexpr SurrogatePair SURROGATE_MAX{0xDBFF, 0xDFFF};

template<typename T>
static constexpr T get_digits_max() noexcept {
    return 1 + std::numeric_limits<T>::digits10;
}

template<typename T>
static constexpr Uint get_max_by_10() noexcept;

template<typename T>
static constexpr Uint get_max_mod_10() noexcept;

template<>
constexpr Uint get_max_by_10<Uint>() noexcept {
    return std::numeric_limits<Uint>::max() / 10;
}

template<>
constexpr Uint get_max_mod_10<Uint>() noexcept {
    return std::numeric_limits<Uint>::max() % 10;
}

template<>
constexpr Uint get_max_by_10<Int>() noexcept {
    return Uint(-std::numeric_limits<Int>::min()) / 10;
}

template<>
constexpr Uint get_max_mod_10<Int>() noexcept {
    return Uint(-std::numeric_limits<Int>::min()) % 10;
}

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
int is_utf16_surrogate_pair(const SurrogatePair& pair) {
    return (pair >= SURROGATE_MIN) && (pair <= SURROGATE_MAX);
}

static inline
Uint32 decode_utf16_surrogate_pair(const SurrogatePair& pair) {
    return 0x10000 |
        ((0x3F & Uint32(pair.high)) << 10) |
        (0x3FF & Uint32(pair.low));
}

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
    if (!setjmp(m_jump_buffer)) {
        m_error_code = Error::NONE;
        m_allocator = get_default_allocator();

        value.~Value();
        value.m_type = Value::NIL;

        read_whitespaces();
        if (m_pos < m_end) {
            read_value(value);
            read_whitespaces();

            if ((m_pos < m_end) && !m_stream_mode) {
                throw_error(Error::EXTRA_CHARACTER);
            }
        }
        else { throw_error(Error::EMPTY_DOCUMENT); }
    }
    else {
        value.~Value();
        value.m_type = Value::NIL;
        THROW_ERROR((Error{m_error_code,
            const_cast<const Char*>(m_error_position)}));
    }
}

void Parser::read_value(Value& value) {
    value.m_type = Value::NIL;

    read_whitespaces();
    if (m_pos < m_end) {
        const auto* p = m_parse_functions;
        while ((p->code != *m_pos) && (p->parse != nullptr)) { ++p; }

        if (nullptr != p->parse) {
            (this->*(p->parse))(value);
        }
        else { throw_error(Error::MISS_VALUE); }
    }
    else { throw_error(Error::END_OF_FILE); }
}

void Parser::read_array(Value& value) {
    ++m_pos;
    read_whitespaces();

    if (m_pos < m_end) {
        if (']' == *m_pos) {
            ++m_pos;
            value.m_array.m_begin = nullptr;
            value.m_array.m_end = nullptr;
            value.m_array.m_allocator = m_allocator;
            value.m_type = Value::ARRAY;
        }
        else {
            Size count = 0;
            read_array_element(value, count);
        }
    }
    else { throw_error(Error::END_OF_FILE); }
}

void Parser::read_array_element(Value& value, Size& count)  {
    if (m_limit) { stack_guard(); }

    char tmp_buffer[sizeof(Value)];
    Value* tmp = static_cast<Value*>(static_cast<void*>(tmp_buffer));

    read_value(*tmp);
    read_whitespaces();
    ++count;

    if (',' == *m_pos) {
        ++m_pos;
        read_array_element(value, count);
    }
    else if (']' == *m_pos) {
        ++m_pos;
        void* ptr = m_allocator->allocate(count * sizeof(Value));
        if (ptr) {
            value.m_array.m_begin = static_cast<Value*>(ptr);
            value.m_array.m_end = static_cast<Value*>(ptr) + count;
            value.m_array.m_allocator = m_allocator;
            value.m_type = Value::ARRAY;
        }
        else { throw_error(Error::BAD_ALLOCATION); }
    }
    else { throw_error(Error::MISS_SQUARE_CLOSE); }

    --count;
    std::memcpy(&value.m_array[count], tmp, sizeof(Value));
}

void Parser::read_object(Value& value) {
    ++m_pos;
    read_whitespaces();

    if (m_pos < m_end) {
        if ('}' == *m_pos) {
            ++m_pos;
            value.m_object.m_begin = nullptr;
            value.m_object.m_end = nullptr;
            value.m_object.m_allocator = m_allocator;
            value.m_type = Value::OBJECT;
        }
        else {
            Size count = 0;
            read_object_member(value, count);
        }
    }
    else { throw_error(Error::END_OF_FILE); }
}

void Parser::read_object_member(Value& value, Size& count) {
    if (m_limit) { stack_guard(); }

    char key_buffer[sizeof(Value)];
    char tmp_buffer[sizeof(Value)];

    Value* key = static_cast<Value*>(static_cast<void*>(key_buffer));
    Value* tmp = static_cast<Value*>(static_cast<void*>(tmp_buffer));

    key->m_type = Value::NIL;

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
        void* ptr = m_allocator->allocate(count * sizeof(Pair));
        if (ptr) {
            value.m_object.m_begin = static_cast<Pair*>(ptr);
            value.m_object.m_end = static_cast<Pair*>(ptr) + count;
            value.m_object.m_allocator = m_allocator;
            value.m_type = Value::OBJECT;
        }
        else { throw_error(Error::BAD_ALLOCATION); }
    }
    else { throw_error(Error::MISS_CURLY_CLOSE); }

    --count;
    std::memcpy(&value.m_object[count].key, &key->m_string, sizeof(String));
    std::memcpy(&value.m_object[count].value, tmp, sizeof(Value));
}

void Parser::read_true(Value& value) {
    if (m_pos + JSON_TRUE_LENGTH <= m_end) {
        if (!std::memcmp(m_pos, JSON_TRUE, JSON_TRUE_LENGTH)) {
            value.m_bool = true;
            value.m_type = Value::BOOL;
            m_pos += JSON_TRUE_LENGTH;
        }
        else { throw_error(Error::NOT_MATCH_TRUE); }
    }
    else { throw_error(Error::END_OF_FILE); }
}

void Parser::read_false(Value& value) {
    if (m_pos + JSON_FALSE_LENGTH <= m_end) {
        if (!std::memcmp(m_pos, JSON_FALSE, JSON_FALSE_LENGTH)) {
            value.m_bool = false;
            value.m_type = Value::BOOL;
            m_pos += JSON_FALSE_LENGTH;
        }
        else { throw_error(Error::NOT_MATCH_FALSE); }
    }
    else { throw_error(Error::END_OF_FILE); }
}

void Parser::read_null(Value& value) {
    if (m_pos + JSON_NULL_LENGTH <= m_end) {
        if (!std::memcmp(m_pos, JSON_NULL, JSON_NULL_LENGTH)) {
            value.m_type = Value::NIL;
            m_pos += JSON_NULL_LENGTH;
        }
        else { throw_error(Error::NOT_MATCH_NULL); }
    }
    else { throw_error(Error::END_OF_FILE); }
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
        else { throw_error(Error::MISS_COLON); }
    }
    else { throw_error(Error::END_OF_FILE); }
}

void Parser::read_quote() {
    read_whitespaces();
    if (m_pos < m_end) {
        if ('"' != *m_pos) { throw_error(Error::MISS_QUOTE); }
    }
    else { throw_error(Error::END_OF_FILE); }
}

void Parser::stack_guard() {
    if (0 == --m_limit) { throw_error(Error::STACK_LIMIT_REACHED); }
}

/* Parse number */
void Parser::read_number(Value& value) {
    NumberInfo info{};

    if ('-' == *m_pos) {
        ++m_pos;
        info.negative = true;
    }

    read_number_integral(info);

    if ((m_pos < m_end) && ('.' == *m_pos)) {
        read_number_fractional(info);
    }

    if ((m_pos < m_end) && (('e' == *m_pos) || ('E' == *m_pos))) {
        read_number_exponent(info);
    }

    info.nonzero_length = info.nonzero_end - info.nonzero_begin;
    if ((info.nonzero_begin < info.point) && (info.nonzero_end > info.point)) {
        --info.nonzero_length;
    }

    info.exponent += (info.point - info.nonzero_end) + info.nonzero_length;
    if (info.nonzero_end > info.point) {
        ++info.exponent;
    }

    if (write_number_integer(info, value.m_number)) {
        write_number_double(info, value.m_number);
    }

    value.m_type = Value::NUMBER;
}

void Parser::read_number_integral(NumberInfo& info) {
    if (m_pos < m_end) {
        if (std::isdigit(*m_pos)) {
            if ('0' != *m_pos) {
                read_number_digits(info);
            }
            else {
                ++m_pos;
            }
            info.point = m_pos;
        }
        else { throw_error(Error::INVALID_NUMBER_INTEGER); }
    }
    else { throw_error(Error::END_OF_FILE); }
}

void Parser::read_number_fractional(NumberInfo& info) {
    ++m_pos;
    if (m_pos < m_end) {
        if (std::isdigit(*m_pos)) {
            read_number_digits(info);
        }
        else { throw_error(Error::INVALID_NUMBER_FRACTION); }
    }
    else { throw_error(Error::END_OF_FILE); }
}

void Parser::read_number_exponent(NumberInfo& info) {
    ++m_pos;
    if (m_pos < m_end) {
        bool is_negative;

        if (std::isdigit(*m_pos)) {
            is_negative = false;
        }
        else if ('+' == *m_pos) {
            ++m_pos;
            is_negative = false;
        }
        else if ('-' == *m_pos) {
            ++m_pos;
            is_negative = true;
        }
        else { throw_error(Error::INVALID_NUMBER_EXPONENT); }

        if (m_pos < m_end) {
            while ((m_pos < m_end) && std::isdigit(*m_pos)) {
                info.exponent = (10 * info.exponent) + (*(m_pos++) - '0');
            }
            if (is_negative) {
                info.exponent = -info.exponent;
            }
        }
        else { throw_error(Error::END_OF_FILE); }
    }
    else { throw_error(Error::END_OF_FILE); }
}

void Parser::read_number_digits(NumberInfo& info) {
    while ((m_pos < m_end) && std::isdigit(*m_pos)) {
        if ('0' != *m_pos) {
            if (!info.nonzero_begin) { info.nonzero_begin = m_pos; }
            info.nonzero_end = m_pos + 1;
        }
        ++m_pos;
    }
}

bool Parser::write_number_integer(const NumberInfo& info, Number& number) {
    bool overflow{false};
    Difference digits;
    Difference digits_max;

    if (info.negative) {
        digits_max = get_digits_max<Int>();
    }
    else {
        digits_max = get_digits_max<Uint>();
    }

    if (0 == info.nonzero_length) {
        digits = 0;
    }
    else if (info.exponent >= info.nonzero_length) {
        digits = info.exponent;
    }
    else {
        digits = -1;
    }

    if ((digits >= 0) && (digits <= digits_max)) {
        Uint max_value;
        Uint max_mod10;
        Uint value{0};
        const Char* pos{info.nonzero_begin};

        if (info.negative) {
            max_value = get_max_by_10<Int>();
            max_mod10 = get_max_mod_10<Int>();
        }
        else {
            max_value = get_max_by_10<Uint>();
            max_mod10 = get_max_mod_10<Uint>();
        }

        while (pos < info.nonzero_end) {
            if ('.' != *pos) {
                Uint mod10 = Uint(*pos - '0');
                if (value >= max_value) {
                    overflow = (value > max_value) ||
                        ((value == max_value) && (mod10 > max_mod10));
                }
                value = (10 * value) + mod10;
                --digits;
            }
            ++pos;
        }

        while (digits) {
            if (value > max_value) { overflow = true; }
            value *= 10;
            --digits;
        }

        if (info.negative) {
            new (&number) Number(-Int(value));
        }
        else {
            new (&number) Number(value);
        }
    }
    else {
        overflow = true;
    }

    return overflow;
}

void Parser::write_number_double(const NumberInfo& info, Number& number) {
    Double value{0};
    Difference exponent{info.exponent};
    const Char* pos{info.nonzero_end - 1};

    while (pos >= info.nonzero_begin) {
        if ('.' != *pos) {
            value = 0.1 * (value + Uint(*pos - '0'));
        }
        --pos;
    }

    while (exponent) {
        if (exponent > 0) {
            value *= 10;
            --exponent;
        }
        else {
            value /= 10;
            ++exponent;
        }
    }

    if (info.negative) {
        new (&number) Number(-value);
    }
    else {
        new (&number) Number(value);
    }
}

/* String parsing */
void Parser::read_string(Value& value) {
    const Char* pos = ++m_pos;
    Size count = read_string_count();
    m_pos = pos;

    Char* str = static_cast<Char*>(m_allocator->allocate(count + 1));
    if (str) {
        value.m_string.m_begin = str;
        value.m_string.m_end = str + count;
        value.m_string.m_allocator = m_allocator;
        value.m_type = Value::STRING;
        str[count] = '\0';

        int ch = *(m_pos++);
        count = 0;
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
                    str = read_string_unicode(str, ch);
                    continue;
                default:
                    throw_error(Error::INVALID_ESCAPE);
                }
            }
            *(str++) = Char(ch);
            ch = *(m_pos++);
        }
    }
    else { throw_error(Error::BAD_ALLOCATION); }
}

Size Parser::read_string_count() {
    Size count{0};

    while ((m_pos < m_end) && ('"' != *m_pos)) {
        if ('\\' == *m_pos) {
            ++m_pos;
            if ('u' == *m_pos) {
                ++m_pos;
                Uint16 code = read_string_unicode();
                if (code < 0x80) {
                    count += 1;
                }
                else if (code < 0x800) {
                    count += 2;
                }
                else if ((code < SURROGATE_MIN.high) ||
                         (code > SURROGATE_MAX.high)) {
                    count += 3;
                }
                else {
                    count += 4;
                }
            }
            else {
                ++m_pos;
                ++count;
            }
        }
        else {
            ++m_pos;
            ++count;
        }
    }
    if ('"' != *m_pos) { throw_error(Error::END_OF_FILE); }

    return count;
}

Char* Parser::read_string_unicode(Char* str, int& ch) {
    Uint32 unicode = read_string_unicode();

    if ((m_pos < m_end) && ('\\' == m_pos[0]) && ('u' == m_pos[1])) {
        m_pos += 2;
        SurrogatePair surrogate{unicode, read_string_unicode()};
        if (is_utf16_surrogate_pair(surrogate)) {
            unicode = decode_utf16_surrogate_pair(surrogate);
        }
        else {
            m_pos -= (2 + UNICODE_LENGTH);
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

    ch = *(m_pos++);
    return str;
}

unsigned Parser::read_string_unicode() {
    unsigned code{0};

    if (m_pos + UNICODE_LENGTH <= m_end) {
        const Char* end = m_pos + UNICODE_LENGTH;

        while (m_pos < end) {
            unsigned ch = unsigned(*m_pos);
            if ((ch >= '0') && (ch <= '9')) {
                ch = ch - '0';
            }
            else if ((ch >= 'A') && (ch <= 'F')) {
                ch = (ch + 0xA - 'A');
            }
            else if ((ch >= 'a') && (ch <= 'f')) {
                ch = (ch + 0xA - 'a');
            }
            else { throw_error(Error::INVALID_UNICODE); }
            code = (code << 4) | ch;
            ++m_pos;
        }
    }
    else { throw_error(Error::END_OF_FILE); }

    return code;
}

[[noreturn]]
void Parser::throw_error(ParserError::Code code) {
    m_error_code = code;
    m_error_position = (Error::END_OF_FILE == code) ? m_end : m_pos;
    std::longjmp(m_jump_buffer, code);
}
