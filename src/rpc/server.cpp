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

Server::Server() { }

Server::~Server() { }

void Server::add_command(const std::string& name, const Value& params,
        const Notification& notification) {
    using std::placeholders::_1;
    add_command(name, params, MethodId(std::bind(notification, _1)));
}

void Server::add_command(const std::string& name, const Value& params,
        const Method& method) {
    using std::placeholders::_1;
    using std::placeholders::_2;
    add_command(name, params, MethodId(std::bind(method, _1, _2)));
}

bool Server::equal_params(const Value& cmd_params, const Value& rsp_params) {
    bool valid;

    if (cmd_params.empty() && rsp_params.empty()) {
        valid = true;
    }
    else if (cmd_params.get_type() != rsp_params.get_type()) {
        valid = false;
    }
    else if (cmd_params.size() != rsp_params.size()) {
        valid = false;
    }
    else if (Value::Type::ARRAY == cmd_params.get_type()) {
        valid = true;
        for (size_t idx = 0; idx < cmd_params.size(); ++idx) {
            if (cmd_params[idx].get_type() != rsp_params[idx].get_type()) {
                valid = false;
                break;
            }
        }
    }
    else if (Value::Type::OBJECT == cmd_params.get_type()) {
        valid = true;
        for (auto it = cmd_params.cbegin(); it != cmd_params.cend(); ++it) {
            if (!rsp_params.is_member(it.key())) {
                valid = false;
                break;
            }
            else if (rsp_params[it.key()].get_type() != it->get_type()) {
                valid = false;
                break;
            }
        }
    }
    else {
        valid = false;
    }

    return valid;
}

void Server::add_command(const std::string& name, const Value& params,
        const MethodId& method_id)
{
    if (nullptr == method_id) { return; }

    CommandMapEntry command{nullptr, method_id};
    command.callback = method_id;
    if (params.is_object() || params.is_array()) {
        command.params = params;
    }
    else if (!params.is_null()) {
        command.params.push_back(params);
    }

    auto it_range = m_commands.equal_range(name);
    if (std::none_of(it_range.first, it_range.second,
        [&command, this] (CommandsMap::const_reference& cmd) {
            return !equal_params(cmd.second.params, command.params);
        }
    )) {
        m_commands.emplace(name, command);
    }
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
        result["data"] = error.get_data();
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

    auto it_range = m_commands.equal_range(vrequest["method"].as_string());
    if (it_range.first == it_range.second) {
        response << create_error({Error::METHOD_NOT_FOUND}, id);
        return;
    }

    auto it = std::find_if(it_range.first, it_range.second,
        [&vrequest, this] (CommandsMap::const_reference& cmd) {
            return equal_params(cmd.second.params, vrequest["params"]);
        }
    );
    if (it == it_range.second) {
        if (id_present) {
            response << create_error({Error::INVALID_PARAMS}, id);
        }
        return;
    }

    try {
        it->second.callback(vrequest["params"], vresponse, id);
        if (id_present) {
            response << create_response(vresponse, id);
        }
    }
    catch (const Error& error) {
        if (id_present) {
            response << create_error(error, id);
        }
    }
    catch (const std::exception& e) {
        if (id_present) {
            response << create_error({-1, e.what()}, id);
        }
    }
    catch (...) {
        if (id_present) {
            response << create_error({-1}, id);
        }
    }
}
