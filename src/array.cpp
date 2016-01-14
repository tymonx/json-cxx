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
 * @file json/array.cpp
 *
 * @brief JSON array interface
 * */

#include <json/array.hpp>
#include <json/value.hpp>

using json::Bool;
using json::Size;
using json::Array;

Array::Array() :
    m_begin{nullptr},
    m_end{nullptr}
{ }

Array::Array(const Array& other) :
    m_begin{new Value[other.size()]{}},
    m_end{m_begin + other.size()}
{
    Value* dst = m_begin;
    const Value* src = other.m_begin;

    while (dst < m_end) { *(dst++) = *(src++); }
}

Array::Array(Array&& other) :
    m_begin{other.m_begin},
    m_end{other.m_end}
{
    other.m_end = other.m_begin = nullptr;
}

Array& Array::operator=(const Array& other) {
    return *this = Array(other);
}

Array& Array::operator=(Array&& other) {
    if (this != &other) {
        delete [] m_begin;
        m_begin = other.m_begin;
        m_end = other.m_end;
        other.m_end = other.m_begin = nullptr;
    }
    return *this;
}

Size Array::size() const {
    return Size(m_end - m_begin);
}

Bool Array::empty() const {
    return m_end == m_begin;
}

Array::~Array() {
    delete [] m_begin;
}
