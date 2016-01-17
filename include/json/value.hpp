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
 * @file json/value.hpp
 *
 * @brief JSON value interface
 * */

#ifndef JSON_CXX_VALUE_HPP
#define JSON_CXX_VALUE_HPP

#include <json/types.hpp>
#include <json/number.hpp>
#include <json/string.hpp>
#include <json/object.hpp>
#include <json/array.hpp>

namespace json {

class Value {
public:
    friend class Parser;

    enum Type {
        NIL,
        OBJECT,
        ARRAY,
        STRING,
        NUMBER,
        BOOL
    };

    Value() :
        m_type{Type::NIL}
    { }

    Value(Null) :
        m_type{Type::NIL}
    { }

    Value(Type type);

    Value(const Value& other);

    Value(Value&& other);

    Value& operator=(const Value& other) {
        return *this = Value(other);
    }

    Value& operator=(Value&& other);

    Value& operator=(Null) {
        this->~Value();
        m_type = Type::NIL;
        return *this;
    }

    Value& operator=(Type type) {
        return *this = Value(type);
    }

    /*!
     * @brief Get JSON type
     * */
    Type get_type() const { return m_type; }

    /*!
     * @brief Check if JSON value is a string
     * @return true when is otherwise false
     * */
    bool is_string() const { return Type::STRING == m_type; }

    /*!
     * @brief Check if JSON value is a object
     * @return true when is otherwise false
     * */
    bool is_object() const { return Type::OBJECT == m_type; }

    /*!
     * @brief Check if JSON value is a array
     * @return true when is otherwise false
     * */
    bool is_array() const { return Type::ARRAY == m_type; }

    /*!
     * @brief Check if JSON value is a number
     * @return true when is otherwise false
     * */
    bool is_number() const { return Type::NUMBER == m_type; }

    /*!
     * @brief Check if JSON value is a boolean
     * @return true when is otherwise false
     * */
    bool is_bool() const { return Type::BOOL == m_type; }

    /*!
     * @brief Check if JSON value is a null
     * @return true when is otherwise false
     * */
    bool is_null() const { return Type::NIL == m_type; }

    /*!
     * @brief Check if JSON value is a signed integer
     * @return true when is otherwise false
     * */
    bool is_int() const {
        return is_number() ? m_number.is_int() : false;
    }

    /*!
     * @brief Check if JSON value is a unsigned integer
     * @return true when is otherwise false
     * */
    bool is_uint() const {
        return is_number() ? m_number.is_uint() : false;
    }

    /*!
     * @brief Check if JSON value is a double
     * @return true when is otherwise false
     * */
    bool is_double() const {
        return is_number() ? m_number.is_double() : false;
    }

    /*! Equivalent to is_null() */
    bool operator!() const { return is_null(); }

    /*! Convert JSON value to string */
    explicit operator String&() { return m_string; }

    /*! Convert JSON value to string */
    explicit operator const String&() const { return m_string; }

    /*! Convert JSON value to string */
    explicit operator const char*() const { return m_string.data(); }

    /*! Convert JSON value to boolean */
    explicit operator Bool() const { return m_bool; }

    /*! Convert JSON value to null */
    explicit operator Null() const { return nullptr; }

    /*! Convert JSON value to signed integer */
    explicit operator Int() const { return Int(m_number); }

    /*! Convert JSON value to unsigned integer */
    explicit operator Uint() const { return Uint(m_number); }

    /*! Convert JSON value to double */
    explicit operator Double() const { return Double(m_number); }

    /*! Convert JSON value to array */
    explicit operator Array&() { return m_array; }

    /*! Convert JSON value to number */
    explicit operator Number&() { return m_number; }

    /*! Convert JSON value to array */
    explicit operator const Array&() const { return m_array; }

    /*! Convert JSON value to object */
    explicit operator const Object&() const { return m_object; }

    /*! Convert JSON value to number */
    explicit operator const Number&() const { return m_number; }

    ~Value();
private:
    Type m_type;

    union {
        Object m_object;
        Array m_array;
        String m_string;
        Number m_number;
        Bool m_bool;
    };
};

}

#include <json/pair.hpp>

#endif /* JSON_CXX_VALUE_HPP */
