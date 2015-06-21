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
 * @file json/rpc/server/microhttpd_server.hpp
 *
 * @brief JSON client interface
 * */

#ifndef JSON_CXX_RPC_MICROHTTPD_SERVER_HPP
#define JSON_CXX_RPC_MICROHTTPD_SERVER_HPP

#include <json/rpc/server.hpp>

#include <memory>

struct MHD_Daemon;
struct MHD_Connection;

namespace json {
namespace rpc {
namespace server {

/*!
 * JSON server class
 * */
class MicrohttpdServer : public json::rpc::Server {
public:
    using Port = std::uint16_t;

    static constexpr const Port DEFAULT_PORT = 80;

    MicrohttpdServer(Port port = DEFAULT_PORT);

    virtual ~MicrohttpdServer() final;

    virtual void start() final;

    virtual void stop() final;
private:
    struct MicrohttpdDeleter {
        void operator()(struct ::MHD_Daemon*);
    };

    using MicrohttpdPtr = std::unique_ptr<::MHD_Daemon, MicrohttpdDeleter>;

    Port m_port{};
    MicrohttpdPtr m_mhd{nullptr};

    static int send_response(struct ::MHD_Connection* connection,
            unsigned status, const std::string& message);

    static int method_handler(void* cls, struct ::MHD_Connection *connection,
        const char* url, const char* method, const char* version,
        const char* upload_data, size_t* upload_data_size, void** con_cls);

    static int method_post(void* cls, struct ::MHD_Connection* connection,
            const char* url, const char* method, const char* version,
            const char* upload_data, size_t* upload_data_size, void** con_cls);
};

}
}
}

#endif /* JSON_CXX_RPC_MICROHTTPD_SERVER_HPP */
