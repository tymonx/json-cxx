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
 * @file json/rpc/server.cpp
 *
 * @brief JSON RPC server implementation
 * */

#include <json/rpc/server.hpp>

#include <algorithm>
#include <exception>

using namespace std;
using json::rpc::Server;

Server::~Server() { }

void Server::add_command(const std::string& name,
        const Notification& notification) {
    using std::placeholders::_1;
    add_command(name, MethodId(std::bind(notification, _1)));
}

void Server::add_command(const std::string& name, const Method& method) {
    using std::placeholders::_1;
    using std::placeholders::_2;
    add_command(name, MethodId(std::bind(method, _1, _2)));
}

void Server::add_command(const std::string& name, const MethodId& method_id) {
    if (nullptr == method_id) { return; }
    m_commands[name] = method_id;
}

bool Server::valid_request(const Value& value) {
    if (!value.is_object()) { return false; }
    if (value["jsonrpc"] != "2.0") { return false; }
    if (!value["method"].is_string()) { return false; }

    bool is_id = value.is_member("id");
    bool is_params = value.is_member("params");
    if (is_params) {
        if (!value["params"].is_array() && !value["params"].is_object()) {
            return false;
        }
    }

    if (4 == value.size()) {
        if (!is_params || !is_id) { return false; }
    }
    else if (3 == value.size()) {
        if (!is_params && !is_id) { return false; }
    }
    else if (2 != value.size()) { return false; }

    if (!value["id"].is_string() && !value["id"].is_number()
     && !value["id"].is_null()) {
        return false;
    }

    return true;
}

json::Value Server::create_error(const Error& error, const Value& id) {
    Value result;
    result["jsonrpc"] = "2.0";
    result["error"]["code"] = error.get_code();
    result["error"]["message"] = error.get_message();
    if (error.get_data() != nullptr) {
        result["error"]["data"] = error.get_data();
    }
    result["id"] = id;
    return result;
}

json::Value Server::create_response(const Value& params, const Value& id) {
    Value result;
    result["jsonrpc"] = "2.0";
    result["result"] = params;
    result["id"] = id;
    return result;
}

void Server::execute(const std::string& request, std::string& response) {
    Value vrequest;
    Value vresponse;
    Value id;
    bool id_present;

    Deserializer deserializer(request);
    if (deserializer.is_invalid()) {
        response << create_error({Error::PARSE_ERROR}, nullptr);
        return;
    }

    deserializer >> vrequest;
    if (!valid_request(vrequest)) {
        response << create_error({Error::INVALID_REQUEST}, nullptr);
        return;
    }
    id = vrequest["id"];
    id_present = vrequest.is_member("id");

    auto it = m_commands.find(vrequest["method"].as_string());
    if (it == m_commands.cend()) {
        response << create_error({Error::METHOD_NOT_FOUND}, id);
        return;
    }

    try {
        if (nullptr == m_method_handler) {
            it->second(vrequest["params"], vresponse, id);
        }
        else {
            m_method_handler(it->second, vrequest["params"], vresponse, id);
        }
        response << create_response(vresponse, id);
    }
    catch (const Error& error) {
        response << create_error(error, id);
    }
    catch (const std::exception& e) {
        response << create_error({-1, e.what()}, id);
    }
    catch (...) {
        response << create_error({-1}, id);
    }
    if (!id_present) { response.clear(); }
}
