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

#include "json/string.hpp"

#include <cstring>

using json::String;

String::String() :
    m_begin{new char[1]{}},
    m_end{m_begin} { }

String::String(std::size_t count, char ch) :
    m_begin{new char[count + 1]},
    m_end{m_begin + count}
{
    std::memset(m_begin, ch, count);
    *m_end = '\0';
}

String::String(const String& other, std::size_t pos, std::size_t count) :
    m_begin{other.m_begin + pos},
    m_end{m_begin + count}
{
    m_begin = (m_begin < other.m_end) ? m_begin : other.m_end;
    m_end = (m_end < other.m_end) ? m_end : other.m_end;
    m_end = (m_begin < m_end) ? m_end : m_begin;

    count = std::size_t(m_end - m_begin);
    m_begin = static_cast<char*>(
        std::memcpy(new char[count + 1], m_begin, count)
    );
    m_end = m_begin + count;
    *m_end = '\0';
}

String::String(const char* str, std::size_t count) :
    m_begin{new char[count + 1]},
    m_end{m_begin + count}
{
    std::memcpy(m_begin, str, count);
    *m_end = '\0';
}

String::String(const char* str) :
    m_begin{},
    m_end{}
{
    std::size_t count = std::strlen(str);
    m_begin = static_cast<char*>(
        std::memcpy(new char[count + 1], str, count + 1)
    );
    m_end = m_begin + count;
}

String::String(const char* first, const char* last) :
    m_begin{},
    m_end{}
{
    std::size_t count = std::size_t(last - first);
    m_begin = static_cast<char*>(
        std::memcpy(new char[count + 1], first, count)
    );
    m_end = m_begin + count;
    *m_end = '\0';
}

String::String(std::initializer_list<char> init) :
    m_begin{new char[init.size() + 1]},
    m_end{m_begin + init.size()}
{
    std::memcpy(m_begin, init.begin(), init.size());
    *m_end = '\0';
}

String::String(const String& other) :
    m_begin{new char[other.size() + 1]},
    m_end{m_begin + other.size()}
{
    std::memcpy(m_begin, other.m_begin, other.size());
    *m_end = '\0';
}

String::String(String&& other) :
    m_begin{other.m_begin},
    m_end{other.m_end}
{
    other.m_end = other.m_begin = nullptr;
}

String& String::operator=(const String& other) {
    if (this != &other) {
        delete [] m_begin;
        m_begin = static_cast<char*>(
            std::memcpy(new char[other.size() + 1], other.m_begin,
            other.size())
        );
        m_end = m_begin + other.size();
        *m_end = '\0';
    }
    return *this;
}

String& String::operator=(String&& other) {
     if (this != &other) {
        m_begin = other.m_begin;
        m_end = other.m_end;
        other.m_end = other.m_begin = nullptr;
    }
    return *this;
}

void String::swap(String& other) {
    String tmp(std::move(*this));
    *this = std::move(other);
    other = std::move(tmp);
}

void String::clear() {
    if (m_end != m_begin) {
        delete [] m_begin;
        m_end = m_begin = new char[1]{};
    }
}
