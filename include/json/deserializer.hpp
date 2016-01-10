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
 * @file deserializer.hpp
 *
 * @brief JSON deserializer interface
 * */

#ifndef JSON_CXX_DESERIALIZER_HPP
#define JSON_CXX_DESERIALIZER_HPP

#include <json/value.hpp>

#include <string>
#include <cstring>
#include <cstdint>
#include <limits>

namespace json {

/*!
 * @brief JSON deserialization from string stream contains JSON objects {} or
 * arrays [] to JSON C++ value
 * */
class Deserializer {
public:
    static constexpr const std::size_t DEFAULT_LIMIT_PER_OBJECT =
        std::numeric_limits<std::uint32_t>::max();

    /*!
     * @brief Default constructor
     *
     * No JSON values stored, no parsing started
     * */
    Deserializer();

    /*!
     * @brief JSON deserialization from null-terminated character array
     * to JSON C++ object or array
     *
     * This constructor will start parsing string and store valid JSON
     * object {} or array [] on the stack. To pop from stack use operator>>()
     *
     * @param[in]   str     String contains JSON objects {} or arrays [].
     *                      It may contains whitespaces (spaces, newlines,
     *                      tabulations or carriage returns)
     * */
    Deserializer(const char* str, std::size_t length) : Deserializer() {
        parsing(str, length);
    }

    /*!
     * @brief JSON deserialization from null-terminated character array
     * to JSON C++ object or array
     *
     * This constructor will start parsing string and store valid JSON
     * object {} or array [] on the stack. To pop from stack use operator>>()
     *
     * @param[in]   str     String contains JSON objects {} or arrays [].
     *                      It may contains whitespaces (spaces, newlines,
     *                      tabulations or carriage returns)
     * */
    Deserializer(const char* str) : Deserializer() {
        parsing(str, std::strlen(str));
    }

    /*!
     * @brief JSON deserialization from String to JSON C++ object or array
     *
     * This constructor will start parsing string and store valid JSON
     * objects {} or arrays [] on the stack. To pop from stack
     * use operator>>()
     *
     * @param[in]   str     String contains JSON objects {} or arrays [].
     *                      It may contains whitespaces (spaces, newlines,
     *                      tabulations or carriage returns)
     * */
    Deserializer(const std::string& str) : Deserializer() {
        parsing(str.c_str(), str.length());
    }

    /*!
     * @brief JSON deserialization from null-terminated character array
     * to JSON C++ object or array
     *
     * This constructor will start parsing string and store valid JSON
     * object {} or array [] on the stack. To pop from stack use operator>>()
     *
     * @param[in]   str     String contains JSON objects {} or arrays [].
     *                      It may contains whitespaces (spaces, newlines,
     *                      tabulations or carriage returns)
     * */
    template<std::size_t N>
    Deserializer(const char str[N]) : Deserializer() {
        parsing(str, N);
    }

    Deserializer(const Deserializer&) = default;
    Deserializer(Deserializer&&) = default;
    Deserializer& operator=(const Deserializer&) = default;
    Deserializer& operator=(Deserializer&&) = default;

    ~Deserializer();

    /*!
     * @brief Start parsing null-terminated character array that contains JSON
     * objects {} or arrays []
     *
     * All parsed and valid JSON values are stored on the stack. To pop from
     * stack use operator>>()
     *
     * @param[in]   str     String contains JSON objects {} or arrays [].
     *                      It may contains whitespaces (spaces, newlines,
     *                      tabulations or carriage returns)
     * */
    Deserializer& operator<<(const char* str) {
        parsing(str, std::strlen(str));
        return *this;
    }

    /*!
     * @brief Start parsing String that constains JSON objects {} or arrays []
     *
     * All parsed and valid JSON values are stored on the stack.
     * To pop from stack use operator>>()
     *
     * @param[in]   str     String contains JSON objects {} or arrays [].
     *                      It may contains whitespaces (spaces, newlines,
     *                      tabulations or carriage returns)
     * */
    Deserializer& operator<<(const std::string& str) {
        parsing(str.c_str(), str.length());
        return *this;
    }

    /*!
     * @brief Pop parsed single JSON value from the stack
     *
     * JSON value may be a JSON object or an array. Check the stack size
     * with size() or empty()
     *
     * @param[out]  value   JSON value to store from the stack
     * */
    Deserializer& operator>>(Value& value) {
        value = m_value;
        return *this;
    }

    void clear();

    /*!
     * @brief Set maximum characters to parse per JSON object or array
     *
     * This limitation protect application stack from reach out of the
     * memory or from stack attack
     *
     * @param[in]   limit   Set maximum characters to parse per one valid
     *                      JSON object or array. Invoking method without
     *                      any argument sets default limitation
     * */
    void set_limit(std::size_t limit) {
        m_limit = limit;
    }

    const Value& get_value() const {
        return m_value;
    }

    void parsing(const char* str, std::size_t length);

    void parsing(const std::string& str) {
        parsing(str.c_str(), str.length());
    }

    void parsing(const char* str) {
        parsing(str, std::strlen(str));
    }

    template<std::size_t N>
    void parsing(const char str[N]) {
        parsing(str, N - 1);
    }
private:
    Value m_value = nullptr;
    std::size_t m_limit{DEFAULT_LIMIT_PER_OBJECT};
};

}

/*!
 * @brief Parsing given null-terminated character array and store all parsed
 * JSON objects {} or arrays [] on the stack
 *
 * Because this use stack to store all parsed JSON objects, only last
 * parsed data will be pop from stack and stored to given value output.
 * operator>>() behave like input stream and may be used in the chain:
 *
 * @code
 * R"({"key1":1, "key2":2}[1, 2])" >> value2 >> value1;
 * @endcode
 *
 * @param[in]   str     String contains JSON objects {} or arrays [].
 *                      It may contains whitespaces (spaces, newlines,
 *                      tabulations or carriage returns)
 *
 * @param[out]  value   JSON value to store from the stack
 * */
static inline
json::Deserializer operator>>(const char* str, json::Value& value) {
    json::Deserializer deserializer(str);
    value = deserializer.get_value();
    return deserializer;
}

/*!
 * @brief Parsing given String object and store all parsed JSON objects {}
 * or arrays [] on the stack
 *
 * Because this use stack to store all parsed JSON objects, only last
 * parsed data will be pop from stack and stored to given value output.
 * operator>>() behave like input stream and may be used in the chain:
 *
 * @code
 * R"({"key1":1, "key2":2}[1, 2])" >> value2 >> value1;
 * @endcode
 *
 * @param[in]   str     String contains JSON objects {} or arrays [].
 *                      It may contains whitespaces (spaces, newlines,
 *                      tabulations or carriage returns)
 *
 * @param[out]  value   JSON value to store from the stack
 * */
static inline
json::Deserializer operator>>(const std::string& str, json::Value& value) {
    json::Deserializer deserializer(str);
    value = deserializer.get_value();
    return deserializer;
}

/*!
 * @brief Parsing given null-terminated character array and store all parsed
 * JSON objects {} or arrays [] on the stack
 *
 * Because this use stack to store all parsed JSON objects, only last
 * parsed data will be pop from stack and stored to given value output.
 * operator>>() behave like input stream and may be used in the chain:
 *
 * @code
 * R"({"key1":1, "key2":2}[1, 2])" >> value2 >> value1;
 * @endcode
 *
 * @param[in]   str     String contains JSON objects {} or arrays [].
 *                      It may contains whitespaces (spaces, newlines,
 *                      tabulations or carriage returns)
 *
 * @param[out]  value   JSON value to store from the stack
 * */
template<std::size_t N>
json::Deserializer operator>>(const char str[N], json::Value& value) {
    return str >> value;
}

#endif /* JSON_CXX_DESERIALIZER_HPP */
