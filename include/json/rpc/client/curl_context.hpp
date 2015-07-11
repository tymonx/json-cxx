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
#include <json/rpc/client/executor.hpp>
#include <json/rpc/client/http_settings.hpp>
#include <json/rpc/client/http_client.hpp>

#include <list>
#include <string>
#include <memory>
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
    struct InfoRead;

    struct CurlEasyDeleter {
        void operator()(void* curl_easy);
    };

    struct CurlSlistDeleter {
        void operator()(struct ::curl_slist* curl_slist);
    };

    using Id = std::uint16_t;
    using CurlEasyPtr = std::unique_ptr<void, CurlEasyDeleter>;
    using CurlSlistPtr = std::unique_ptr<struct ::curl_slist, CurlSlistDeleter>;
    using InfoReadPtr = std::unique_ptr<InfoRead>;
    using InfoReadCallback = void(CurlContext*, InfoReadPtr, unsigned);

    struct InfoRead {
        CurlContext* context{nullptr};
        InfoReadCallback* callback{nullptr};
        MessagePtr message{nullptr};
        std::string::size_type request_pos{};
        std::string request{};
        std::string* response{nullptr};
    };

    CurlContext(HttpClient* client, void* curl_multi);

    ~CurlContext();

    const HttpClient* get_client() const { return m_client; }

    void splice_message(MessageList& other, MessageList::const_iterator it);

    void dispatch_messages();

    bool active() const { return !m_messages.empty() || (m_requests > 0); }
private:
    CurlContext(const CurlContext&) = delete;
    CurlContext(CurlContext&&) = delete;
    CurlContext& operator=(const CurlContext&) = delete;
    CurlContext& operator=(CurlContext&&) = delete;

    static size_t write_function(char* buffer, size_t size, size_t nmemb,
            void* userdata);

    static size_t read_function(char* buffer, size_t size, size_t nmemb,
            void* userdata);

    static void info_read(CurlContext*, InfoReadPtr, unsigned curl_code);

    void call_method_sync(MessageList::iterator& it);
    void call_method_async(MessageList::iterator& it);
    void send_notification_sync(MessageList::iterator& it);
    void send_notification_async(MessageList::iterator& it);
    void connect(MessageList::iterator& it);
    void disconnect(MessageList::iterator& it);
    void set_error_to_exception(MessageList::iterator& it);
    void set_http_settings(MessageList::iterator& it);
    void set_id_builder(MessageList::iterator& it);

    bool message_expired(MessageList::iterator& it);
    void add_request(MessagePtr&& message, std::string&& request,
            std::string* response);

    HttpClient* m_client;
    void* m_curl_multi;
    CurlSlistPtr m_headers{nullptr};
    bool m_is_connected{false};
    Executor m_executor{};
    MessageList m_messages{};
    Client::IdBuilder m_id_builder{nullptr};
    time::Miliseconds m_time_live_ms{0_ms};
    time::Miliseconds m_timeout_ms{0_ms};
    std::string m_url{HttpClient::DEFAULT_URL};
    size_t m_requests{0};
    Id m_id{0};
};

using CurlContextPtr = std::unique_ptr<CurlContext>;
using CurlContextList = std::list<CurlContextPtr>;

} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_CURL_CONTEXT_HPP */
