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

using json::allocator::Pool;

struct Header;

struct Header {
    Header* prev;
    Header* next;
    Header* end;
};

static constexpr std::uintptr_t MAX_ALIGN_SIZE = alignof(json::Value);
static constexpr std::uintptr_t MAX_ALIGN_OFFSET = MAX_ALIGN_SIZE - 1;
static constexpr std::uintptr_t MAX_ALIGN_MASK = ~MAX_ALIGN_OFFSET;

template <typename T>
static inline Header* header_cast(T* p) noexcept {
    return static_cast<Header*>(p);
}

static inline std::uintptr_t max_align(std::uintptr_t ptr) noexcept {
    return (ptr + MAX_ALIGN_OFFSET) & MAX_ALIGN_MASK;
}

static inline std::uintptr_t align(std::uintptr_t ptr) noexcept {
    return max_align(ptr + sizeof(Header)) - sizeof(Header);
}

template<typename T, typename K>
static inline T* align(K* ptr) noexcept {
    return reinterpret_cast<T*>(align(std::uintptr_t(ptr)));
}

template<typename T>
static inline T* align(std::uintptr_t ptr) noexcept {
    return reinterpret_cast<T*>(align(ptr));
}

#include <iostream>

Pool::Pool(void* memory, Size max_size) :
    m_memory{memory}
{
    if (m_memory) {
        m_begin = align<void>(m_memory);
        m_end = reinterpret_cast<void*>(
            (std::uintptr_t(m_memory) + max_size) & MAX_ALIGN_MASK
        );
        header_cast(m_begin)->prev = nullptr;
        header_cast(m_begin)->next = header_cast(m_end);
        header_cast(m_begin)->end = header_cast(m_begin) + 1;
        m_last = m_begin;
    }
}

void* Pool::allocate(Size size) {
    if ((0 == size) || !m_begin) { return nullptr; }

    void* ptr = nullptr;

    lock();
    auto pos = header_cast(m_last);
    while (pos >= header_cast(m_begin)) {
        ptr = pos->end + 1;
        auto end = align<Header>(std::uintptr_t(ptr) + size);
        if (end <= pos->next) {
            auto tmp = pos->next;
            pos->next = pos->end;
            pos->next->prev = pos;
            pos->next->next = tmp;
            pos->next->end = end;
            if (pos->next > header_cast(m_last)) {
                m_last = pos->next;
            }
            if (tmp < header_cast(m_end)) {
                tmp->prev = pos->next;
            }
            break;
        }
        else {
            ptr = nullptr;
            pos = pos->prev;
        }
    }
    unlock();

    std::cout << "Allocate pos: " << ptr << std::endl;

    return ptr;
}

void Pool::deallocate(void* ptr) noexcept {
    auto pos = header_cast(ptr) - 1;

    std::cout << "Deallocate pos: " << ptr << std::endl;
    if ((pos > header_cast(m_begin)) && (pos < header_cast(m_end))) {
        std::cout << "Deallocate pos2: " << ptr << std::endl;
        lock();
        pos->prev->next = pos->next;
        if (pos->next < header_cast(m_end)) {
            pos->next->prev = pos->prev;
        }
        if (pos == header_cast(m_last)) {
            m_last = pos->prev;
        }
        unlock();
    }
}

void Pool::lock() noexcept { }

void Pool::unlock() noexcept { }

Pool::~Pool() { }
