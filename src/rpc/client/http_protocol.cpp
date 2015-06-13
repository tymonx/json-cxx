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
 * @file json/rpc/client/http_protocol.cpp
 *
 * @brief JSON client protocol IPv4 protocol
 * */

#include <json/rpc/client/http_protocol.hpp>

using json::rpc::client::HttpProtocol;

constexpr const char HttpProtocol::DEFAULT_URL[];

constexpr const HttpProtocol::Miliseconds HttpProtocol::DEFAULT_TIMEOUT_MS;

HttpProtocol::HttpProtocol(const Url& url)
    : Protocol(ProtocolType::HTTP), m_url{url} { }

void HttpProtocol::set_pipeline_length(unsigned pipeline_length) {
    if (!pipeline_length) { m_pipeline_length = DEFAULT_PIPELINE_LENGTH; }
    else { m_pipeline_length = pipeline_length; }
}

void HttpProtocol::set_timeout(const Miliseconds& miliseconds) {
    if (0_ms == miliseconds) { m_timeout_ms = DEFAULT_TIMEOUT_MS; }
    else { m_timeout_ms = miliseconds; }
}

void HttpProtocol::add_header(const Header& header) {
    if (header.first.empty() || header.second.empty()) { return; }
    m_headers.emplace(header.first, header.second);
}
