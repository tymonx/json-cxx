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

using json::formatter::Compact;

static constexpr std::size_t MAX_CHAR_BUFFER = 64;

/*! JSON null  */
static constexpr const char JSON_NULL[] = "null";

/*! JSON boolean true */
static constexpr const char JSON_TRUE[] = "true";

/*! JSON boolean false */
static constexpr const char JSON_FALSE[] = "false";

static constexpr const char NUMBER_CHARS[] = "0123456789";

Compact::Compact(WritterPtr writter) :
    Formatter(std::move(writter)) { }

Compact::~Compact() { }

void Compact::formatting(const json::Value& value) {
    if (m_writter) {
        write_value(value);
    }
}

void Compact::write_value(const Value& value) {
    switch (value.get_type()) {
    case Value::Type::OBJECT:
        write_object(value);
        break;
    case Value::Type::ARRAY:
        write_array(value);
        break;
    case Value::Type::STRING:
        write_string(String(value));
        break;
    case Value::Type::NUMBER:
        write_number(value);
        break;
    case Value::Type::BOOLEAN:
        write_boolean(value);
        break;
    case Value::Type::NIL:
        write_empty(value);
        break;
    default:
        break;
    }
}

void Compact::write_object(const Value& value) {
    std::size_t num = value.size();

    m_writter->write('{');
    for (const auto& obj : Object(value)) {
        write_string(obj.first);
        m_writter->write(':');
        write_value(obj.second);
        if (--num) { m_writter->write(','); }
    }
    m_writter->write('}');
}

void Compact::write_array(const Value& value) {
    std::size_t num = value.size();

    m_writter->write('[');
    for (const auto& v : Array(value)) {
        write_value(v);
        if (--num) { m_writter->write(','); }
    }
    m_writter->write(']');
}

void Compact::write_string(const std::string& str) {
    std::size_t count = 0;
    std::string tmp;

    for (const auto& ch : str) {
        if (('\\' == ch) || ('\"' == ch)) { ++count; }
        ++count;
    }

    tmp.reserve(2 + count);
    tmp.push_back('"');
    if (count == str.size()) {
        tmp.append(str);
    }
    else {
        for (const auto& ch : str) {
            if (('\\' == ch) || ('\"' == ch)) {
                tmp.push_back('\\');
            }
            tmp.push_back(ch);
        }
    }
    tmp.push_back('"');
    m_writter->write(tmp);
}

void Compact::write_number(const Value& value) {
    switch (Number(value).get_type()) {
    case Number::Type::INT:
        write_number_int(Int64(value));
        break;
    case Number::Type::UINT:
        write_number_uint(Uint64(value));
        break;
    case Number::Type::DOUBLE:
        write_number_double(Double(value));
        break;
    default:
        break;
    }
}

void Compact::write_number_int(Int64 value) {
    std::array<char, MAX_CHAR_BUFFER> buffer;
    std::size_t count = 0;
    Uint64 tmp;

    if (value < 0) {
        buffer[0] = '-';
        ++count;
        value = -value;
    }

    tmp = Uint64(value);
    do {
        ++count;
        tmp /= 10;
    } while (tmp);

    tmp = Uint64(value);
    char* pos = buffer.data() + count;
    do {
        *(--pos) = NUMBER_CHARS[tmp % 10];
        tmp /= 10;
    } while (tmp);

    m_writter->write(buffer.data(), count);
}

void Compact::write_number_uint(Uint64 value) {
    std::array<char, MAX_CHAR_BUFFER> buffer;
    std::size_t count = 0;
    Uint64 tmp;

    tmp = value;
    do {
        ++count;
        tmp /= 10;
    } while (tmp);

    tmp = value;
    char* pos = buffer.data() + count;
    do {
        *(--pos) = NUMBER_CHARS[tmp % 10];
        tmp /= 10;
    } while (tmp);

    m_writter->write(buffer.data(), count);
}

void Compact::write_number_double(Double value) {
    std::array<char, MAX_CHAR_BUFFER> buffer;
    std::size_t count = 0;
    double integral_part;
    Uint64 tmp;

    if (std::signbit(value)) {
        buffer[0] = '-';
        ++count;
        value = std::abs(value);
    }

    tmp = Uint64(value);
    do {
        ++count;
        tmp /= 10;
    } while (tmp);

    tmp = Uint64(value);
    char* pos = buffer.data() + count;
    do {
        *(--pos) = NUMBER_CHARS[tmp % 10];
        tmp /= 10;
    } while (tmp);

    buffer[count++] = '.';

    value = 10 * std::modf(value, &integral_part);
    do {
        buffer[count++] = NUMBER_CHARS[Uint64(value) % 10];
        value = 10 * std::modf(value, &integral_part);
    } while (value >= std::numeric_limits<Double>::epsilon());

    m_writter->write(buffer.data(), count);
}

void Compact::write_boolean(const Value& value) {
    if (Bool(value)) {
        m_writter->write(JSON_TRUE, 4);
    }
    else {
        m_writter->write(JSON_FALSE, 5);
    }
}

void Compact::write_empty(const Value&) {
    m_writter->write(JSON_NULL, 4);
}
