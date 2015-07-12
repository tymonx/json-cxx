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
 * @file json/rpc/server/http_settings.hpp
 *
 * @brief HTTP JSON server settings interface
 * */

#ifndef JSON_CXX_RPC_SERVER_HTTP_SETTINGS_HPP
#define JSON_CXX_RPC_SERVER_HTTP_SETTINGS_HPP

#include <json/rpc/time.hpp>

namespace json {
namespace rpc {
namespace server {

class HttpSettings {
public:
    using Miliseconds = time::Miliseconds;
    using Seconds = time::Seconds;
    using ThreadPoolSize = size_t;

    static const auto UNKNOWN_THREAD_POOL_SIZE = ThreadPoolSize(-1);

    static constexpr const auto UNKNOWN_TIMEOUT_MS = Miliseconds(-1);

    HttpSettings();

    HttpSettings(const HttpSettings&) = default;
    HttpSettings(HttpSettings&&) = default;
    HttpSettings& operator=(const HttpSettings&) = default;
    HttpSettings& operator=(HttpSettings&&) = default;

    ~HttpSettings();

    void set_thread_pool_size(const ThreadPoolSize& size) {
        m_thread_pool_size = size;
    }

    const ThreadPoolSize& get_thread_pool_size() const {
        return m_thread_pool_size;
    }

    void set_timeout(const Seconds& timeout_sec) {
        set_timeout(std::chrono::duration_cast<Miliseconds>(timeout_sec));
    }

    void set_timeout(const Miliseconds& timeout_ms) {
        m_timeout_ms = timeout_ms;
    }

    const Miliseconds& get_timeout() const { return m_timeout_ms; }
private:
    Miliseconds m_timeout_ms{UNKNOWN_TIMEOUT_MS};
    ThreadPoolSize m_thread_pool_size{UNKNOWN_THREAD_POOL_SIZE};
};

} /* server */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_SERVER_HTTP_SETTINGS_HPP */
