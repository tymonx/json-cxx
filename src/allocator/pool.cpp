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
 * @file json/allocator/pool.cpp
 *
 * @brief JSON allocator pool interface
 * */

#include <json/allocator/pool.hpp>
#include <json/value.hpp>

#include <cstddef>
#include <cstdint>

using json::Size;
using json::allocator::Pool;

struct Header;

struct Header {
    Header* prev;
    Header* next;
    Header* end;
};

static constexpr std::size_t HEADER_ALIGN_SIZE = alignof(Header);
static constexpr std::size_t HEADER_ALIGN_OFFSET = HEADER_ALIGN_SIZE - 1;
static constexpr std::uintptr_t HEADER_ALIGN_MASK = ~HEADER_ALIGN_SIZE;

static constexpr std::size_t MAX_ALIGN_SIZE = alignof(std::max_align_t);
static constexpr std::size_t MAX_ALIGN_OFFSET = MAX_ALIGN_SIZE - 1;
static constexpr std::uintptr_t MAX_ALIGN_MASK = ~MAX_ALIGN_SIZE;

static inline
std::uintptr_t header_align(std::uintptr_t p) noexcept {
    return (p + HEADER_ALIGN_OFFSET) & HEADER_ALIGN_MASK;
}

static inline
std::uintptr_t max_align(std::uintptr_t p) noexcept {
    return (p + MAX_ALIGN_OFFSET) & MAX_ALIGN_MASK;
}

template<typename T, typename K>
static inline T* header_ptr(K* p) noexcept {
    return reinterpret_cast<T*>(max_align(
        header_align(std::uintptr_t(p)) + sizeof(Header)) - sizeof(Header));
}

template <typename T>
static inline Header* header_cast(T* p) noexcept {
    return static_cast<Header*>(p);
}

Pool::Pool(void* memory, Size max_size) :
    m_memory{memory}
{
    if (m_memory) {
        m_begin = header_ptr<void>(m_memory);
        m_end = reinterpret_cast<void*>(std::uintptr_t(m_memory) + max_size);
        if (m_begin != m_end) {
            header_cast(m_begin)->prev = nullptr;
            header_cast(m_begin)->next = header_cast(m_end);
            header_cast(m_begin)->end = header_cast(m_begin) + 1;
            m_last = m_begin;
        }
    }
}

void* Pool::allocate(Size size) {
    if ((0 == size) || !m_begin) { return nullptr; }

    void* ptr = nullptr;
    auto pos = header_cast(m_last);

    while (pos >= header_cast(m_begin)) {
        ptr = pos->end + 1;
        auto end = header_ptr<Header>(
            reinterpret_cast<Header*>(std::uintptr_t(ptr) + size));
        if (end <= pos->next) {
            auto tmp = pos->next;
            pos->next = pos->end;
            pos->next->prev = pos;
            pos->next->next = tmp;
            pos->next->end = end;
            if (tmp < header_cast(m_end)) {
                tmp->prev = pos->next;
            }
            if (pos->next > header_cast(m_last)) {
                m_last = pos->next;
            }
            break;
        }
        else {
            ptr = nullptr;
            pos = pos->prev;
        }
    }

    return ptr;
}

void Pool::deallocate(void* ptr, Size) noexcept {
    auto pos = header_cast(ptr) - 1;

    if ((pos > header_cast(m_begin)) && (pos < header_cast(m_end))) {
        lock();
        if (pos == header_cast(m_last)) {
            m_last = pos->prev;
        }
        if (pos->prev) {
            pos->prev->next = pos->next;
        }
        if (pos->next) {
            pos->next->prev = pos->prev;
        }
        unlock();
    }
}

void Pool::lock() noexcept { }

void Pool::unlock() noexcept { }

Pool::~Pool() { }
