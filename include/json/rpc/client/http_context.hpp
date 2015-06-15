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
 * @file json/rpc/client/message.hpp
 *
 * @brief JSON client message interface
 *
 * Message used for communication between clients and proactor
 * */

#ifndef JSON_CXX_RPC_CLIENT_HTTP_CONTEXT_HPP
#define JSON_CXX_RPC_CLIENT_HTTP_CONTEXT_HPP

#include <json/rpc/list.hpp>
#include <json/rpc/client/context.hpp>
#include <json/rpc/client/http_protocol.hpp>
#include <json/rpc/client/http_proactor.hpp>

#include <string>
#include <memory>
#include <vector>

/* Forward declaration */
struct curl_slist;

namespace json {
namespace rpc {
namespace client {

class HttpContext : public Context {
public:
    HttpContext(Client* client, HttpProactor& proactor,
            const HttpProtocol& protocol);

    virtual ~HttpContext() final;

    virtual void dispatch_event(Event* event) final;
private:
    struct CurlEasyDeleter {
        void operator()(void* curl_easy);
    };

    struct CurlSlistDeleter {
        void operator()(struct ::curl_slist* curl_slist);
    };

    using MessageId = std::uint16_t;
    using CurlEasyPtr = std::unique_ptr<void, CurlEasyDeleter>;
    using CurlSlistPtr = std::unique_ptr<struct ::curl_slist, CurlSlistDeleter>;

    struct pipeline {
        CurlEasyPtr curl_easy{nullptr};
        Event* event{nullptr};
        std::string::size_type request_pos{0};
        std::string::size_type response_pos{0};
        std::string request{};
        std::string response{};
    };

    static size_t write_function(char* buffer, size_t size, size_t nmemb,
            void* userdata);

    static size_t read_function(char* buffer, size_t size, size_t nmemb,
            void* userdata);

    bool build_message(Event* event, Value& message, unsigned id);
    bool event_to_message(Event* event);
    bool event_to_pipeline(Event* event, struct pipeline& pipe, unsigned id);

    using Pipelines = std::vector<struct pipeline>;

    MessageId m_message_id{0};
    CurlSlistPtr m_headers{nullptr};
    Pipelines m_pipelines{};
    HttpProactor& m_proactor;
    HttpProtocol m_protocol{};
    List m_events{};
};

} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_HTTP_CONTEXT_HPP */
