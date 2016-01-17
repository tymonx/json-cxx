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

#include <iterator>
#include <type_traits>
#include <initializer_list>

namespace json {

class String {
public:
    friend class Parser;

    template<typename T>
    class base_iterator;

    using iterator = base_iterator<Char>;

    using const_iterator = base_iterator<const Char>;

    using reverse_iterator = std::reverse_iterator<iterator>;

    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using difference_type = Difference;

    using size_type = Size;

    static const constexpr Size npos{Size(-1)};

    String();

    String(Size count, Char ch);

    String(const String& other, Size pos, Size count = npos);

    String(const Char* str, Size count);

    String(const Char* str);

    String(const_iterator first, const_iterator last) :
        String(first.base(), Size(last - first))
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
        return *this = other;
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

    String& assign(const_iterator first, const_iterator last) {
        return *this = String(first, last);
    }

    String& assign(std::initializer_list<Char> init) {
        return *this = String(init);
    }

    void swap(String& other);

    /*! @brief Access specified Character
     *
     * */
    template<typename T>
    Char& at(const T& pos) {
        return m_begin[pos];
    }

    /*! @brief Access specified Character
     *
     * */
    template<typename T>
    const Char& at(const T& pos) const {
        return m_begin[pos];
    }

    /*! @brief Access specified Character
     *
     * */
    template<typename T>
    Char& operator[](const T& pos) {
        return m_begin[pos];
    }

    /*! @brief Access specified Character
     *
     * */
    template<typename T>
    const Char& operator[](const T& pos) const {
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
        return m_begin.base();
    }

    /*! @brief Returns a pointer to the first Character of a string
     *
     * */
    const Char* data() const {
        return m_begin.base();
    }

    /*! @brief Returns an iterator to the beginning
     *
     * */
    iterator begin() {
        return m_begin;
    }

    /*! @brief Returns an iterator to the beginning
     *
     * */
    const_iterator begin() const {
        return m_begin.base();
    }

    /*! @brief Returns an iterator to the beginning
     *
     * */
    const_iterator cbegin() const {
        return m_begin.base();
    }

    /*! @brief Returns an iterator to the end
     *
     * */
    iterator end() {
        return m_end;
    }

    /*! @brief Returns an iterator to the end
     *
     * */
    const_iterator end() const {
        return m_end.base();
    }

    /*! @brief Returns an iterator to the end
     *
     * */
    const_iterator cend() const {
        return m_end.base();
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
    Size length() const { return size(); }

    /*! @brief Check if string is empty
     *
     * @return true when string is empty
     * */
    Bool empty() const {
        return m_end == m_begin;
    }

    operator char*() { return m_begin.base(); }

    operator char*() const { return m_begin.base(); }

    ~String();

    template<typename T>
    class base_iterator : public
        std::iterator<std::random_access_iterator_tag, T> {
    public:
        using value_type = T;

        using difference_type = Difference;

        using pointer = value_type*;

        using reference = value_type&;

        using iterator_category = std::random_access_iterator_tag;

        using removed_const = typename std::remove_const<T>::type;

        static constexpr bool is_const = std::is_const<T>::value;

        base_iterator() { }
        base_iterator(pointer p) : m_ptr{p} { }
        base_iterator(const base_iterator&) = default;
        base_iterator(base_iterator&&) = default;
        base_iterator& operator=(const base_iterator&) = default;
        base_iterator& operator=(base_iterator&&) = default;

        template<typename = typename std::enable_if<is_const>>
        base_iterator(const base_iterator<removed_const>& other) :
            m_ptr{other.base()} { }

        template<typename K>
        reference operator[](const K& pos) const {
            return *(m_ptr + pos);
        }

        template<typename K>
        reference operator+=(const K& pos) {
            m_ptr += pos;
            return *this;
        }

        template<typename K>
        reference operator-=(const K& pos) {
            m_ptr -= pos;
            return *this;
        }

        template<typename K>
        base_iterator operator+(const K& pos) const {
            return pointer(m_ptr + pos);
        }

        template<typename K>
        base_iterator operator-(const K& pos) const {
            return pointer(m_ptr - pos);
        }

        difference_type operator-(const base_iterator& other) const {
            return difference_type(m_ptr - other.m_ptr);
        }

        base_iterator& operator++() {
            ++m_ptr;
            return *this;
        }

        base_iterator& operator--() {
            --m_ptr;
            return *this;
        }

        base_iterator operator++(int) {
            base_iterator tmp(m_ptr);
            ++m_ptr;
            return tmp;
        }

        base_iterator operator--(int) {
            base_iterator tmp(m_ptr);
            --m_ptr;
            return tmp;
        }

        reference operator*() const { return *m_ptr; }

        pointer operator->() const { return m_ptr ; }

        pointer base() const { return m_ptr; }

        bool operator!() const { return nullptr == m_ptr; }

        operator pointer() { return m_ptr; }

        operator pointer() const { return m_ptr; }

        void swap(base_iterator& other) {
            base_iterator tmp(*this);
            *this = other;
            other = tmp;
        }

        bool operator==(const base_iterator& other) const {
            return m_ptr == other.m_ptr;
        }

        bool operator!=(const base_iterator& other) const {
            return m_ptr != other.m_ptr;
        }

        bool operator<(const base_iterator& other) const {
            return m_ptr < other.m_ptr;
        }

        bool operator<=(const base_iterator& other) const {
            return m_ptr <= other.m_ptr;
        }

        bool operator>(const base_iterator& other) const {
            return m_ptr > other.m_ptr;
        }

        bool operator>=(const base_iterator& other) const {
            return m_ptr >= other.m_ptr;
        }
    private:
        pointer m_ptr{nullptr};
    };

private:
    iterator m_begin{nullptr};
    iterator m_end{nullptr};
};

}

#endif /* JSON_CXX_STRING_HPP */
