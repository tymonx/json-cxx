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
 * @file json/value.hpp
 *
 * @brief JSON value interface
 * */

#ifndef JSON_CXX_VALUE_HPP
#define JSON_CXX_VALUE_HPP

#include "number.hpp"

#include <string>
#include <vector>
#include <utility>

namespace json {

template<bool is_const>
class base_iterator;

class Value;

/*! JSON string */
using String = std::string;

/*! JSON member pair key:value */
using Pair = std::pair<String, Value>;

/*! JSON object that contain JSON members */
using Object = std::vector<Pair>;

/*! JSON array that contains JSON values */
using Array = std::vector<Value>;

/*! JSON boolean */
using Bool = bool;

/*! JSON null */
using Null = std::nullptr_t;

/*!
 * @brief JSON value
 *
 * JSON value class that can contain JSON object, array, number, boolen or null
 * */
class Value {
public:
    friend class Deserializer;

    /*! Non-const iterator that can iterate through JSON values */
    using iterator = base_iterator<false>;

    /*! Const iterator that can iterate through JSON values */
    using const_iterator = base_iterator<true>;

    /*! JSON type */
    enum class Type {
        NIL,
        OBJECT,
        ARRAY,
        STRING,
        NUMBER,
        BOOLEAN
    };

    /*!
     * @brief Default constructor
     *
     * Without any arguments creates JSON null. Passed Type to constructor will
     * create given JSON type with zero initialization. For object/array
     * creates empty container, for boolean type set to false, for
     * number set to zero and for string set to ""
     *
     * @param[in]   type    JSON type
     * */
    Value(Type type = Type::NIL);

    /*!
     * @brief Create JSON null
     *
     * This constructor is equivalent to default constructor without any given
     * arguments or with Type::NIL
     *
     * @code
     * Value value(nullptr);
     * @endcode
     *
     * @param[in]   null    Valid argument is only nullptr
     * */
    Value(Null null);

    /*!
     * @brief Create JSON boolean
     *
     * Initialization with given boolean argument
     *
     * @param[in]   boolean  May be true or false
     * */
    Value(Bool boolean);

    /*!
     * @brief Create JSON string
     *
     * Create JSON string with given null-terminated characters array.
     * String will be copied
     *
     * param[in]    str     Null-terminated characters array
     * */
    Value(const char* str);

    /*!
     * @brief Create JSON string
     *
     * Create JSON string with given String object.
     * String will be copied
     *
     * param[in]    str     String object
     * */
    Value(const String& str);

    /*!
     * @brief Create JSON object with member
     *
     * Initialize object as JSON object with one given JSON member as pair
     * that contains key:value
     *
     * @param[in]   pair    JSON pair key:value
     * */
    Value(const Pair& pair);

    /*!
     * @brief Create JSON object with member
     *
     * Initialize object as JSON object with one given JSON member as
     * separate key string and JSON value. Key string is null-terminated
     * characters array
     *
     * @param[in]   key     Unique key string for value
     * @param[in]   value   JSON value
     * */
    Value(const char* key, const Value& value);

    /*!
     * @brief Create JSON object with member
     *
     * Initialize object as JSON object with one given JSON member as
     * separate key string and JSON value
     *
     * @param[in]   key     Unique key string for value
     * @param[in]   value   JSON value
     * */
    Value(const String& key, const Value& value);

    /*!
     * @brief Create JSON number as unsigned int
     *
     * Initialize number with given value
     *
     * @param[in]   value   Unsigned integer
     * */
    Value(Uint value);

    /*!
     * @brief Create JSON number as unsigned int
     *
     * Initialize number with given value
     *
     * @param[in]   value   Unsigned integer
     * */
    Value(Int value);

    /*!
     * @brief Create JSON number as double
     *
     * Initialize number with given value
     *
     * @param[in]   value   Double
     * */
    Value(Double value);

    /*!
     * @brief Create JSON number
     *
     * Initialize object with given JSON number
     *
     * @param[in]   number   JSON Number
     * */
    Value(const Number& number);

    /*!
     * @brief Create JSON array with copies of JSON values
     *
     * @param[in]   count   Array size
     * @param[in]   value   JSON value used to fill JSON array
     * */
    Value(size_t count, const Value& value);

    /*!
     * @brief Create JSON object with given JSON members
     *
     * @param[in]   init_list   JSON members list used to fill JSON object
     * */
    Value(std::initializer_list<Pair> init_list);

    /*!
     * @brief Create JSON array with given JSON values
     *
     * @param[in]   init_list   JSON values list used to fill JSON array
     * */
    Value(std::initializer_list<Value> init_list);

    /*!
     * @brief Copy constructor
     * */
    Value(const Value& value);

    /*!
     * @brief Move constructor
     *
     * After moving JSON value to new object, given JSON value is changed to
     * JSON null
     * */
    Value(Value&& value);

    /*!
     * @brief Destructor
     *
     * Call suitable JSON type destructor based on his Type
     * */
    ~Value();

    /*!
     * @brief Copy assignment
     *
     * If JSON values type are not equal, destroy old contents, create new JSON
     * type based on given JSON value argument and copy all data
     *
     * @param[in]   value   JSON value to copy
     * */
    Value& operator=(const Value& value);

    /*!
     * @brief Move assignment
     *
     * If JSON values type are not equal, destroy old contents, create new JSON
     * type based on given JSON value argument and move all data. JSON value
     * that data have been moved to object, will be changed to JSON null
     *
     * @param[in]   value   JSON value to move
     * */
    Value& operator=(Value&& value);

    /*!
     * @brief Assignment JSON object with JSON members
     *
     * Destroy old contents and assignment new value. JSON type will be changed
     * to JSON object
     *
     * @param[in]   init_list   JSON object with JSON members
     * */
    Value& operator=(std::initializer_list<Pair> init_list);

    /*!
     * @brief Assignment JSON array with JSON values
     *
     * Destroy old contents and assignment new value. JSON type will be changed
     * to JSON array
     *
     * @param[in]   init_list   JSON array with JSON values
     * */
    Value& operator=(std::initializer_list<Value> init_list);

    /*!
     * @brief Add new JSON value to JSON array
     *
     * If object is a JSON array, add new JSON value to the end
     * If object is a JSON object and given JSON value is a member or an object,
     * add contents to the end
     * If object is a JSON string and given JSON value is also a string, append
     * existing string with new content at the end
     *
     * param[in]    value   JSON value
     * */
    Value& operator+=(const Value& value);

    /*!
     * @brief Assign JSON array with JSON value copies
     *
     * Replace actual content with new JSON array that holds JSON value copies.
     * JSON Type will be changed to JSON array
     *
     * @param[in]   count   Array size
     * @param[in]   value   JSON value copies
     * */
    void assign(size_t count, const Value& value);

    /*!
     * @brief Assign JSON object with JSON members
     *
     * Replace actual content with new JSON objects that holds JSON members.
     * JSON Type will be changed to JSON object
     *
     * @param[in]   init_list   JSON object with JSON members
     * */
    void assign(std::initializer_list<Pair> init_list);

    /*!
     * @brief Assign JSON array with JSON values
     *
     * Replace actual content with new JSON array that holds JSON values.
     * JSON Type will be changed to JSON array
     *
     * @param[in]   init_list   JSON array with JSON values
     * */
    void assign(std::initializer_list<Value> init_list);

    /*!
     * @brief Push at the end new JSON member
     *
     * This work only for JSON object type. When JSON is a null, JSON type
     * will be changed to JSON object and then perform push_back() method
     *
     * @param[in]   pair    JSON member key:value
     * */
    void push_back(const Pair& pair);

    /*!
     * @brief Push at the end new JSON value
     *
     * This work only for JSON array type. When JSON is a null, JSON type
     * will be changed to JSON array and then perform push_back() method
     *
     * @param[in]   value    JSON value
     * */
    void push_back(const Value& value);

    /*!
     * @brief Pop JSON value or member from JSON array or object
     *
     * This work only for JSON array or object type
     * */
    void pop_back();

    /*!
     * @brief Get number of elements in JSON array or object type
     *
     * For any others JSON types it returns always zero
     *
     * @return  Number of JSON array or object elements
     * */
    size_t size() const;

    /*!
     * @brief Clear
     *
     * For JSON array or object clear all elements, the size will be zero
     * For JSON string clear to empty "" string
     * For JSON number clear to zero
     * For JSON boolean change value to false
     * */
    void clear();

    /*!
     * @brief Check if JSON array or object is empty
     *
     * Equivalent to !size() method
     *
     * @return   true when container is empty otherwise false
     * */
    bool empty() const;

    /*!
     * @brief Erase member from JSON object with given key
     *
     * Available only for JSON object
     *
     * @param[in]   key     String object
     * */
    size_t erase(const String& key);

    /*!
     * @brief Erase member from JSON object with given key
     *
     * Available only for JSON object
     *
     * @param[in]   key     Null-terminated characters array
     * */
    size_t erase(const char* key);

    iterator erase(const_iterator pos);

    iterator erase(const_iterator first, const_iterator last);

    iterator insert(const_iterator pos, const Value& value);

    iterator insert(const_iterator pos, Value&& value);

    iterator insert(const_iterator pos, size_t count, const Value& value);

    iterator insert(const_iterator pos,
            const_iterator first, const_iterator last);

    iterator insert(const_iterator pos, std::initializer_list<Value> init_list);

    /*!
     * @brief Swap JSON values
     *
     * @param[in]   value JSON value to swap
     * */
    void swap(Value& value);

    /*!
     * @brief Array access to JSON value in JSON array or JSON member in JSON
     * object
     *
     * When object is null type, after calling operator[] change type to JSON
     * array. When index is equal to size(), append only JSON array with new
     * JSON null element at the end
     *
     * @code
     *  Value value;    // Create JSON null
     *  value[0] = 1;   // Change type to JSON array and assign number
     *  value[1];       // Append array with new JSON null element
     * @endcode
     *
     * @param[in]   index   Element index access
     *
     * @return  JSON value indexed in array or object, otherwise return myself
     * */
    Value& operator[](size_t index);

    /*!
     * @brief Array access to JSON value in JSON array or JSON member in JSON
     * object
     *
     * When object is not neither array or object, return myself
     *
     * @param[in]   index   Element index access
     *
     * @return  JSON value indexed in array or object, otherwise return myself
     * */
    const Value& operator[](size_t index) const;

    /*!
     * @brief Array access to JSON value in JSON array or JSON member in JSON
     * object
     *
     * When object is null type, after calling operator[] change type to JSON
     * array. When index is equal to size(), append only JSON array with new
     * JSON null element at the end
     *
     * @code
     *  Value value;    // Create JSON null
     *  value[0] = 1;   // Change type to JSON array and assign number
     *  value[1];       // Append array with new JSON null element
     * @endcode
     *
     * @param[in]   index   Element index access
     *
     * @return  JSON value indexed in array or object, otherwise return myself
     * */
    Value& operator[](int index);

    /*!
     * @brief Array access to JSON value in JSON array or JSON member in JSON
     * object
     *
     * When object is neither array or object, return myself
     *
     * @param[in]   index   Element index access
     *
     * @return  JSON value indexed in array or object, otherwise return myself
     * */
    const Value& operator[](int index) const;

    /*!
     * @fn Value::operator[](const char*)
     * @brief Get key
     * */
    Value& operator[](const char* key);

    /*!
     * @fn Value::operator[](const char*) const
     * @brief Get key
     * */
    const Value& operator[](const char* key) const;

    /*!
     * @fn Value::operator[](const String&)
     * @brief Get key
     * */
    Value& operator[](const String& key);

    /*!
     * @fn Value::operator[](const String&) const
     * @brief Get key
     * */
    const Value& operator[](const String& key) const;

    Type get_type() const;

    bool is_member(const std::string& key) const;

    bool is_member(const char* key) const;

    bool is_string() const;

    bool is_object() const;

    bool is_array() const;

    bool is_number() const;

    bool is_boolean() const;

    bool is_null() const;

    bool is_int() const;

    bool is_uint() const;

    bool is_double() const;

    explicit operator String&();

    explicit operator const String&() const;

    explicit operator const char*() const;

    explicit operator Bool() const;

    explicit operator Null() const;

    explicit operator Int() const;

    explicit operator Uint() const;

    explicit operator Double() const;

    explicit operator Array&();

    explicit operator Number&();

    explicit operator const Array&() const;

    explicit operator const Object&() const;

    explicit operator const Number&() const;

    bool operator!() const;

    friend bool operator==(const Value&, const Value&);
    friend bool operator!=(const Value&, const Value&);
    friend bool operator<(const Value&, const Value&);
    friend bool operator>(const Value&, const Value&);
    friend bool operator<=(const Value&, const Value&);
    friend bool operator>=(const Value&, const Value&);

    iterator begin();

    iterator end();

    const_iterator begin() const;

    const_iterator end() const;

    const_iterator cbegin() const;

    const_iterator cend() const;

private:
    enum Type m_type;

    union {
        Object m_object;
        Array m_array;
        String m_string;
        Number m_number;
        Bool m_boolean;
    };

    void create_container(Type type);
};

bool operator==(const Value&, const Value&);
bool operator!=(const Value&, const Value&);
bool operator< (const Value&, const Value&);
bool operator> (const Value&, const Value&);
bool operator<=(const Value&, const Value&);
bool operator>=(const Value&, const Value&);

} /* namespace json */

#endif /* JSON_CXX_VALUE_HPP */
