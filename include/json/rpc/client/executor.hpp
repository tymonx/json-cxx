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
 * @file json/rpc/client/executor.hpp
 *
 * @brief JSON responses executor for client. Threads pool implementation
 * */

#ifndef JSON_CXX_RPC_CLIENT_EXECUTOR_HPP
#define JSON_CXX_RPC_CLIENT_EXECUTOR_HPP

#include <json/rpc/error.hpp>
#include <json/rpc/client/message.hpp>
#include <json/rpc/client.hpp>

#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

namespace json {
namespace rpc {
namespace client {

class Executor {
public:
    using ErrorToException = Client::ErrorToException;

    static const size_t DEFAULT_THREAD_POOL_SIZE = 8;

    Executor(size_t thread_pool_size = DEFAULT_THREAD_POOL_SIZE);

    void execute(MessagePtr&& message, const Error& error = {Error::OK});

    void resize(size_t size);

    ~Executor();
private:
    using ThreadPool = std::vector<std::thread>;
    using Message = std::pair<MessagePtr, Error>;
    using Messages = std::queue<Message>;

    void task();
    void message_processing(MessagePtr& message, Error& error);

    void call_method_sync(MessagePtr& message, Error& error);
    void call_method_async(MessagePtr& message, Error& error);
    void send_notification_sync(MessagePtr& message, Error& error);
    void send_notification_async(MessagePtr& message, Error& error);
    void set_error_to_exception(MessagePtr& message, Error& error);

    ErrorToException m_error_to_exception{nullptr};
    std::mutex m_mutex{};
    std::condition_variable m_cond_variable{};
    ThreadPool m_thread_pool{};
    Messages m_messages{};
    volatile std::atomic_bool m_stop{false};
};

} /* client */
} /* rpc */
} /* json */

#endif /* JSON_CXX_RPC_CLIENT_EXECUTOR_HPP */
