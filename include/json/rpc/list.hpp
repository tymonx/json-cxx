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
 * @file json/rpc/list.hpp
 *
 * @brief JSON double-linked list interface
 * */

#ifndef JSON_CXX_RPC_LIST_HPP
#define JSON_CXX_RPC_LIST_HPP

#include <utility>
#include <iterator>

namespace json {
namespace rpc {

class ListItem {
public:
    ListItem() : mp_prev{nullptr}, mp_next{nullptr} { }
    ListItem(const ListItem&) = delete;
    ListItem(ListItem&&) = delete;
private:
    ListItem* mp_prev;
    ListItem* mp_next;
    friend class List;
};

class List {
public:
    template<bool is_const = false>
    class base_iterator {
    public:
        friend base_iterator<true>;

        /*! C++11 workaround for missing C++14 standard */
        template<bool B, class T, class F>
        using conditional_t = typename std::conditional<B, T, F>::type;

        /*! Iterator type */
        using value_type = ListItem;

        /*! Iterator pointer type */
        using pointer = conditional_t<is_const, const value_type*, value_type*>;

        /*! Iterator reference*/
        using reference = conditional_t<is_const, const value_type&, value_type&>;

        /*! Iterator category */
        using iterator_category = std::bidirectional_iterator_tag;

        using difference_type = std::ptrdiff_t;

        base_iterator() = default;
        base_iterator(const base_iterator& it) = default;
        base_iterator(base_iterator&& it) = default;
        base_iterator(pointer pit) : mp_it(pit) { }

        template<typename = typename std::enable_if<is_const>>
        base_iterator(const base_iterator<>& other) : mp_it(other.mp_it) { }

        base_iterator& operator++() {
            mp_it = mp_it->mp_next;
            return *this;
        }
        base_iterator operator++(int) {
            base_iterator it(mp_it);
            mp_it = mp_it->mp_next;
            return it;
        }
        base_iterator& operator--() {
            mp_it = mp_it->mp_prev;
            return *this;
        }
        base_iterator operator--(int) {
            base_iterator it(mp_it);
            mp_it = mp_it->mp_prev;
            return it;
        }

        bool operator!=(const base_iterator& other) { return mp_it != other.mp_it; }
        bool operator==(const base_iterator& other) { return mp_it == other.mp_it; }
        reference operator*() { return *mp_it; }
        pointer operator->() { return mp_it; }
    private:
        pointer mp_it;
    };

    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;

    /*!
     * @brief Check if list is empty
     *
     * @return true when list is empty otherwise return false
     * */
    bool empty() const { return nullptr == mp_first; }

    void push(ListItem* pitem);

    ListItem* pop() { return remove(mp_first); }

    ListItem* remove(ListItem* pitem);

    void splice(List& list);

    void clear() {
        mp_first = nullptr;
        mp_last = nullptr;
    }

    iterator begin() { return mp_first; }
    iterator end() { return nullptr; }
    const_iterator cbegin() const { return mp_first; }
    const_iterator cend() const { return nullptr; }

    ListItem* front() { return mp_first; }
    ListItem* back() { return mp_last; }
    const ListItem* front() const { return mp_first; }
    const ListItem* back() const { return mp_last; }
private:
    ListItem* mp_first = nullptr;
    ListItem* mp_last = nullptr;
};

}
}

#endif /* JSON_CXX_RPC_LIST_HPP */
