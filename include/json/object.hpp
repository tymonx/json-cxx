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
#include <json/allocator.hpp>

#include <iterator>
#include <type_traits>

namespace json {

class Pair;

class Object {
public:
    friend class Parser;

    template<typename T>
    class base_iterator;

    using iterator = base_iterator<Pair>;

    using const_iterator = base_iterator<const Pair>;

    using reverse_iterator = std::reverse_iterator<iterator>;

    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using difference_type = Difference;

    using size_type = Size;

    Object(Allocator* allocator = get_default_allocator()) :
        m_allocator{allocator}
    { }

    Object(Size size, Allocator* allocator = get_default_allocator());

    Object(const Object&, Allocator* allocator = get_default_allocator());

    Object(Object&& other, Allocator* allocator = get_default_allocator()) :
        m_begin{other.m_begin},
        m_end{other.m_end},
        m_allocator{allocator}
    {
        other.m_end = other.m_begin = nullptr;
    }

    Object& operator=(const Object& other) {
        return *this = Object(other);
    }

    Object& operator=(Object&&);

    template<typename T>
    Pair& operator[] (const T& pos) {
        return m_begin[pos];
    }

    template<typename T>
    const Pair& operator[] (const T& pos) const {
        return m_begin[pos];
    }

    Size size() const {
        return Size(m_end - m_begin);
    }

    Bool empty() const {
        return m_end == m_begin;
    }

    ~Object();

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
    Allocator* m_allocator{get_default_allocator()};
};

}

#endif /* JSON_CXX_OBJECT_HPP */
