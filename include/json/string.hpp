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

#include <json/types.hpp>

#include <cstring>
#include <limits>
#include <utility>
#include <initializer_list>

namespace json {

class String {
public:
    friend class Parser;

    static const constexpr Size npos =
        std::numeric_limits<Size>::max();

    String();

    String(Size count, Char ch);

    String(const String& other, Size pos, Size count = npos);

    String(const Char* str, Size count);

    String(const Char* str) :
        String(str, std::strlen(str))
    { }

    String(const Char* first, const Char* last) :
        String(first, Size(last - first))
    { }

    template<Size N>
    String(const Char str[N]) : String(str, N - 1) { }

    String(std::initializer_list<Char> init) :
        String(init.begin(), init.size())
    { }

    String(const String& other) :
        String(other.data(), other.size())
    { }

    String(String&& other) :
        m_begin{other.m_begin},
        m_end{other.m_end}
    {
        other.m_end = other.m_begin = nullptr;
    }

    String& operator=(String&& other);

    String& operator=(const String& other) {
        return *this = String(other.data(), other.size());
    }

    String& operator=(const Char* s) {
        return *this = String(s);
    }

    String& operator=(Char ch) {
        return *this = String(1, ch);
    }

    String& operator=(std::initializer_list<Char> init) {
        return *this = String(init);
    }

    String& assign(Size count, Char ch) {
        return *this = String(count, ch);
    }

    String& assign(const String& other) {
        return *this = other;
    }

    String& assign(String&& other) {
        return *this = std::move(other);
    }

    String& assign(const String& other, Size pos, Size count) {
        return *this = String(other, pos, count);
    }

    String& assign(const Char* s, Size count) {
        return *this = String(s, count);
    }

    String& assign(const Char* s) {
        return *this = String(s);
    }

    String& assign(const Char* first, const Char* last) {
        return *this = String(first, last);
    }

    String& assign(std::initializer_list<Char> init) {
        return *this = String(init);
    }

    void swap(String& other);

    /*! @brief Access specified Character
     *
     * */
    Char& at(Size pos) {
        return m_begin[pos];
    }

    /*! @brief Access specified Character
     *
     * */
    const Char& operator[](Size pos) const {
        return m_begin[pos];
    }

    /*! @brief Access first element
     *
     * */
    Char& front() {
        return *m_begin;
    }

    /*! @brief Access first element
     *
     * */
    const Char& front() const {
        return *m_begin;
    }

    /*! @brief Access first element
     *
     * */
    Char& back() {
        return *(m_end - 1);
    }

    /*! @brief Access first element
     *
     * */
    const Char& back() const {
        return *(m_end - 1);
    }

    /*! @brief Returns a pointer to the first Character of a string
     *
     * */
    Char* data() {
        return m_begin;
    }

    /*! @brief Returns a pointer to the first Character of a string
     *
     * */
    const Char* data() const {
        return m_begin;
    }

    /*! @brief Returns an iterator to the beginning
     *
     * */
    Char* begin() {
        return m_begin;
    }

    /*! @brief Returns an iterator to the beginning
     *
     * */
    const Char* begin() const {
        return m_begin;
    }

    /*! @brief Returns an iterator to the beginning
     *
     * */
    const Char* cbegin() const {
        return m_begin;
    }

    /*! @brief Returns an iterator to the end
     *
     * */
    Char* end() {
        return m_end;
    }

    /*! @brief Returns an iterator to the end
     *
     * */
    const Char* end() const {
        return m_end;
    }

    /*! @brief Returns an iterator to the end
     *
     * */
    const Char* cend() const {
        return m_end;
    }

    /*! @brief Removes all Characters
     * */
    void clear();

    /*! @brief String size without null-terminator
     *
     * @return Return number of Characters in string
     * */
    Size size() const {
        return Size(m_end - m_begin);
    }

    /*! @brief String size without null-terminator
     *
     * @return Return number of Characters in string
     * */
    Size length() const {
        return Size(m_end - m_begin);
    }

    /*! @brief Check if string is empty
     *
     * @return true when string is empty
     * */
    Bool empty() const {
        return m_end == m_begin;
    }

    ~String();
private:
    Char* m_begin{nullptr};
    Char* m_end{nullptr};
};

}

#endif /* JSON_CXX_STRING_HPP */
