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
 * @file parser_number.hpp
 *
 * @brief JSON parser number interface
 * */

#ifndef JSON_CXX_PARSER_NUMBER_HPP
#define JSON_CXX_PARSER_NUMBER_HPP

#include <json/types.hpp>
#include <json/number.hpp>
#include <json/parser_error.hpp>

#include <csetjmp>

namespace json {

class ParserNumber {
public:
    ParserNumber(const Char* pos, const Char* end) :
        m_pos{pos}, m_end{end} { }

    void parsing(Number& number);

    const Char* get_position() const {
        return m_pos;
    }

    ParserError& error() {
        return m_error;
    }

    const ParserError& error() const {
        return m_error;
    }
private:
    std::jmp_buf m_jump_buffer{};
    ParserError m_error{};

    bool m_negative{false};
    bool m_overflow{false};
    Difference m_exponent{0};
    Difference m_length{0};
    const Char* m_point{nullptr};
    const Char* m_nonzero_begin{nullptr};
    const Char* m_nonzero_end{nullptr};
    const Char* m_pos{nullptr};
    const Char* m_end{nullptr};

    ParserNumber(const ParserNumber&) = delete;
    ParserNumber(ParserNumber&&) = delete;
    ParserNumber& operator=(const ParserNumber&) = delete;
    ParserNumber& operator=(ParserNumber&&) = delete;

    void read_integral_part();
    void read_fractional_part();
    void read_exponent_part();
    void read_exponent_number();
    void read_digits();
    void write_number(Number& number);
    void write_number_integer(Number& number);
    void write_number_double(Number& number);

    [[noreturn]]
    void throw_error(ParserError::Code code);
};

}

#endif /* JSON_CXX_PARSER_NUMBER_HPP */
