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

#include "json/formatter/pretty.hpp"

using json::formatter::Pretty;

Pretty::Pretty(Writter* writter) :
    Compact(writter) { }

Pretty::~Pretty() { }

void Pretty::write_object(const Object& object) {
    auto it_pos = object.cbegin();
    auto it_end = object.cend();

    if (it_pos != it_end) {
        m_writter->write('{');

        ++m_level;
        std::size_t indent_length = m_indent * m_level;
        while (it_pos < it_end) {
            m_writter->write('\n');
            m_writter->write(indent_length, ' ');
            write_string(it_pos->first);
            m_writter->write(" : ", 3);
            write_value(it_pos->second);
            if (++it_pos != it_end) {
                m_writter->write(',');
            }
        }
        m_writter->write('\n');

        --m_level;

        m_writter->write(m_indent * m_level, ' ');
        m_writter->write('}');
    }
    else {
        m_writter->write("{}", 2);
    }
}

void Pretty::write_array(const Array& array) {
    auto it_pos = array.cbegin();
    auto it_end = array.cend();

    if (it_pos != it_end) {
        m_writter->write('[');

        ++m_level;
        std::size_t indent_length = m_indent * m_level;
        while (it_pos < it_end) {
            m_writter->write('\n');
            m_writter->write(indent_length, ' ');
            write_value(*it_pos);
            if (++it_pos != it_end) {
                m_writter->write(',');
            }
        }
        m_writter->write('\n');

        --m_level;

        m_writter->write(m_indent * m_level, ' ');
        m_writter->write(']');
    }
    else {
        m_writter->write("[]", 2);
    }
}
