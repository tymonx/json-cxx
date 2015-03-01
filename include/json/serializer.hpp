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

#ifndef _JSON_SERIALIZER_HPP_
#define _JSON_SERIALIZER_HPP_

#include "json.hpp"

namespace json {

/*!
 * @brief JSON serialization from JSON C++ objects to string stream that
 * contains JSON objects {} or arrays []
 * */
class Serializer {
public:
    /*! JSON serialization mode */
    enum class Mode {
        COMPACT,    /*!< No whitespaces (any spaces, newlines etc.) */
        PRETTY      /*!< Human readable mode */
    };

    /*! Default number of spaces for indentation. Default is 4 */
    static const size_t DEFAULT_INDENT;

    /*! Default serialization mode. Default is Mode::COMPACT */
    static const Mode DEFAULT_MODE;

    /*!
     * @brief Default constructor
     *
     * Create JSON serializer object with default settings
     * */
    Serializer(Mode mode = DEFAULT_MODE);

    /*!
     * @brief Serializer JSON C++ object or JSON C++ array
     *
     * When JSON C++ object is null, store '{}'
     *
     * @param[in]   value   JSON C++ to serialize
     * @param[in]   mode    Serialization mode
     * */
    Serializer(const Value& value, Mode mode = DEFAULT_MODE);

    /*!
     * @brief Serialize JSON C++ object or array
     *
     * @param[in]   value   JSON C++ to serialize
     * */
    Serializer& operator<<(const Value& value);

    /*!
     * @brief Clear serialization content
     * */
    void clear();

    /*!
     * @brief Set serialization mode
     *
     * @param[in]   mode    Serialization mode
     * */
    void set_mode(Mode mode);

    /*!
     * @brief Enable adding newlines for serializing JSON C++ values
     *
     * Newlines are added after new value in JSON array or new member in
     * JSON object
     *
     * @note
     * This setting may be changed by set_mode()
     *
     * @param[in]   enable  Enable adding newlines
     * */
    void enable_newline(bool enable = true);

    /*!
     * @brief Set number of spaces for indentation
     *
     * Spaces are added after new value in JSON array or new member in
     * JSON object
     *
     * @note
     * This setting may be changed by set_mode()
     *
     * @param[in]   indent  Number of spaces used for indentation
     * */
    void set_indent(size_t indent);

    /*!
     * @brief Flush serialized JSON C++ values to string
     *
     * After invoking operator<<(), serialization content will be also clear
     *
     * @return  String appended with serialized JSON C++ values
     * */
    friend String& operator<<(String&, Serializer&);

    /*!
     * @brief Flush serialized JSON C++ values to string
     *
     * After invoking operator<<(), serialization content will be also clear.
     * Using move semantics
     *
     * @return  String appended with serialized JSON C++ values
     * */
    friend String& operator<<(String&, Serializer&&);

    /*!
     * @brief Flush serialized JSON C++ values to output stream
     *
     * After invoking operator<<(), serialization content will be also clear
     *
     * @return  Output stream appended with serialized JSON C++ values
     * */
    friend std::ostream& operator<<(std::ostream&, Serializer&);

    /*!
     * @brief Flush serialized JSON C++ values to output stream
     *
     * After invoking operator<<(), serialization content will be also clear
     * Using move semantics
     *
     * @return  Output stream appended with serialized JSON C++ values
     * */
    friend std::ostream& operator<<(std::ostream&, Serializer&&);
private:
    String m_serialized;
    size_t m_level;
    size_t m_indent;
    bool m_enable_newline;
    size_t m_colon_start;
    size_t m_colon_stop;

    void write_object(const Value& value);
    void write_value(const Value& value);
    void write_array(const Value& value);
    void write_number(const Value& value);
    void write_string(const Value& value);
    void write_boolean(const Value& value);
    void write_empty(const Value& value);
};

String& operator<<(String&, Serializer&);
std::ostream& operator<<(std::ostream&, Serializer&);

String& operator<<(String&, Serializer&&);
std::ostream& operator<<(std::ostream&, Serializer&&);

}

#endif /* _JSON_SERIALIZER_HPP_ */
