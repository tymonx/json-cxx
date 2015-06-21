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
 * @file json/rpc/client/call_method.cpp
 *
 * @brief JSON client protocol IPv4 protocol
 * */

#include <json/rpc/client/call_method.hpp>

using json::rpc::client::CallMethod;

CallMethod::~CallMethod() { }

void CallMethod::check_response(const Value& value) {
    if (!value.is_object()) {
        set_error({Error::PARSE_ERROR, "Invalid JSON object"});
    }
    else if (3 != value.size()) {
        set_error({Error::PARSE_ERROR, "Invalid params number"});
    }
    else if (value["jsonrpc"] != "2.0") {
        set_error({Error::PARSE_ERROR, "Invalid/missing 'jsonrpc'"});
    }
    else if (!value.is_member("id")) {
        set_error({Error::PARSE_ERROR, "Missing 'id'"});
    }
    else if (value.is_member("result")) {
        if (get_id() != value["id"]) {
            set_error({Error::PARSE_ERROR, "Invalid 'id'"});
        }
    }
    else if (value.is_member("error")) {
        auto& error = value["error"];
        if (!error.is_object()) {
            set_error({Error::PARSE_ERROR,
                    "Invalid JSON object 'error'"});
        }
        else if (!error["code"].is_int()) {
            set_error({Error::PARSE_ERROR,
                    "Invalid/missing 'code' in 'error'"});
        }
        else if (!error["message"].is_string()) {
            set_error({Error::PARSE_ERROR,
                    "Invalid/missing 'message' in 'error'"});
        }
        else if (!value["id"].is_null() && (get_id() != value["id"])) {
            set_error({Error::PARSE_ERROR,
                    "Invalid 'id'"});
        }
        else if (2 == error.size()) {
            /* Do nothing */
        }
        else if (3 == error.size()) {
            if (!error.is_member("data")) {
                set_error({Error::PARSE_ERROR,
                        "Missing 'data' member in 'error'"});
            }
        }
        else {
            set_error({Error::PARSE_ERROR,
                    "Invalid params number in 'error'"});
        }
    }
    else {
        set_error({Error::PARSE_ERROR, "Missing 'result' or 'error'"});
    }
}

void CallMethod::processing() {
    if (get_error()) { return; }

    Value value;
    Deserializer deserializer(get_response());
    if (deserializer.is_invalid()) {
        auto error = deserializer.get_error();
        return set_error({Error::PARSE_ERROR,
                "Invalid response: \'" + get_response() +
                "\' " + error.decode() + " at " +
                std::to_string(error.offset)});
    }
    deserializer >> value;
    check_response(value);
    if (get_error()) {
        return set_error({get_error().get_code(),
                "Invalid response: \'" + get_response() +
                "\' " + get_error().what()});
    }
    if (value.is_member("result")) {
        set_value(value["result"]);
    }
    else {
        set_error({
            value["error"]["code"].as_int(),
            value["error"]["message"].as_string(),
            value["error"]["data"]
        });
    }
}
