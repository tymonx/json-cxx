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
 * @file json/rpc/server/microhttpd_server.cpp
 *
 * @brief HTTP JSON RPC server implementation based on Microhttpd library
 * */

#include <json/rpc/server/microhttpd_server.hpp>

#include <cstring>
#include <microhttpd.h>

using std::strcmp;
using json::rpc::server::MicrohttpdServer;
using StringUniquePtr = std::unique_ptr<std::string>;

MicrohttpdServer::MicrohttpdServer(Port port) : m_port{port} { }

MicrohttpdServer::~MicrohttpdServer() { }

void MicrohttpdServer::MicrohttpdDeleter::operator()(::MHD_Daemon* mhd) {
    if (nullptr != mhd) {
        MHD_stop_daemon(mhd);
    }
}

int MicrohttpdServer::send_response(struct MHD_Connection* connection,
        unsigned status, const std::string& message)
{
    struct MHD_Response* mhd_response = MHD_create_response_from_buffer(
            message.length(),
            const_cast<char*>(message.c_str()),
            MHD_RESPMEM_MUST_COPY);

    if (nullptr == mhd_response) {
        return MHD_NO;
    }

    MHD_add_response_header(mhd_response, MHD_HTTP_HEADER_CONTENT_TYPE,
        "application/json");

    /* Response to client */
    int ret = ::MHD_queue_response(connection, status, mhd_response);
    MHD_destroy_response(mhd_response);

    return ret;
}

int MicrohttpdServer::method_post(void* cls, struct MHD_Connection* connection,
        const char* url, const char* method, const char* version,
        const char* upload_data, size_t* upload_data_size, void** con_cls)
{
    (void)cls;
    (void)url;
    (void)method;
    (void)version;
    (void)connection;

    StringUniquePtr request{static_cast<std::string*>(*con_cls)};

    if (nullptr == request) {
        request.reset(new (std::nothrow) std::string);
        if (nullptr == request) {
            return MHD_NO;
        }
        *con_cls = request.release();
        return MHD_YES;
    }

    if (0 != *upload_data_size) {
        request->append(upload_data, *upload_data_size);
        *upload_data_size = 0;
        request.release();
        return MHD_YES;
    }

    *con_cls = nullptr;
    std::string response{};

    static_cast<MicrohttpdServer*>(cls)->execute(*request, response);

    return send_response(connection, MHD_HTTP_OK, response);
}

int MicrohttpdServer::method_handler(void* cls, struct MHD_Connection *connection,
    const char* url, const char* method, const char* version,
    const char* upload_data, size_t* upload_data_size, void** con_cls) {

    if (0 == strcmp(method, "POST")) {
        return method_post(cls, connection, url, method, version,
                upload_data, upload_data_size, con_cls);
    }

    return send_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED,
            "Method not allowed");
}

void MicrohttpdServer::start() {
    m_mhd = MicrohttpdPtr{MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
            m_port, nullptr, nullptr,
            method_handler, this,
            MHD_OPTION_END)};
    if (nullptr == m_mhd) {
        throw std::runtime_error("Cannot start server");
    }
}

void MicrohttpdServer::stop() {
    m_mhd = nullptr;
}
