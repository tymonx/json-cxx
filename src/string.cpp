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
 * @file json/string.cpp
 *
 * @brief JSON string interface
 * */

#include <json/string.hpp>

#include <cstring>

using json::String;

static const json::Char STRING_EMPTY[] = "";

String::String(const_iterator first, const_iterator last,
        Allocator* allocator) :
    m_allocator{allocator}
{
    Size num = Size(last - first);
    m_begin = static_cast<Char*>(
            m_allocator->allocate((num + 1) * sizeof(Char)));
    if (m_begin) {
        m_end = m_begin + num;
        std::memcpy(m_begin, first, num * sizeof(Char));
        *m_end = '\0';
    }
}

String::String(Size count, Char ch, Allocator* allocator) :
    m_allocator{allocator}
{
    m_begin = static_cast<Char*>(m_allocator->allocate(count + 1));
    if (m_begin) {
        m_end = m_begin + count;
        std::memset(m_begin, ch, count * sizeof(Char));
        *m_end = '\0';
    }
}

String::String(const String& other, Size pos, Size count,
        Allocator* allocator) :
    m_begin{other.m_begin + pos},
    m_end{m_begin + count},
    m_allocator{allocator}
{
    m_begin = (m_begin < other.m_end) ? m_begin : other.m_end;
    m_end = (m_end < other.m_end) ? m_end : other.m_end;
    m_end = (m_begin < m_end) ? m_end : m_begin;

    *this = String(m_begin, m_end, allocator);
}

String::~String() {
    m_allocator->deallocate(m_begin);
    m_end = m_begin = nullptr;
}

String& String::operator=(String&& other) {
    if (this != &other) {
        this->~String();
        m_begin = other.m_begin;
        m_end = other.m_end;
        other.m_end = other.m_begin = nullptr;
    }
    return *this;
}

void String::swap(String& other) {
    auto tmp_begin = m_begin;
    auto tmp_end = m_end;

    m_begin = other.m_begin;
    m_end = other.m_end;

    other.m_begin = tmp_begin;
    other.m_end = tmp_end;
}

void String::clear() {
    if (m_end != m_begin) {
        this->~String();
        Char* str = static_cast<Char*>(m_allocator->allocate(1));
        if (str) {
            m_end = m_begin = str;
            *str = '\0';
        }
    }
}

const json::Char* String::c_str() const {
    return empty() ? STRING_EMPTY : m_begin;
}
