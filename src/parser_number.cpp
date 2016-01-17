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
 * @file parser_number.cpp
 *
 * @brief JSON parser number interface
 * */

#include "parser_number.hpp"

#include <json/parser_error.hpp>

#include <new>
#include <cctype>
#include <limits>

using json::Uint;
using json::Int;
using json::Difference;
using json::Number;
using json::ParserNumber;
using Error = json::ParserError;

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

void ParserNumber::parsing(Number& number) {
    if ('-' == *m_pos) {
        ++m_pos;
        m_negative = true;
    }

    read_integral_part();

    if ((m_pos < m_end) && ('.' == *m_pos)) {
        read_fractional_part();
    }

    if ((m_pos < m_end) && (('e' == *m_pos) || ('E' == *m_pos))) {
        read_exponent_part();
    }

    write_number(number);
}

void ParserNumber::read_integral_part() {
    if (m_pos < m_end) {
        if (std::isdigit(*m_pos)) {
            if ('0' != *m_pos) {
                read_digits();
            }
            else {
                ++m_pos;
            }
            m_point = m_pos;
        }
        else { throw Error{Error::INVALID_NUMBER_INTEGER, m_pos}; }
    }
    else { throw Error{Error::END_OF_FILE, m_end}; }
}

void ParserNumber::read_fractional_part() {
    ++m_pos;
    if (m_pos < m_end) {
        if (std::isdigit(*m_pos)) {
            read_digits();
        }
        else { throw Error{Error::INVALID_NUMBER_FRACTION, m_pos}; }
    }
    else { throw Error{Error::END_OF_FILE, m_end}; }
}

void ParserNumber::read_exponent_part() {
    ++m_pos;
    if (m_pos < m_end) {
        if (std::isdigit(*m_pos)) {
            read_exponent_number();
        }
        else if ('+' == *m_pos) {
            ++m_pos;
            read_exponent_number();
        }
        else if ('-' == *m_pos) {
            ++m_pos;
            read_exponent_number();
            m_exponent = -m_exponent;
        }
        else { throw Error{Error::INVALID_NUMBER_EXPONENT, m_pos}; }
    }
    else { throw Error{Error::END_OF_FILE, m_end}; }
}

void ParserNumber::read_exponent_number() {
    if (m_pos < m_end) {
        while ((m_pos < m_end) && std::isdigit(*m_pos)) {
            m_exponent = (10 * m_exponent) + (*(m_pos++) - '0');
        }
    }
    else { throw Error{Error::END_OF_FILE, m_end}; }
}

void ParserNumber::read_digits() {
    while ((m_pos < m_end) && std::isdigit(*m_pos)) {
        if ('0' != *m_pos) {
            if (!m_nonzero_begin) { m_nonzero_begin = m_pos; }
            m_nonzero_end = m_pos + 1;
        }
        ++m_pos;
    }
}

void ParserNumber::write_number(Number& number) {
    if ((m_nonzero_end <= m_point) || (m_nonzero_begin > m_point)) {
        m_length = (m_nonzero_end - m_nonzero_begin);
    }
    else {
        m_length = (m_nonzero_end - m_nonzero_begin - 1);
    }

    m_exponent += m_length;

    if (m_nonzero_end <= m_point) {
        m_exponent += (m_point - m_nonzero_end);
    }
    else {
        m_exponent += (m_point - m_nonzero_end + 1);
    }

    write_number_integer(number);
    if (m_overflow) {
        write_number_double(number);
    }
}

void ParserNumber::write_number_integer(Number& number) {
    Difference digits;
    Difference digits_max;

    if (m_negative) {
        digits_max = get_digits_max<Int>();
    }
    else {
        digits_max = get_digits_max<Uint>();
    }

    if (0 == m_length) {
        digits = 0;
    }
    else if (m_exponent >= m_length) {
        digits = m_exponent;
    }
    else {
        digits = -1;
    }

    if ((digits >= 0) && (digits <= digits_max)) {
        Uint max_value;
        Uint max_mod10;
        Uint value{0};
        const Char* pos{m_nonzero_begin};

        if (m_negative) {
            max_value = get_max_by_10<Int>();
            max_mod10 = get_max_mod_10<Int>();
        }
        else {
            max_value = get_max_by_10<Uint>();
            max_mod10 = get_max_mod_10<Uint>();
        }

        while (pos < m_nonzero_end) {
            if ('.' != *pos) {
                Uint mod10 = Uint(*pos - '0');
                if (value >= max_value) {
                    m_overflow = (value > max_value) ||
                        ((value == max_value) && (mod10 > max_mod10));
                }
                value = (10 * value) + mod10;
                --digits;
            }
            ++pos;
        }

        while (digits) {
            if (value > max_value) { m_overflow = true; }
            value *= 10;
            --digits;
        }

        if (m_negative) {
            new (&number) Number(-Int(value));
        }
        else {
            new (&number) Number(value);
        }
    }
    else {
        m_overflow = true;
    }
}

void ParserNumber::write_number_double(Number& number) {
    Double value{0};
    Difference exponent{m_exponent};
    const Char* pos{m_nonzero_end - 1};

    while (pos >= m_nonzero_begin) {
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

    if (m_negative) {
        new (&number) Number(-value);
    }
    else {
        new (&number) Number(value);
    }
}
