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
#include <utility>

using json::String;

String::String() :
    m_begin{new char[1]{}},
    m_end{m_begin}
{ }

String::String(const Char* str) :
    String(str, std::strlen(str))
{ }

String::String(const Char* first, const Char* last) :
    String(first, Size(last - first))
{ }

String::String(std::initializer_list<Char> init) :
    String(init.begin(), init.size())
{ }

String::String(const String& other) :
    String(other.data(), other.size())
{ }

String::String(String&& other) :
    m_begin{other.m_begin},
    m_end{other.m_end}
{
    other.m_end = other.m_begin = nullptr;
}

String& String::operator=(const String& other) {
    return *this = String(other.data(), other.size());
}

String& String::operator=(const Char* s) {
    return *this = String(s);
}

String& String::operator=(Char ch) {
    return *this = String(1, ch);
}

String& String::operator=(std::initializer_list<Char> init) {
    return *this = String(init);
}

String& String::assign(Size count, Char ch) {
    return *this = String(count, ch);
}

String& String::assign(const String& other) {
    return *this = other;
}

String& String::assign(String&& other) {
    return *this = std::move(other);
}

String& String::assign(const String& other, Size pos, Size count) {
    return *this = String(other, pos, count);
}

String& String::assign(const Char* s, Size count) {
    return *this = String(s, count);
}

String& String::assign(const Char* s) {
    return *this = String(s);
}

String& String::assign(const Char* first, const Char* last) {
    return *this = String(first, last);
}

String& String::assign(std::initializer_list<Char> init) {
    return *this = String(init);
}

String::String(const Char* str, Size count) :
    m_begin{new Char[count + 1]},
    m_end{m_begin + count}
{
    std::memcpy(m_begin, str, count);
    *m_end = '\0';
}

String::String(Size count, Char ch) :
    m_begin{new Char[count + 1]},
    m_end{m_begin + count}
{
    std::memset(m_begin, ch, count);
    *m_end = '\0';
}

String::String(const String& other, Size pos, Size count) :
    m_begin{other.m_begin + pos},
    m_end{m_begin + count}
{
    m_begin = (m_begin < other.m_end) ? m_begin : other.m_end;
    m_end = (m_end < other.m_end) ? m_end : other.m_end;
    m_end = (m_begin < m_end) ? m_end : m_begin;

    String tmp(m_begin, m_end);
    *this = std::move(tmp);
}

String::~String() {
    delete [] m_begin;
}

String& String::operator=(String&& other) {
    if (this != &other) {
        delete [] m_begin;
        m_begin = other.m_begin;
        m_end = other.m_end;
        other.m_end = other.m_begin = nullptr;
    }
    return *this;
}

void String::swap(String& other) {
    Char* tmp_begin = m_begin;
    Char* tmp_end = m_end;

    m_begin = other.m_begin;
    m_end = other.m_end;

    other.m_begin = tmp_begin;
    other.m_end = tmp_end;
}

void String::clear() {
    if (m_end != m_begin) {
        delete [] m_begin;
        m_end = m_begin = new Char[1]{};
    }
}
