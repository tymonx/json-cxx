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
 * @file json/rpc/client/http_settings.hpp
 *
 * @brief HTTP JSON client settings interface
 * */

#ifndef JSON_CXX_RPC_CLIENT_HTTP_SETTINGS_HPP
#define JSON_CXX_RPC_CLIENT_HTTP_SETTINGS_HPP

#include <json/rpc/time.hpp>

#include <string>
#include <chrono>
#include <utility>
#include <functional>
#include <unordered_map>

namespace json {
namespace rpc {
namespace client {

class HttpSettings {
public:
    using Header = std::pair<std::string, std::string>;
    using Headers = std::unordered_map<std::string, std::string>;
    using IdBuilder = std::function<std::string(unsigned)>;

    static constexpr const unsigned DEFAULT_PIPELINE_LENGTH = 8;

    static constexpr const Miliseconds DEFAULT_TIME_LIVE_MS = 0_ms;

    static constexpr const Miliseconds DEFAULT_TIMEOUT_MS = 1000_ms;

    HttpSettings() { }

    HttpSettings(const HttpSettings&) = default;
    HttpSettings(HttpSettings&&) = default;
    HttpSettings& operator=(const HttpSettings&) = default;
    HttpSettings& operator=(HttpSettings&&) = default;

    virtual ~HttpSettings();

    void set_pipeline_length(unsigned pipeline_length);

    unsigned get_pipeline_length() const { return m_pipeline_length; }

    void set_timeout(const Seconds& timeout_sec) {
        set_timeout(std::chrono::duration_cast<Miliseconds>(timeout_sec));
    }

    void set_timeout(const Miliseconds& timeout_ms) {
        m_time_timeout_ms = timeout_ms;
    }

    void set_time_live(const Seconds& live_sec) {
        set_time_live(std::chrono::duration_cast<Miliseconds>(live_sec));
    }

    void set_time_live(const Miliseconds& live_ms) {
        m_time_live_ms = live_ms;
    }

    const Miliseconds& get_time_live() const { return m_time_live_ms; }

    const Miliseconds& get_timeout() const { return m_time_timeout_ms; }

    void add_header(const Header& header);
    void remove_header(const Header& header) {
        m_headers.erase(header.first);
    }

    const Headers& get_headers() const { return m_headers; }

    void set_id_builder(IdBuilder id_builder) { m_id_builder = id_builder; }

    IdBuilder& get_id_builder() { return m_id_builder; }

    const IdBuilder& get_id_builder() const { return m_id_builder; }
private:
    unsigned m_pipeline_length{DEFAULT_PIPELINE_LENGTH};
    Miliseconds m_time_live_ms{DEFAULT_TIME_LIVE_MS};
    Miliseconds m_time_timeout_ms{DEFAULT_TIMEOUT_MS};
    Headers m_headers{};
    IdBuilder m_id_builder{nullptr};
};

} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_HTTP_SETTINGS_HPP */
