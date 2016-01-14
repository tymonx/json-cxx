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
 * @file value.cpp
 *
 * @brief JSON value implementation
 * */

#include "json/value.hpp"
#include "json/iterator.hpp"
#include "json/value_error.hpp"

#include <limits>
#include <type_traits>
#include <functional>

using json::Value;

Value::Value(Type type) : m_type(type) {
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
    case Type::BOOLEAN:
        m_boolean = false;
        break;
    case Type::NIL:
    default:
        break;
    }
}

Value::Value(Null) : m_type(Type::NIL) { }

Value::Value(Bool boolean) : m_type(Type::BOOLEAN) {
    m_boolean = boolean;
}

Value::Value(const char* str) : m_type(Type::STRING) {
    new (&m_string) String(str);
}

Value::Value(const String& str) : m_type(Type::STRING) {
    new (&m_string) String(str);
}

Value::Value(const Pair& pair) : m_type(Type::OBJECT) {
    new (&m_object) Object{pair};
}

Value::Value(const char* key, const Value& value) : m_type(Type::OBJECT) {
    new (&m_object) Object{std::make_pair(key, value)};
}

Value::Value(const String& key, const Value& value) : m_type(Type::OBJECT) {
    new (&m_object) Object{std::make_pair(key, value)};
}

Value::Value(Uint value) : m_type(Type::NUMBER) {
    new (&m_number) Number(value);
}

Value::Value(Int value) : m_type(Type::NUMBER) {
    new (&m_number) Number(value);
}

Value::Value(Double value) : m_type(Type::NUMBER) {
    new (&m_number) Number(value);
}

Value::Value(const Number& number) : m_type(Type::NUMBER) {
    new (&m_number) Number(number);
}

Value::Value(size_t count, const Value& value) : m_type(Type::ARRAY) {
    new (&m_array) Array(count, value);
}

Value::Value(std::initializer_list<Pair> init_list) : m_type(Type::OBJECT) {
    new (&m_object) Object();

    for (auto it = init_list.begin(); it < init_list.end(); ++it) {
        (*this)[it->first] = it->second;
    }
}

Value::Value(std::initializer_list<Value> init_list) : m_type(Type::ARRAY) {
    new (&m_array) Array(init_list);
}

Value::Value(const Value& value) : m_type(value.m_type) {
    switch (m_type) {
    case Type::OBJECT:
        new (&m_object) Object(value.m_object);
        break;
    case Type::ARRAY:
        new (&m_array) Array(value.m_array);
        break;
    case Type::STRING:
        new (&m_string) String(value.m_string);
        break;
    case Type::NUMBER:
        new (&m_number) Number(value.m_number);
        break;
    case Type::BOOLEAN:
        m_boolean = value.m_boolean;
        break;
    case Type::NIL:
    default:
        break;
    }
}

Value::Value(Value&& value) : m_type(value.m_type) {
    switch (m_type) {
    case Type::OBJECT:
        new (&m_object) Object(std::move(value.m_object));
        break;
    case Type::ARRAY:
        new (&m_array) Array(std::move(value.m_array));
        break;
    case Type::STRING:
        new (&m_string) String(std::move(value.m_string));
        break;
    case Type::NUMBER:
        new (&m_number) Number(std::move(value.m_number));
        break;
    case Type::BOOLEAN:
        m_boolean = value.m_boolean;
        break;
    case Type::NIL:
    default:
        break;
    }
}

Value::~Value() {
    switch (m_type) {
    case Type::OBJECT:
        m_object.~vector();
        break;
    case Type::ARRAY:
        m_array.~vector();
        break;
    case Type::STRING:
        m_string.~basic_string();
        break;
    case Type::NUMBER:
        m_number.~Number();
        break;
    case Type::BOOLEAN:
    case Type::NIL:
    default:
        break;
    }
}

Value& Value::operator=(const Value& value) {
    if (this != &value) {
        if (value.m_type == m_type) {
            switch (m_type) {
            case Type::OBJECT:
                m_object = value.m_object;
                break;
            case Type::ARRAY:
                m_array = value.m_array;
                break;
            case Type::STRING:
                m_string = value.m_string;
                break;
            case Type::NUMBER:
                m_number = value.m_number;
                break;
            case Type::BOOLEAN:
                m_boolean = value.m_boolean;
                break;
            case Type::NIL:
            default:
                break;
            }
        }
        else {
            this->~Value();
            m_type = value.m_type;
            switch (m_type) {
            case Type::OBJECT:
                new (&m_object) Object(value.m_object);
                break;
            case Type::ARRAY:
                new (&m_array) Array(value.m_array);
                break;
            case Type::STRING:
                new (&m_string) String(value.m_string);
                break;
            case Type::NUMBER:
                new (&m_number) Number(value.m_number);
                break;
            case Type::BOOLEAN:
                m_boolean = value.m_boolean;
                break;
            case Type::NIL:
            default:
                break;
            }
        }
    }
    return *this;
}

Value& Value::operator=(Value&& value) {
    if (this != &value) {
        if (value.m_type == m_type) {
            switch (m_type) {
            case Type::OBJECT:
                m_object = std::move(value.m_object);
                break;
            case Type::ARRAY:
                m_array = std::move(value.m_array);
                break;
            case Type::STRING:
                m_string = std::move(value.m_string);
                break;
            case Type::NUMBER:
                m_number = std::move(value.m_number);
                break;
            case Type::BOOLEAN:
                m_boolean = value.m_boolean;
                break;
            case Type::NIL:
            default:
                break;
            }
        }
        else {
            this->~Value();
            m_type = value.m_type;
            switch (m_type) {
            case Type::OBJECT:
                new (&m_object) Object(std::move(value.m_object));
                break;
            case Type::ARRAY:
                new (&m_array) Array(std::move(value.m_array));
                break;
            case Type::STRING:
                new (&m_string) String(std::move(value.m_string));
                break;
            case Type::NUMBER:
                new (&m_number) Number(std::move(value.m_number));
                break;
            case Type::BOOLEAN:
                m_boolean = value.m_boolean;
                break;
            case Type::NIL:
            default:
                break;
            }
        }
    }
    return *this;
}

Value& Value::operator+=(const Value& value) {
    switch (m_type) {
    case Type::OBJECT:
        if (value.is_object()) {
            for (auto it = value.cbegin(); value.cend() != it; ++it) {
                (*this)[it.key()] = *it;
            }
        }
        break;
    case Type::ARRAY:
        if (value.is_array()) {
            m_array.insert(m_array.end(),
                    value.m_array.begin(),
                    value.m_array.end());
        }
        else if (value.is_object()) {
             m_array.insert(m_array.end(),
                    value.m_object.begin(),
                    value.m_object.end());
        }
        else {
            m_array.push_back(value);
        }
        break;
    case Type::STRING:
        if (value.is_string()) {
            m_string += value.m_string;
        }
        break;
    case Type::NUMBER:
        if (value.is_number()) {
            m_number += value.m_number;
        }
        break;
    case Type::NIL:
        *this = value;
        break;
    case Type::BOOLEAN:
    default:
        break;
    }

    return *this;
}

void Value::assign(std::initializer_list<Pair> init_list) {
    if (!is_object()) {
        this->~Value();
        m_type = Type::OBJECT;
        new (&m_object) Object();
    }
    else {
        m_object.clear();
    }

    for (auto it = init_list.begin(); it < init_list.end(); ++it) {
        (*this)[it->first] = it->second;
    }
}

void Value::assign(std::initializer_list<Value> init_list) {
    if (is_array()) {
        m_array.assign(init_list);
    }
    else {
        this->~Value();
        m_type = Type::ARRAY;
        new (&m_array) Array(init_list);
    }
}

void Value::assign(std::size_t count, const Value& value) {
    if (is_array()) {
        m_array.assign(count, value);
    }
    else {
        this->~Value();
        m_type = Type::ARRAY;
        new (&m_array) Array(count, value);
    }
}

std::size_t Value::size() const {
    std::size_t value;

    switch (m_type) {
    case Type::OBJECT:
        value = m_object.size();
        break;
    case Type::ARRAY:
        value = m_array.size();
        break;
    case Type::STRING:
    case Type::NIL:
    case Type::NUMBER:
    case Type::BOOLEAN:
    default:
        value = 0;
        break;
    }

    return value;
}

void Value::clear() {
    switch (m_type) {
    case Type::OBJECT:
        m_object.clear();
        break;
    case Type::ARRAY:
        m_array.clear();
        break;
    case Type::STRING:
        m_string.clear();
        break;
    case Type::NUMBER:
        m_number = 0;
        break;
    case Type::BOOLEAN:
        m_boolean = false;
        break;
    case Type::NIL:
    default:
        break;
    }
}

std::size_t Value::erase(const char* key) {
    if (!is_object()) { return 0; }

    for (auto it = m_object.begin(); it != m_object.end(); ++it) {
        if (it->first == key) {
            m_object.erase(it);
            return 1;
        }
    }
    return 0;
}

Value::iterator Value::erase(const_iterator pos) {
    iterator tmp;

    if (is_array() && pos.is_array()) {
        tmp = m_array.erase(pos.m_array_iterator);
    }
    else if (is_object() && pos.is_object()) {
        tmp = m_object.erase(pos.m_object_iterator);
    }
    else {
        tmp = end();
    }

    return tmp;
}

Value::iterator Value::erase(const_iterator first, const_iterator last) {
    iterator tmp;

    for (auto it = first; it < last; ++it) {
        tmp = erase(it);
    }

    return tmp;
}

Value::iterator Value::insert(const_iterator pos, const Value& value) {
    iterator tmp;

    if (is_array() && pos.is_array()) {
        tmp = m_array.insert(pos.m_array_iterator, value);
    }
    else if (is_object() && pos.is_object() && value.is_object()) {
        for (auto it = value.cbegin(); value.cend() != it; ++it, ++pos) {
            if (!is_member(it.key())) {
                tmp = m_object.insert(pos.m_object_iterator,
                            Pair(it.key(), *it));
            }
        }
    }
    else {
        tmp = end();
    }

    return tmp;
}

Value::iterator Value::insert(const_iterator pos, Value&& value) {
    iterator tmp;

    if (is_array() && pos.is_array()) {
        tmp = m_array.insert(pos.m_array_iterator, std::move(value));
    }
    else if (is_object() && pos.is_object() && value.is_object()) {
        for (auto it = value.cbegin(); value.cend() != it; ++it, ++pos) {
            if (!is_member(it.key())) {
                tmp = m_object.insert(pos.m_object_iterator,
                            Pair(it.key(), std::move(*it)));
            }
        }
    }
    else {
        tmp = end();
    }

    return tmp;
}

Value::iterator Value::insert(const_iterator pos,
        size_t count, const Value& value) {
    iterator tmp;

    while (0 < count--) {
        tmp = insert(pos++, value);
    }

    return tmp;
}

Value::iterator Value::insert(const_iterator pos,
        const_iterator first, const_iterator last) {
    iterator tmp;

    for (auto it = first; it < last; ++it) {
        tmp = insert(pos++, *it);
    }

    return tmp;
}

Value::iterator Value::insert(const_iterator pos,
        std::initializer_list<Value> init_list) {
    return insert(pos, init_list.begin(), init_list.end());
}

Value& Value::operator[](std::size_t index) {
    if (is_null()) { *this = Type::ARRAY; }

    Value* ptr;

    if (is_array()) {
        if (size() == index) {
            m_array.emplace_back(nullptr);
        }
        ptr = &m_array[index];
    }
    else  if (is_object()) {
        ptr = &m_object[index].second;
    }
    else {
        ptr = this;
    }

    return *ptr;
}

const Value& Value::operator[](std::size_t index) const {
    const Value* ptr;

    if (is_array()) {
        ptr = &m_array[index];
    }
    else if (is_object()) {
        ptr = &m_object[index].second;
    }
    else {
        ptr = this;
    }

    return *ptr;
}

Value& Value::operator[](const char* key) {
    if (!is_object()) {
        if (is_null()) { *this = Type::OBJECT; }
        else { return *this; }
    }

    for (auto& pair : m_object) {
        if (pair.first == key) {
            return pair.second;
        }
    }

    m_object.emplace_back(key, nullptr);
    return m_object.back().second;
}

const Value& Value::operator[](const char* key) const {
    static const Value null_value{};

    if (!is_object()) { return *this; }

    for (const auto& pair : m_object) {
        if (pair.first == key) {
            return pair.second;
        }
    }

    return null_value;
}

void Value::push_back(const Value& value) {
    if (is_null()) { *this = Value(Type::ARRAY); }

    if (is_array()) {
        m_array.push_back(value);
    }
}

void Value::push_back(const Pair& pair) {
    if (is_null()) { *this = Type::OBJECT; }

    if (is_object()) {
        (*this)[pair.first] = pair.second;
    }
    else if (is_array()) {
        m_array.push_back(pair);
    }
}

void Value::pop_back() {
    if (is_array()) {
        m_array.pop_back();
    }
    else if (is_object()) {
        m_object.pop_back();
    }
    else {
        *this = Type::NIL;
    }
}

void Value::swap(Value& value) {
    Value temp(std::move(value));
    value = std::move(*this);
    *this = std::move(temp);
}

bool Value::is_member(const char* key) const {
    if (!is_object()) { return false; }

    for (const auto& pair : m_object) {
        if (pair.first == key) {
            return true;
        }
    }

    return false;
}

json::String& Value::as_string() {
    if (Type::STRING != m_type) {
        throw ValueError(ValueError::NOT_STRING);
    }
    return m_string;
}

const json::String& Value::as_string() const {
    if (Type::STRING != m_type) {
        throw ValueError(ValueError::NOT_STRING);
    }
    return m_string;
}

const char* Value::as_char() const {
    if (Type::STRING != m_type) {
        throw ValueError(ValueError::NOT_STRING);
    }
    return m_string.c_str();
}

json::Bool Value::as_bool() const {
    if (Type::BOOLEAN != m_type) {
        throw ValueError(ValueError::NOT_BOOLEAN);
    }
    return m_boolean;
}

json::Null Value::as_null() const {
    if (Type::NIL != m_type) {
        throw ValueError(ValueError::NOT_NULL);
    }
    return nullptr;
}

json::Int Value::as_int() const {
    if (Type::NUMBER != m_type) {
        throw ValueError(ValueError::NOT_NUMBER);
    }
    return Int(m_number);
}

json::Uint Value::as_uint() const {
    if (Type::NUMBER != m_type) {
        throw ValueError(ValueError::NOT_NUMBER);
    }
    return Uint(m_number);
}

json::Double Value::as_double() const {
    if (Type::NUMBER != m_type) {
        throw ValueError(ValueError::NOT_NUMBER);
    }
    return Double(m_number);
}

json::Array& Value::as_array() {
    if (Type::ARRAY != m_type) {
        throw ValueError(ValueError::NOT_ARRAY);
    }
    return m_array;
}

json::Number& Value::as_number() {
    if (Type::NUMBER != m_type) {
        throw ValueError(ValueError::NOT_NUMBER);
    }
    return m_number;
}

const json::Array& Value::as_array() const {
    if (Type::ARRAY != m_type) {
        throw ValueError(ValueError::NOT_ARRAY);
    }
    return m_array;
}

const json::Object& Value::as_object() const {
    if (Type::OBJECT != m_type) {
        throw ValueError(ValueError::NOT_OBJECT);
    }
    return m_object;
}

const json::Number& Value::as_number() const {
    if (Type::NUMBER != m_type) {
        throw ValueError(ValueError::NOT_NUMBER);
    }
    return m_number;
}

bool Value::operator==(const json::Value& other) const {
    if (m_type != other.m_type) { return false; }
    bool result;

    switch (m_type) {
    case Value::Type::OBJECT:
        result = (m_object == other.m_object);
        break;
    case Value::Type::ARRAY:
        result = (m_array == other.m_array);
        break;
    case Value::Type::STRING:
        result = (m_string == other.m_string);
        break;
    case Value::Type::NUMBER:
        result = (m_number == other.m_number);
        break;
    case Value::Type::BOOLEAN:
        result = (m_boolean == other.m_boolean);
        break;
    case Value::Type::NIL:
        result = true;
        break;
    default:
        result = false;
        break;
    }

    return result;
}

bool Value::operator<(const json::Value& val) const {
    if (m_type != val.m_type) { return false; }
    bool result;

    switch (m_type) {
    case Value::Type::OBJECT:
        result = (m_object < val.m_object);
        break;
    case Value::Type::ARRAY:
        result = (m_array < val.m_array);
        break;
    case Value::Type::STRING:
        result = (m_string < val.m_string);
        break;
    case Value::Type::NUMBER:
        result = (m_number < val.m_number);
        break;
    case Value::Type::BOOLEAN:
        result = (m_boolean < val.m_boolean);
        break;
    case Value::Type::NIL:
    default:
        result = false;
        break;
    }

    return result;
}

Value::iterator Value::begin() {
    iterator tmp;

    if (is_array()) {
        tmp = m_array.begin();
    }
    else if (is_object()) {
        tmp = m_object.begin();
    }
    else {
        tmp = this;
    }

    return tmp;
}

Value::iterator Value::end() {
    iterator tmp;

    if (is_array()) {
        tmp = m_array.end();
    }
    else if (is_object()) {
        tmp = m_object.end();
    }
    else {
        tmp = this;
    }

    return tmp;
}

Value::const_iterator Value::cbegin() const {
    const_iterator tmp;

    if (is_array()) {
        tmp = m_array.cbegin();
    }
    else if (is_object()) {
        tmp = m_object.cbegin();
    }
    else {
        tmp = this;
    }

    return tmp;
}

Value::const_iterator Value::cend() const {
    const_iterator tmp;

    if (is_array()) {
        tmp = m_array.cend();
    }
    else if (is_object()) {
        tmp = m_object.cend();
    }
    else {
        tmp = this;
    }

    return tmp;
}

Value::const_iterator Value::begin() const {
    return cbegin();
}

Value::const_iterator Value::end() const {
    return cend();
}
