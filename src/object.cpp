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
 * @file json/object.cpp
 *
 * @brief JSON object interface
 * */

#include <json/object.hpp>
#include <json/pair.hpp>
#include <json/value.hpp>

#include <cstring>

using json::Pair;
using json::Value;
using json::Object;

static Value* g_invalid_value = nullptr;
static const Value g_null_value = nullptr;

Object::Object(const_iterator first, const_iterator last, Allocator* allocator) :
    m_allocator{allocator}
{
    Size num = Size(last - first);
    m_begin = static_cast<Pair*>(m_allocator->allocate(num * sizeof(Pair)));
    if (m_begin) {
        m_end = m_begin + num;
        for (auto it = begin(); it < cend(); ++it, ++first) {
            new (it) Pair(*first, m_allocator);
        }
    }
}

Object::Object(std::initializer_list<Pair> init, Allocator* allocator) :
    Object(init.begin(), init.end(), allocator)
{ }

Object::Object(const String& key, const Value& value, Allocator* allocator) :
    Object(Pair(key, value, allocator), allocator)
{ }

Object::Object(const Pair& pair, Allocator* allocator) :
    Object(&pair, &pair + 1, allocator)
{ }

Object::Object(const Object& other, Allocator* allocator) :
    Object(other.cbegin(), other.cend(), allocator)
{ }

Object& Object::operator=(Object&& other) {
    if (this != &other) {
        this->~Object();
        m_begin = other.m_begin;
        m_end = other.m_end;
        m_allocator = other.m_allocator;
        other.m_end = other.m_begin = nullptr;
    }
    return *this;
}

Object::~Object() {
    for (auto it = begin(); it < cend(); ++it) {
        it->~Pair();
    }
    m_allocator->deallocate(m_begin);
    m_end = m_begin = nullptr;
}

Value& Object::at(const Char* str) {
    return at(str, std::strlen(str));
}

const Value& Object::at(const Char* str) const {
    return at(str, std::strlen(str));
}

Value& Object::at(const String& str) {
    return at(str, str.length());
}

const Value& Object::at(const String& str) const {
    return at(str, str.length());
}

Value& Object::at(const Char* key, Size length) {
    const Value& value = static_cast<const Object*>(this)->at(key, length);
    if (&value != &g_null_value) {
        return const_cast<Value&>(value);
    }

    Size num = size();
    auto ptr = static_cast<Pair*>(
        m_allocator->allocate((num + 1) * sizeof(Pair))
    );
    if (ptr) {
        std::memcpy(ptr, m_begin, num * sizeof(Pair));
        m_allocator->deallocate(m_begin);
        m_begin = ptr;
        m_end = m_begin + num + 1;
        return (new (m_begin + num) Pair(key, nullptr, m_allocator))->value;
    }
    return *g_invalid_value;
}

const Value& Object::at(const Char* key, Size length) const {
    if (!empty()) {
        auto it = cend() - 1;
        while (it >= cbegin()) {
            if ((it->key.size() == length) &&
                    !std::memcmp(it->key, key, length)) {
                return it->value;
            }
            --it;
        }
    }
    return g_null_value;
}

Object::iterator Object::erase(const_iterator pos) {
    if ((pos >= m_begin) && (pos < m_end)) {
        Size num = size();
        pos->~Pair();
        --m_end;
        return static_cast<Pair*>(std::memmove(const_cast<Pair*>(pos.base()),
            pos + 1, (num - 1) * sizeof(Pair)));

    }
    return m_end;
}

Object::iterator Object::erase(const_iterator first, const_iterator last) {
    Size count = Size(last - first);
    iterator it{m_end};

    while (count--) {
        it = erase(first);
    }

    return it;
}
