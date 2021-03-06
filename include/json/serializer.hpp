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
 * @file serializer.hpp
 *
 * @brief JSON serializer interface
 * */

#ifndef JSON_CXX_SERIALIZER_HPP
#define JSON_CXX_SERIALIZER_HPP

#include <json/value.hpp>
#include <json/formatter.hpp>

#include <string>

namespace json {

/*!
 * @brief JSON serialization from JSON C++ objects to string stream that
 * contains JSON objects {} or arrays []
 * */
class Serializer {
public:
    /*!
     * @brief Default constructor
     *
     * Create JSON serializer object with default settings
     * */
    Serializer(Formatter* formatter = nullptr);

    /*!
     * @brief Serializer JSON C++ object or JSON C++ array
     *
     * When JSON C++ object is null, store '{}'
     *
     * @param[in]   value   JSON C++ to serialize
     * */
    Serializer(const Value& value, Formatter* formatter = nullptr) :
            Serializer(formatter) { write(value); }

    void write(const Value& value);

    const std::string& read() const {
        return m_serialized;
    }

    /*!
     * @brief Clear serialization content
     * */
    void clear() {
        m_serialized.clear();
    }

    /*!
     * @brief Serialize JSON C++ object or array
     *
     * @param[in]   value   JSON C++ to serialize
     * */
    Serializer& operator<<(const Value& value) {
        write(value);
        return *this;
    }

    ~Serializer();
private:
    Serializer(const Serializer&) = delete;
    Serializer(Serializer&&) = delete;
    Serializer& operator=(const Serializer&) = delete;
    Serializer& operator=(Serializer&&) = delete;

    Formatter* m_formatter;
    std::string m_serialized;
};

}

/*!
 * @brief Flush serialized JSON C++ values to output stream
 *
 * After invoking operator<<(), serialization content will be also clear
 *
 * @return  Output stream appended with serialized JSON C++ values
 * */
static inline
std::ostream& operator<<(std::ostream& os, const json::Value& value) {
    json::Serializer serializer(value);
    return os << serializer.read();
}

/*!
 * @brief Flush serialized JSON C++ values to output stream
 *
 * After invoking operator<<(), serialization content will be also clear
 *
 * @return  Output stream appended with serialized JSON C++ values
 * */
static inline
std::ostream& operator<<(std::ostream& os,
        const json::Serializer& serializer) {
    return os << serializer.read();
}

/*!
 * @brief Flush serialized JSON C++ values to output stream
 *
 * After invoking operator<<(), serialization content will be also clear
 *
 * @return  Output stream appended with serialized JSON C++ values
 * */
static inline
std::string& operator<<(std::string& str, const json::Value& value) {
    json::Serializer serializer(value);
    return str = serializer.read();
}

/*!
 * @brief Flush serialized JSON C++ values to output stream
 *
 * After invoking operator<<(), serialization content will be also clear
 *
 * @return  Output stream appended with serialized JSON C++ values
 * */
static inline
std::string& operator<<(std::string& str,
        const json::Serializer& serializer) {
    return str = serializer.read();
}

#endif /* JSON_CXX_SERIALIZER_HPP */
