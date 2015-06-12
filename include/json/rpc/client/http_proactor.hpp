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
 * @file json/rpc/client/proactor.hpp
 *
 * @brief JSON client reactor interface
 * */

#ifndef JSON_CXX_RPC_CLIENT_HTTP_PROACTOR_HPP
#define JSON_CXX_RPC_CLIENT_HTTP_PROACTOR_HPP

#include <json/rpc/client/proactor.hpp>

#include <atomic>
#include <thread>
#include <memory>
#include <unistd.h>

namespace json {
namespace rpc {
namespace client {

class HttpProactor : public Proactor {
public:
    static Proactor& get_instance() {
        static HttpProactor proactor{};
        return proactor;
    }

    HttpProactor();

    virtual ~HttpProactor() final;

    virtual void notify() final;
private:
    static void curl_multi_deleter(void*);

    using CurlMultiPtr = std::unique_ptr<void, decltype(curl_multi_deleter)>;

    CurlMultiPtr m_curl_multi{nullptr, curl_multi_deleter};

    void task();

    volatile std::atomic<bool> m_task_done{false};
    std::thread m_thread{};

    fd_set m_fdread{};
    fd_set m_fdwrite{};
    fd_set m_fdexcep{};
    int m_maxfd{-1};

    uint64_t m_event{0};
    int m_eventfd{0};
};

}
}
}

#endif /* JSON_CXX_RPC_CLIENT_HTTP_PROACTOR_HPP */
