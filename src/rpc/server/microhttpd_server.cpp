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

#include <string>
#include <cstring>
#include <microhttpd.h>

using json::rpc::server::MicrohttpdServer;
using RequestPtr = std::unique_ptr<std::string>;

static const char METHOD_NOT_ALLOWED[] = "Method not allowed";

MicrohttpdServer::~MicrohttpdServer() { }

void MicrohttpdServer::MicrohttpdDeleter::operator()(::MHD_Daemon* mhd) {
    if (nullptr != mhd) { MHD_stop_daemon(mhd); }
}

void MicrohttpdServer::set_settings(const HttpSettings& settings) {
    if (HttpSettings::UNKNOWN_THREAD_POOL_SIZE
            != settings.get_thread_pool_size()) {
        m_thread_pool_size = settings.get_thread_pool_size();
    }

    if (HttpSettings::UNKNOWN_TIMEOUT_MS != settings.get_timeout()) {
        m_timeout_ms = settings.get_timeout();
    }
}

int MicrohttpdServer::method_post(void* cls, struct MHD_Connection* connection,
        const char* upload_data, size_t* upload_data_size, void** con_cls)
{
    RequestPtr request{static_cast<std::string*>(*con_cls)};

    if (nullptr == request) {
        request.reset(new (std::nothrow) std::string{});
        if (nullptr == request) { return MHD_NO; }
        *con_cls = request.release();
        return MHD_YES;
    }

    if (0 != *upload_data_size) {
        request->append(upload_data, *upload_data_size);
        *upload_data_size = 0;
        request.release();
        return MHD_YES;
    }

    std::string response{};
    static_cast<MicrohttpdServer*>(cls)->execute(*request, response);
    *con_cls = nullptr;

    struct MHD_Response* mhd_response = MHD_create_response_from_buffer(
            response.length(), const_cast<char*>(response.c_str()),
            MHD_RESPMEM_MUST_COPY);
    if (nullptr == mhd_response) { return MHD_NO; }

    MHD_add_response_header(mhd_response, MHD_HTTP_HEADER_CONTENT_TYPE,
        "application/json");

    /* Response to client */
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, mhd_response);
    MHD_destroy_response(mhd_response);

    return ret;
}

int MicrohttpdServer::method_handler(void* cls, struct MHD_Connection *connection,
    const char*, const char* method, const char*,
    const char* upload_data, size_t* upload_data_size, void** con_cls) {

    if (0 == std::strcmp(method, "POST")) {
        return method_post(cls, connection, upload_data,
                upload_data_size, con_cls);
    }

    struct MHD_Response* mhd_response = MHD_create_response_from_buffer(
            std::strlen(METHOD_NOT_ALLOWED),
            const_cast<char*>(METHOD_NOT_ALLOWED),
            MHD_RESPMEM_PERSISTENT);
    if (nullptr == mhd_response) { return MHD_NO; }

    /* Response to client */
    int ret = MHD_queue_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED,
            mhd_response);
    MHD_destroy_response(mhd_response);
    return ret;
}

void MicrohttpdServer::start() {
    unsigned int timeout_sec = 0;

    if (500 <= m_timeout_ms.count())  {
        timeout_sec = unsigned(m_timeout_ms.count() + 500) / 1000;
    }
    else {
        timeout_sec = 1;
    }

    if (0 == m_thread_pool_size) {
        m_mhd = MicrohttpdPtr{MHD_start_daemon(
            MHD_USE_THREAD_PER_CONNECTION | MHD_USE_POLL,
            m_port, nullptr, nullptr, method_handler, this,
            MHD_OPTION_CONNECTION_TIMEOUT, timeout_sec,
            MHD_OPTION_END
        )};
    }
    else {
        m_mhd = MicrohttpdPtr{MHD_start_daemon(
            MHD_USE_SELECT_INTERNALLY,
            m_port, nullptr, nullptr, method_handler, this,
            MHD_OPTION_CONNECTION_TIMEOUT, timeout_sec,
            MHD_OPTION_THREAD_POOL_SIZE, m_thread_pool_size,
            MHD_OPTION_END
        )};
    }
    if (nullptr == m_mhd) {
        throw std::runtime_error("Cannot start server");
    }
}

void MicrohttpdServer::stop() {
    m_mhd = nullptr;
}
