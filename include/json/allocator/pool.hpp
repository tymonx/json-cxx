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
 * @file json/allocator/pool.hpp
 *
 * @brief JSON allocator pool interface
 * */

#ifndef JSON_CXX_ALLOCATOR_POOL_HPP
#define JSON_CXX_ALLOCATOR_POOL_HPP

#include <json/types.hpp>
#include <json/allocator.hpp>

namespace json {
namespace allocator {

class Pool : public Allocator {
public:
    Pool(void* memory, Size max_size);

    template<typename T>
    Pool(T* memory, Size max_size) :
        Pool(static_cast<void*>(memory), max_size)
    { }

    virtual void lock() noexcept;

    virtual void unlock() noexcept;

    virtual void* allocate(Size n) override final;

    virtual void deallocate(void* ptr) noexcept override final;

    virtual ~Pool();
private:
    Pool(const Pool&) = delete;
    Pool(Pool&&) = delete;
    Pool& operator=(const Pool&) = delete;
    Pool& operator=(Pool&&) = delete;

    void* m_memory{nullptr};
    void* m_begin{nullptr};
    void* m_end{nullptr};
    void* m_last{nullptr};
};

}
}

#endif /* JSON_CXX_ALLOCATOR_POOL_HPP */
