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

#include <exception>
#include <cstring>
#include <cstdint>

namespace json {

/*!
 * @brief JSON deserialization from string stream contains JSON objects {} or
 * arrays [] to JSON C++ value
 * */
class Deserializer {
public:
    /*!
     * @brief Default constructor
     *
     * No JSON values stored, no parsing started
     * */
    Deserializer() { }

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
    Deserializer(const char* str) {
        parsing(str);
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
    Deserializer(const char* str, std::size_t length) {
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
    template<std::size_t N>
    Deserializer(const char str[N]) {
        parsing(str);
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
    Deserializer(const String& str) {
        parsing(str);
    }

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
        parsing(str);
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
    Deserializer& operator<<(const String& str) {
        parsing(str);
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
    void set_limit(std::size_t limit = MAX_LIMIT_PER_OBJECT) {
        m_limit = limit;
    }

    void clear() {
        m_value = nullptr;
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

    /*! JSON error parsing */
    class Error : public std::exception {
    public:
        /*! Error parsing codes */
        enum Code {
            NONE,
            END_OF_FILE,
            MISS_VALUE,
            MISS_QUOTE,
            MISS_COLON,
            MISS_CURLY_CLOSE,
            MISS_SQUARE_CLOSE,
            NOT_MATCH_NULL,
            NOT_MATCH_TRUE,
            NOT_MATCH_FALSE,
            INVALID_WHITESPACE,
            INVALID_ESCAPE,
            INVALID_UNICODE,
            INVALID_NUMBER_INTEGER,
            INVALID_NUMBER_FRACTION,
            INVALID_NUMBER_EXPONENT
        };

        Error(Code code, const char* cbegin, const char* cend,
                const char* position);

        Error(const Error&) = default;
        Error(Error&&) = default;
        Error& operator=(const Error&) = default;
        Error& operator=(Error&&) = default;

        /*!
         * @brief Return error explanatory string
         *
         * @return  When success return decoded error code as a human readable
         *          message, otherwise return empty string ""
         * */
        virtual const char* what() const noexcept;

        Code get_code() const { return m_code; }

        std::size_t get_line() const { return m_line; }

        std::size_t get_column() const { return m_column; }

        std::size_t get_offset() const { return m_offset; }

        virtual ~Error();
    private:
        /*! Error parsing code */
        Code m_code;
        /*! Line position indicative error */
        std::size_t m_line;
        /*! Column number indicative error */
        std::size_t m_column;
        /*! Column number indicative error */
        std::size_t m_offset;

        const char* m_message;
    };
private:
    /*! Stack protection */
    static const std::size_t MAX_LIMIT_PER_OBJECT;

    Value m_value = nullptr;
    std::size_t m_limit = MAX_LIMIT_PER_OBJECT;

    const char* m_begin;
    const char* m_current;
    const char* m_end;

    void read_object(Value& value);
    void read_object_member(Value& value, std::size_t& count);
    void read_string(String& str);
    void read_string_unicode(String& str);
    void read_string_escape(String& str);
    void read_value(Value& value);
    void read_array(Value& value);
    void read_array_element(Value& value, std::size_t& count);
    void read_colon();
    void read_quote();
    void read_true(Value& value);
    void read_false(Value& value);
    void read_null(Value& value);
    void read_number(Value& value);
    void read_number_digit(Uint64& str);
    void read_number_integer(Number& number);
    void read_number_fractional(Number& number);
    void read_number_exponent(Number& number);
    void read_unicode(const char** pos, std::uint32_t& code);
    void read_whitespaces(bool enable_error = true);
    void count_string_chars(std::size_t& count);

    [[noreturn]] void throw_error(Error::Code code);
};

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
Deserializer operator>>(const char* str, Value& value) {
    Deserializer deserializer(str);
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
Deserializer operator>>(const char str[N], Value& value) {
    Deserializer deserializer(str);
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
Deserializer operator>>(const String& str, Value& value) {
    Deserializer deserializer(str);
    value = deserializer.get_value();
    return deserializer;
}

}

#endif /* JSON_CXX_DESERIALIZER_HPP */
