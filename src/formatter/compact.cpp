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
 * @file formatter/pretty.cpp
 *
 * @brief JSON formatter implementation
 * */

#include "json/formatter/compact.hpp"

#include <array>
#include <cmath>
#include <limits>
#include <algorithm>

using json::formatter::Compact;

static constexpr std::size_t MAX_CHAR_BUFFER = 64;

/*! JSON null  */
static constexpr const char JSON_NULL[] = "null";

/*! JSON boolean true */
static constexpr const char JSON_TRUE[] = "true";

/*! JSON boolean false */
static constexpr const char JSON_FALSE[] = "false";

static constexpr const char NUMBER_CHARS[] = "0123456789";

Compact::Compact(Writter* writter) :
    Formatter(writter) { }

Compact::~Compact() { }

void Compact::formatting(const json::Value& value) {
    if (m_writter) {
        write_value(value);
    }
}

void Compact::write_value(const Value& value) {
    switch (value.get_type()) {
    case Value::Type::OBJECT:
        write_object(Object(value));
        break;
    case Value::Type::ARRAY:
        write_array(Array(value));
        break;
    case Value::Type::STRING:
        write_string(String(value));
        break;
    case Value::Type::NUMBER:
        write_number(Number(value));
        break;
    case Value::Type::BOOLEAN:
        write_boolean(Bool(value));
        break;
    case Value::Type::NIL:
        write_empty();
        break;
    default:
        break;
    }
}

void Compact::write_object(const Object& object) {
    auto it_pos = object.cbegin();
    auto it_end = object.cend();

    m_writter->write('{');
    while (it_pos < it_end) {
        write_string(it_pos->first);
        m_writter->write(':');
        write_value(it_pos->second);
        if (++it_pos != it_end) {
            m_writter->write(',');
        }
    }
    m_writter->write('}');
}

void Compact::write_array(const Array& array) {
    auto it_pos = array.cbegin();
    auto it_end = array.cend();

    m_writter->write('[');
    while (it_pos < it_end) {
        write_value(*it_pos);
        if (++it_pos != it_end) {
            m_writter->write(',');
        }
    }
    m_writter->write(']');
}

void Compact::write_string(const String& str) {
    std::size_t count = 0;

    std::for_each(str.cbegin(), str.cend(),
        [&count] (const char& ch) {
            if (('\\' == ch) || ('\"' == ch)) {
                ++count;
            }
            ++count;
        }
    );

    m_writter->write('"');
    if (count == str.size()) {
        m_writter->write(str);
    }
    else {
        std::string tmp;
        tmp.reserve(count);
        std::for_each(str.cbegin(), str.cend(),
            [&tmp] (const char& ch) {
                if (('\\' == ch) || ('\"' == ch)) {
                    tmp.push_back('\\');
                }
                tmp.push_back(ch);
            }
        );
        m_writter->write(tmp);
    }
    m_writter->write('"');
}

void Compact::write_boolean(Bool value) {
    if (value) {
        m_writter->write(JSON_TRUE, 4);
    }
    else {
        m_writter->write(JSON_FALSE, 5);
    }
}

void Compact::write_empty() {
    m_writter->write(JSON_NULL, 4);
}

static std::size_t write_number_uint(char* buffer, json::Uint64 value) {
    std::size_t count = 0;
    json::Uint64 tmp = value;

    do {
        ++count;
        tmp /= 10;
    } while (tmp);

    char* pos = buffer + count;
    do {
        *(--pos) = NUMBER_CHARS[value % 10];
        value /= 10;
    } while (value);

    return count;
}

static std::size_t write_number_int(char* buffer, json::Int64 value) {
    std::size_t count = 0;

    if (value < 0) {
        buffer[0] = '-';
        value = -value;
        ++buffer;
        ++count;
    }

    count += write_number_uint(buffer, json::Uint64(value));

    return count;
}

static std::size_t write_number_double(char* buffer, json::Double value) {
    std::size_t count = 0;

    if (std::signbit(value)) {
        buffer[0] = '-';
        value = std::abs(value);
        ++buffer;
        ++count;
    }

    count += write_number_uint(buffer, json::Uint64(value));
    buffer += count;
    buffer[count++] = '.';

    double integral_part;
    value = 10 * std::modf(value, &integral_part);
    do {
        buffer[count++] = NUMBER_CHARS[json::Uint64(value) % 10];
        value = 10 * std::modf(value, &integral_part);
    } while (value >= std::numeric_limits<json::Double>::epsilon());

    return count;
}

void Compact::write_number(const Number& number) {
    std::array<char, MAX_CHAR_BUFFER> buffer;
    std::size_t count;

    switch (number.get_type()) {
    case Number::Type::INT:
        count = write_number_int(buffer.data(), Int64(number));
        break;
    case Number::Type::UINT:
        count = write_number_uint(buffer.data(), Uint64(number));
        break;
    case Number::Type::DOUBLE:
        count = write_number_double(buffer.data(), Double(number));
        break;
    default:
        count = 0;
        break;
    }

    m_writter->write(buffer.data(), count);
}
