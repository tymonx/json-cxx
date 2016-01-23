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

#include <algorithm>

using json::Object;

Object::Object(Size size, Allocator* allocator) :
    m_allocator{allocator}
{
    m_begin = static_cast<Pair*>(
        m_allocator->allocate(size * sizeof(Pair))
    );
    if (m_begin != nullptr) {
        m_end = m_begin + size;
        std::for_each(m_begin, m_end,
            [this] (Pair& pair)  {
                new (&pair) Value(m_allocator);
            }
        );
    }
}

Object::Object(const Object& other, Allocator* allocator) :
    m_allocator{allocator}
{
    m_begin = static_cast<Pair*>(
        m_allocator->allocate(other.size() * sizeof(Pair))
    );
    if (m_begin != nullptr) {
        m_end = m_begin + other.size();
        auto it = other.cbegin();
        std::for_each(m_begin, m_end,
            [this, &it] (Pair& pair)  {
                new (&pair) Pair(*(it++), m_allocator);
            }
        );
    }
}

Object& Object::operator=(Object&& other) {
    if (this != &other) {
        this->~Object();
        m_begin = other.m_begin;
        m_end = other.m_end;
        other.m_end = other.m_begin = nullptr;
    }
    return *this;
}

Object::~Object() {
    std::for_each(m_begin, m_end,
        [](Pair& pair) {
            pair.~Pair();
        }
    );
    m_allocator->deallocate(m_begin.base(), size() * sizeof(Pair));
}
