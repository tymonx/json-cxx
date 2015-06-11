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
    ListItem() : m_prev{nullptr}, m_next{nullptr} { }
    virtual ~ListItem();
private:
    ListItem(const ListItem&) = delete;
    ListItem(ListItem&&) = delete;
    ListItem& operator=(const ListItem&) = delete;
    ListItem& operator=(ListItem&&) = delete;

    ListItem* m_prev{nullptr};
    ListItem* m_next{nullptr};
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
        base_iterator(pointer it) : m_it(it) { }

        template<typename = typename std::enable_if<is_const>>
        base_iterator(const base_iterator<>& other) : m_it(other.m_it) { }

        base_iterator& operator++() {
            m_it = m_it->m_next;
            return *this;
        }
        base_iterator operator++(int) {
            base_iterator it(m_it);
            m_it = m_it->m_next;
            return it;
        }
        base_iterator& operator--() {
            m_it = m_it->m_prev;
            return *this;
        }
        base_iterator operator--(int) {
            base_iterator it(m_it);
            m_it = m_it->m_prev;
            return it;
        }

        bool operator!=(const base_iterator& other) {
            return m_it != other.m_it;
        }
        bool operator==(const base_iterator& other) {
            return m_it == other.m_it;
        }
        reference operator*() { return *m_it; }
        pointer operator->() { return m_it; }
    private:
        pointer m_it;
    };

    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;

    List() { }

    List(List&& other) : m_first{other.m_first}, m_last{other.m_last} {
        other.m_first = nullptr;
        other.m_last = nullptr;
    }

    /*!
     * @brief Check if list is empty
     *
     * @return true when list is empty otherwise return false
     * */
    bool empty() const { return nullptr == m_first; }

    void push(ListItem* item);

    ListItem* pop() { return remove(m_first); }

    ListItem* remove(ListItem* item);

    void splice(List& list);

    void clear() {
        m_first = nullptr;
        m_last = nullptr;
    }

    iterator begin() { return m_first; }
    iterator end() { return nullptr; }
    const_iterator cbegin() const { return m_first; }
    const_iterator cend() const { return nullptr; }

    ListItem* front() { return m_first; }
    ListItem* back() { return m_last; }
    const ListItem* front() const { return m_first; }
    const ListItem* back() const { return m_last; }
private:
    List(const List&) = delete;
    List& operator=(const List&) = delete;

    ListItem* m_first{nullptr};
    ListItem* m_last{nullptr};
};

}
}

#endif /* JSON_CXX_RPC_LIST_HPP */
