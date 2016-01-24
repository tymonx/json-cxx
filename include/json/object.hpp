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
 * @file json/object.hpp
 *
 * @brief JSON object interface
 * */

#ifndef JSON_CXX_OBJECT_HPP
#define JSON_CXX_OBJECT_HPP

#include <json/types.hpp>
#include <json/string.hpp>
#include <json/allocator.hpp>

#include <cstring>
#include <iterator>
#include <type_traits>
#include <initializer_list>

namespace json {

class Pair;
class Value;

/*! @brief JSON object class
 *
 * JSON object that stores JSON pairs that includes key string and JSON value
 * */
class Object {
public:
    friend class Parser;

    template<typename T>
    class base_iterator;

    /*!< Iterator */
    using iterator = base_iterator<Pair>;

    /*!< Const iterator */
    using const_iterator = base_iterator<const Pair>;

    /*!< Reverse iterator */
    using reverse_iterator = std::reverse_iterator<iterator>;

    /*!< Const reverse iterator */
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /*!< Difference type that can hold difference between pointers */
    using difference_type = Difference;

    /*!< Size type */
    using size_type = Size;

    /*! @brief Default constructor. Constructs an empty JSON object
     *
     * @param[in] allocator Allocator to use for all memory allocations of
     *                      this container and others
     * */
    Object(Allocator* allocator = Allocator::get_default()) :
        m_allocator{allocator}
    { }

    /*! @brief Constructs a JSON object with one JSON pair
     *
     * @param[in] key       String key for the pair
     * @param[in] value     JSON value for the pair
     * @param[in] allocator Allocator to use for all memory allocations of
     *                      this container and others
     * */
    Object(const String& key, const Value& value,
            Allocator* allocator = Allocator::get_default());

    /*! @brief Constructs a JSON object with one copied JSON pair
     *
     * @param[in] pair      JSON pair to copy
     * @param[in] allocator Allocator to use for all memory allocations of
     *                      this container and others
     * */
    Object(const Pair& pair,
            Allocator* allocator = Allocator::get_default());

    /*! @brief Constructs a JSON object with the contents of the range [first,
     * last)
     *
     * @param[in] first     First iterator
     * @param[in] last      Last iterator
     * @param[in] allocator Allocator to use for all memory allocations of
     *                      this container and others
     * */
    Object(const_iterator first, const_iterator last,
            Allocator* allocator = Allocator::get_default());

    /*! @brief Constructs a JSON object with the contents of the initializer
     * list
     *
     * @param[in] init      Initializer list ti initialize the elements
     * @param[in] allocator Allocator to use for all memory allocations of
     *                      this container and others
     * */
    Object(std::initializer_list<Pair> init,
            Allocator* allocator = Allocator::get_default());

    /*! @brief Copy constructor. Constructs a JSON object with the copy of
     * other. Use allocator from other
     *
     * @param[in] other     Another JSON object to be used as source to
     *                      initialize the elements of the JSON object with
     * */
    Object(const Object& other) :
        Object(other, other.m_allocator)
    { }

    /*! @brief Copy constructor. Constructs a JSON object with the copy of
     * other
     *
     * @param[in] other     Another JSON object to be used as source to
     *                      initialize the elements of the JSON object with
     * @param[in] allocator Allocator to use for all memory allocations of
     *                      this container and others
     * */
    Object(const Object& other, Allocator* allocator) :
        Object(other.cbegin(), other.cend(), allocator)
    { }

    /*! @brief Move constructor. Constructs a JSON object with the contents
     * of the other using move semantics. Use allocator from other
     *
     * @param[in] other     Another JSON object to be used as source to
     *                      initialize the elements of the JSON object with
     * */
    Object(Object&& other) :
        Object(std::move(other), other.m_allocator)
    { }

    /*! @brief Move constructor. Constructs a JSON object with the contents
     * of the other using move semantics
     *
     * @param[in] other     Another JSON object to be used as source to
     *                      initialize the elements of the JSON object with
     * @param[in] allocator Allocator to use for all memory allocations of
     *                      this container and others
     * */
    Object(Object&& other, Allocator* allocator) :
        m_begin{other.m_begin},
        m_end{other.m_end},
        m_allocator{allocator}
    {
        other.m_end = other.m_begin = nullptr;
    }

    Object& operator=(const Object& other) {
        return *this = Object(other, m_allocator);
    }

    Object& operator=(Object&&);

    Value& at(const Char* str, Size size);

    const Value& at(const Char* str, Size size) const;

    Value& at(const Char* str) {
        return at(str, std::strlen(str));
    }

    const Value& at(const Char* str) const {
        return at(str, std::strlen(str));
    }

    template<Size N>
    Value& at(const Char str[N]) {
        return at(str, N - 1);
    }

    template<Size N>
    const Value& at(const Char str[N]) const {
        return at(str, N - 1);
    }

    Value& at(const String& str) {
        return at(str, str.length());
    }

    const Value& at(const String& str) const {
        return at(str, str.length());
    }

    Value& operator[](const Char* str) {
        return at(str);
    }

    const Value& operator[](const Char* str) const {
        return at(str);
    }

    template<Size N>
    Value& operator[](const Char str[N]) {
        return at(str, N - 1);
    }

    template<Size N>
    const Value& operator[](const Char str[N]) const {
        return at(str, N - 1);
    }

    Value& operator[](const String& str) {
        return at(str);
    }

    const Value& operator[](const String& str) const {
        return at(str);
    }

    Pair& operator[] (int pos) {
        return m_begin[pos];
    }

    const Pair& operator[] (int pos) const {
        return m_begin[pos];
    }

    Pair& operator[] (Size pos) {
        return m_begin[pos];
    }

    const Pair& operator[] (Size pos) const {
        return m_begin[pos];
    }

    Size size() const {
        return Size(m_end - m_begin);
    }

    Bool empty() const {
        return m_end == m_begin;
    }

    Pair& front() {
        return *m_begin;
    }

    const Pair& front() const {
        return *m_begin;
    }

    Pair& back() {
        return *(m_end - 1);
    }

    const Pair& back() const {
        return *(m_end - 1);
    }

    Pair* data() {
        return m_begin;
    }

    const Pair* data() const {
        return m_begin;
    }

    void clear() noexcept;

    iterator erase(const_iterator pos) noexcept;

    iterator erase(const_iterator first, const_iterator last) noexcept;

    iterator begin() {
        return m_begin;
    }

    const_iterator begin() const {
        return m_begin;
    }

    const_iterator cbegin() const {
        return m_begin;
    }

    iterator end() {
        return m_end;
    }

    const_iterator end() const {
        return m_end;
    }

    const_iterator cend() const {
        return m_end;
    }

    reverse_iterator rbegin() {
        return reverse_iterator(m_end);
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(m_end);
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(m_end);
    }

    reverse_iterator rend() {
        return reverse_iterator(m_begin);
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(m_begin);
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(m_begin);
    }

    ~Object() noexcept;

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

        operator pointer() {
            return m_ptr;
        }

        operator const pointer() const {
            return m_ptr;
        }

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
    Allocator* m_allocator{Allocator::get_default()};
};

}

#endif /* JSON_CXX_OBJECT_HPP */
