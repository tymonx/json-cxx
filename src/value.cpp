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
 * @file json/value.cpp
 *
 * @brief JSON value interface
 * */

#include <json/value.hpp>

#include <new>
#include <utility>

using json::Value;

Value::Value() : m_type{Type::NIL} { }

Value::Value(Null) : m_type{Type::NIL} { }

Value::Value(Type type) : m_type{type} {
    switch (m_type) {
    case Type::OBJECT:
        new (&m_object) Object();
        break;
    case Type::ARRAY:
        new (&m_array) Array();
        break;
    case Type::STRING:
        new (&m_string) String();
        break;
    case Type::NUMBER:
        new (&m_number) Number();
        break;
    case Type::BOOL:
        m_bool = false;
        break;
    case Type::NIL:
    default:
        break;
    }
}

Value::Value(const Value& other) : m_type{other.m_type} {
    switch (m_type) {
    case Type::OBJECT:
        new (&m_object) Object(other.m_object);
        break;
    case Type::ARRAY:
        new (&m_array) Array(other.m_array);
        break;
    case Type::STRING:
        new (&m_string) String(other.m_string);
        break;
    case Type::NUMBER:
        new (&m_number) Number(other.m_number);
        break;
    case Type::BOOL:
        m_bool = other.m_bool;
        break;
    case Type::NIL:
    default:
        break;
    }
}

Value::Value(Value&& other) : m_type{other.m_type} {
    switch (m_type) {
    case Type::OBJECT:
        new (&m_object) Object(std::move(other.m_object));
        break;
    case Type::ARRAY:
        new (&m_array) Array(std::move(other.m_array));
        break;
    case Type::STRING:
        new (&m_string) String(std::move(other.m_string));
        break;
    case Type::NUMBER:
        new (&m_number) Number(std::move(other.m_number));
        break;
    case Type::BOOL:
        m_bool = other.m_bool;
        break;
    case Type::NIL:
    default:
        break;
    }
    other.m_type = Type::NIL;
}

Value::~Value() {
    switch (m_type) {
    case Type::OBJECT:
        m_object.~Object();
        break;
    case Type::ARRAY:
        m_array.~Array();
        break;
    case Type::STRING:
        m_string.~String();
        break;
    case Type::NUMBER:
    case Type::BOOL:
    case Type::NIL:
    default:
        break;
    }
}

Value& Value::operator=(const Value& other) {
    return *this = Value(other);
}

Value& Value::operator=(Value&& other) {
    if (this != &other) {
        this->~Value();
        switch (other.m_type) {
        case Type::OBJECT:
            new (&m_object) Object(std::move(other.m_object));
            break;
        case Type::ARRAY:
            new (&m_array) Array(std::move(other.m_array));
            break;
        case Type::STRING:
            new (&m_string) String(std::move(other.m_string));
            break;
        case Type::NUMBER:
            new (&m_number) Number(std::move(other.m_number));
            break;
        case Type::BOOL:
            m_bool = other.m_bool;
            break;
        case Type::NIL:
        default:
            break;
        }
        m_type = other.m_type;
        other.m_type = Type::NIL;
    }
    return *this;
}

Value& Value::operator=(Null) {
    this->~Value();
    m_type = Type::NIL;
    return *this;
}

Value& Value::operator=(Type type) {
    return *this = Value(type);
}
