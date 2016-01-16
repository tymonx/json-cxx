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
 * @file json/array.hpp
 *
 * @brief JSON array interface
 * */

#ifndef JSON_CXX_ARRAY_HPP
#define JSON_CXX_ARRAY_HPP

#include <json/types.hpp>

namespace json {

class Value;

class Array {
public:
    friend class Parser;

    Array();

    Array(const Array& other);

    Array(Array&& other);

    Array& operator=(const Array& other);

    Array& operator=(Array&& other);

    Size size() const;

    Bool empty() const {
        return m_end == m_begin;
    }

    Value& operator[](Size index);

    const Value& operator[](Size index) const;

    Value* begin() {
        return m_begin;
    }

    const Value* begin() const {
        return m_begin;
    }

    const Value* cbegin() const {
        return m_begin;
    }

    Value* end() {
        return m_end;
    }

    const Value* end() const {
        return m_end;
    }

    const Value* cend() const {
        return m_end;
    }

    ~Array();
private:
    Value* m_begin;
    Value* m_end;
};

}

#endif /* JSON_CXX_ARRAY_HPP */
