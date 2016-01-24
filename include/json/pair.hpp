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
 * @file json/pair.hpp
 *
 * @brief JSON pair interface
 * */

#ifndef JSON_CXX_PAIR_HPP
#define JSON_CXX_PAIR_HPP

#include <json/string.hpp>
#include <json/value.hpp>
#include <json/allocator/default.hpp>

namespace json {

class Pair {
public:
    Pair(Allocator* allocator = allocator::Default::get_instance()) :
        key{allocator},
        value(allocator)
    { }

    Pair(const Pair& other) :
        Pair(other, other.value.get_allocator())
    { }

    Pair(const Pair& other, Allocator* allocator) :
        key{other.key, allocator},
        value(other.value, allocator)
    { }

    Pair(Pair&& other) :
        Pair(std::move(other), other.value.get_allocator())
    { }

    Pair(Pair&& other, Allocator* allocator) :
        key{std::move(other.key), allocator},
        value(std::move(other.value), allocator)
    { }

    Pair& operator=(const Pair& other) {
        key = other.key;
        value = other.value;
        return *this;
    }

    Pair& operator=(Pair&& other) {
        key = std::move(other.key);
        value = std::move(other.value);
        return *this;
    }

    Pair(const String& str, const Value& other,
            Allocator* allocator = allocator::Default::get_instance()) :
        key{str, allocator},
        value(other, allocator)
    { }

    String key;
    Value value;
};

}

#endif /* JSON_CXX_PAIR_HPP */
