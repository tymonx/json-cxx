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
 * @file json/rpc/client/curl_context.hpp
 *
 * @brief CURL context for CURL proactor
 * */

#ifndef JSON_CXX_RPC_CLIENT_CURL_CONTEXT_HPP
#define JSON_CXX_RPC_CLIENT_CURL_CONTEXT_HPP

#include <json/json.hpp>
#include <json/rpc/client.hpp>
#include <json/rpc/client/message.hpp>
#include <json/rpc/client/http_settings.hpp>

#include <list>
#include <string>
#include <memory>
#include <vector>
#include <functional>

/* Forward declaration */
struct curl_slist;

namespace json {
namespace rpc {
namespace client {

/* Forward declaration */
class HttpClient;

class CurlContext {
public:
    CurlContext(HttpClient* client, void* curl_multi);

    ~CurlContext();

    HttpClient* get_client() { return m_client; }

    const HttpClient* get_client() const { return m_client; }

    void splice_message(MessageList& other, MessageList::const_iterator it) {
        m_messages.splice(m_messages.end(), other, it);
    }

    void dispatch_events();

    bool active() const {
        return !m_messages.empty() || m_pipes_active;
    }

    const Miliseconds& get_time_live() const { return m_time_live; }
private:
    CurlContext(const CurlContext&) = delete;
    CurlContext(CurlContext&&) = delete;

    CurlContext& operator=(const CurlContext&) = delete;
    CurlContext& operator=(CurlContext&&) = delete;

    struct CurlEasyDeleter {
        void operator()(void* curl_easy);
    };

    struct CurlSlistDeleter {
        void operator()(struct ::curl_slist* curl_slist);
    };

    struct InfoRead;

    using Id = unsigned;
    using CurlEasyPtr = std::unique_ptr<void, CurlEasyDeleter>;
    using CurlSlistPtr = std::unique_ptr<struct ::curl_slist, CurlSlistDeleter>;
    using InfoReadCallback = std::function<void(CurlContext*,
            struct InfoRead*, unsigned)>;

    struct InfoRead {
        CurlContext* context{nullptr};
        InfoReadCallback callback{};
        CurlEasyPtr curl_easy{nullptr};
    };

    struct Pipeline : public InfoRead {
        MessagePtr message{nullptr};
        std::string::size_type request_pos{};
        std::string request{};
        std::string response{};
    };

    using Pipelines = std::vector<Pipeline>;

    static size_t write_function(char* buffer, size_t size, size_t nmemb,
            void* userdata);

    static size_t read_function(char* buffer, size_t size, size_t nmemb,
            void* userdata);

    void handle_pipe(struct InfoRead*, unsigned curl_code);
    bool handle_event_timeout(MessageList::iterator& it);
    void handle_event_request(MessageList::iterator& it);
    //Value build_message(Request& request, Id id);

    HttpClient* m_client;
    void* m_curl_multi;
    CurlSlistPtr m_headers{nullptr};
    Pipelines::size_type m_pipes_active{0};
    Pipelines m_pipelines{};
    MessageList m_messages{};
    Client::IdBuilder m_id_builder{nullptr};
    Client::ErrorToException m_erro_to_exception{nullptr};
    Miliseconds m_time_live{0_ms};
};

using CurlContextPtr = std::unique_ptr<CurlContext>;
using CurlContextList = std::list<CurlContextPtr>;

} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_CURL_CONTEXT_HPP */
