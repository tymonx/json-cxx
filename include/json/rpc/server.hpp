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
 * @file json/rpc/server.hpp
 *
 * @brief JSON RPC server interface
 * */

#ifndef JSON_CXX_RPC_SERVER_HPP
#define JSON_CXX_RPC_SERVER_HPP

#include <json/json.hpp>
#include <json/rpc/error.hpp>

#include <map>
#include <array>
#include <tuple>
#include <vector>
#include <functional>

namespace json {
namespace rpc {

/*!
 * JSON Client class
 * */
class Server {
public:
    using Notification = std::function<void(const Value&)>;
    using Method = std::function<void(const Value&, Value&)>;
    using MethodId = std::function<void(const Value&, Value&, const Value&)>;

    Server();

    virtual ~Server();

    virtual void start() = 0;

    virtual void stop() = 0;

    void add_command(const std::string& name, const Value& params,
            const Notification& notification);

    void add_command(const std::string& name, const Value& params,
            const Method& method);

    void add_command(const std::string& name, const Value& params,
            const MethodId& method_id);

    template<class T>
    struct CommandEntry {
        std::string name;
        Value params;
        T callback;
    };

    template<class T>
    using CommandsVector = std::vector<CommandEntry<T>>;

    using MethodsVector = CommandsVector<Method>;
    using MethodsIdVector = CommandsVector<MethodId>;
    using NotitificationsVector = CommandsVector<Notification>;

    template<class T, size_t N>
    using CommandsArray = std::array<CommandEntry<T>, N>;

    template<size_t N>
    using MethodsArray = CommandsArray<Method, N>;
    template<size_t N>
    using MethodsIdArray = CommandsArray<MethodId, N>;
    template<size_t N>
    using NotitificationsArray = CommandsArray<Notification, N>;

    template<class T>
    void add_command(const T& commands) {
        for (const auto& command : commands) {
            add_command(command.name, command.params, command.callback);
        }
    }

protected:
    void execute(const std::string& request, std::string& response);
private:
    struct CommandMapEntry {
        Value params;
        MethodId callback;
    };

    using CommandsMap = std::multimap<std::string, CommandMapEntry>;

    bool equal_params(const Value&, const Value&);
    bool valid_request(const Value& value);
    Value create_response(const Value&, const Value& id);
    Value create_error(const Error& error, const Value& id);

    CommandsMap m_commands{};
};

}
}

#endif /* JSON_CXX_RPC_SERVER_HPP */
