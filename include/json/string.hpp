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
 * @file json/string.hpp
 *
 * @brief JSON string interface
 * */

#ifndef JSON_CXX_STRING_HPP
#define JSON_CXX_STRING_HPP

#include <cstdint>

#include <limits>
#include <utility>
#include <initializer_list>

namespace json {

class String {
public:
    friend class Parser;

    static const constexpr std::size_t npos =
        std::numeric_limits<std::size_t>::max();

    String();

    String(std::size_t count, char ch);

    String(const String& other, std::size_t pos, std::size_t count = npos);

    String(const char* str, std::size_t count);

    String(const char* str);

    String(const char* first, const char* last);

    template<std::size_t N>
    String(const char str[N]) : String(str, N - 1) { }

    String(std::initializer_list<char> init);

    String(const String& other);

    String(String&& other);

    String& operator=(const String& other);

    String& operator=(String&& other);

    String& operator=(const char* s) {
        String str(s);
        return *this = std::move(str);
    }

    String& operator=(char ch) {
        String str(1, ch);
        return *this = std::move(str);
    }

    String& operator=(std::initializer_list<char> init) {
        String str(init);
        return *this = std::move(str);
    }

    String& assign(std::size_t count, char ch) {
        String str(count, ch);
        return *this = std::move(str);
    }

    String& assign(const String& other) {
        return *this = other;
    }

    String& assign(String&& other) {
        return *this = std::move(other);
    }

    String& assign(const String& other, std::size_t pos,
            std::size_t count = npos)
    {
        String str(other, pos, count);
        return *this = std::move(str);
    }

    String& assign(const char* s, std::size_t count) {
        String str(s, count);
        return *this = std::move(str);
    }

    String& assign(const char* s) {
        String str(s);
        return *this = std::move(str);
    }

    String& assign(const char* first, const char* last) {
        String str(first, last);
        return *this = std::move(str);
    }

    String& assign(std::initializer_list<char> init) {
        String str(init);
        return *this = std::move(str);
    }

    void swap(String& other);

    /*! @brief Access specified character
     *
     * */
    char& at(std::size_t pos) {
        return m_begin[pos];
    }

    /*! @brief Access specified character
     *
     * */
    const char& operator[](std::size_t pos) const {
        return m_begin[pos];
    }

    /*! @brief Access first element
     *
     * */
    char& front() {
        return *m_begin;
    }

    /*! @brief Access first element
     *
     * */
    const char& front() const {
        return *m_begin;
    }

    /*! @brief Access first element
     *
     * */
    char& back() {
        return *(m_end - 1);
    }

    /*! @brief Access first element
     *
     * */
    const char& back() const {
        return *(m_end - 1);
    }

    /*! @brief Returns a pointer to the first character of a string
     *
     * */
    char* data() {
        return m_begin;
    }

    /*! @brief Returns a pointer to the first character of a string
     *
     * */
    const char* data() const {
        return m_begin;
    }

    /*! @brief Returns an iterator to the beginning
     *
     * */
    char* begin() {
        return m_begin;
    }

    /*! @brief Returns an iterator to the beginning
     *
     * */
    const char* begin() const {
        return m_begin;
    }

    /*! @brief Returns an iterator to the beginning
     *
     * */
    const char* cbegin() const {
        return m_begin;
    }

    /*! @brief Returns an iterator to the end
     *
     * */
    char* end() {
        return m_end;
    }

    /*! @brief Returns an iterator to the end
     *
     * */
    const char* end() const {
        return m_end;
    }

    /*! @brief Returns an iterator to the end
     *
     * */
    const char* cend() const {
        return m_end;
    }

    /*! @brief Removes all characters
     * */
    void clear();

    /*! @brief String size without null-terminator
     *
     * @return Return number of characters in string
     * */
    std::size_t size() const {
        return std::size_t(m_end - m_begin);
    }

    /*! @brief String size without null-terminator
     *
     * @return Return number of characters in string
     * */
    std::size_t length() const {
        return std::size_t(m_end - m_begin);
    }

    /*! @brief Check if string is empty
     *
     * @return true when string is empty
     * */
    bool empty() const {
        return m_end == m_begin;
    }

    ~String() {
        delete [] m_begin;
    }
private:
    char* m_begin;
    char* m_end;
};

}

#endif /* JSON_CXX_STRING_HPP */
