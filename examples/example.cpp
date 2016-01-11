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
 * */

#include <json/json.hpp>

#include <functional>
#include <iostream>
#include <iomanip>
#include <limits>
#include <array>

using json::Value;
using json::Pair;
using json::Serializer;
using json::Deserializer;
using json::formatter::Pretty;

using std::cout;
using std::endl;

static const std::array<std::function<void()>, 7> g_examples{{
    [] {
        Value value("Test");
        cout << "String value: " << std::string(value) << endl;
        cout << "String type: " << (value.get_type() == Value::STRING) << endl;
    },
    [] {
        Value value(true);
        cout << "Bool value: " << bool(value) << endl;
        cout << "Bool type: " << (value.get_type() == Value::BOOLEAN) << endl;
    },
    [] {
        Value value;
        cout << "Null value: " << (value == nullptr) << endl;
        cout << "Null type: " << (value.get_type() == Value::NIL) << endl;
    },
    [] {
        Value value(13);
        cout << "Number value: " << int(value) << endl;
        cout << "Number type: " << (value.get_type() == Value::NUMBER) << endl;
    },
    [] {
        Value value;

        value.push_back(nullptr);
        value.push_back(6);
        value.push_back("Hello");
        value.push_back(value);

        cout << "Array value: " << value << endl;
    },
    [] {
        Value value;

        value.push_back(nullptr);
        value.push_back(6);
        value.push_back("Hello");
        value.push_back(value);

        Pretty pretty;
        cout << "Array value: " << Serializer{value, &pretty} << endl;
    },
    [] {
        Value value;

        R"({
            "number": 5,
            "string": "hello",
            "boolean": true,
            "double": 123456789.87654321e-06,
            "null": null,
            "array": [1, 2, 3],
            "value": {
                "member1": 1,
                "member2": 2
            }
        })" >> value;

        cout << "JSON document: " << value << endl;
        cout << "number: " << value["number"].as_uint() << endl;
        cout << "string: " << value["string"].as_string() << endl;
        cout << "boolean: " << value["boolean"].as_bool() << endl;
        cout << "double: "
            << std::setprecision(std::numeric_limits<double>::max_exponent10)
            << value["double"].as_double() << endl;
        cout << "null: " << value["null"].is_null() << endl;
        cout << "array:";
        for (const auto& v : value["array"]) {
            cout << " " << std::to_string(v.as_uint());
        }
        cout << endl;
        cout << "value:";
        for (auto it = value["value"].cbegin(); it != value["value"].cend(); ++it) {
            cout << " {" << it.key() << ", " << it->as_uint() << "}";
        }
        cout << endl;
        cout << "value (alt):";
        for (const auto& o : json::Object(value["value"])) {
            cout << " {" << o.first << ", " << o.second.as_uint() << "}";
        }
        cout << endl;
        cout << "value.member1: " << value["value"]["member1"].as_uint() << endl;
        cout << "value.member2: " << value["value"]["member2"].as_uint() << endl;
    }
}};

int main(void) {
    int index = 0;

    for (const auto& example : g_examples) {
        cout << endl << "Executing example " << std::to_string(++index) << "..."  << endl;
        example();
    }

    return 0;
}
