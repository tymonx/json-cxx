# Copyright (c) 2015, Tymoteusz Blazejczyk
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of json-cxx nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

add_library(json-cxx
    value.cpp
    number.cpp
    iterator.cpp
    serializer.cpp
    deserializer.cpp
    formatter.cpp
    formatter/compact.cpp
    formatter/pretty.cpp
    writter.cpp
    writter/counter.cpp
    writter/string.cpp
)

add_library(json-cxx-rpc-client
    rpc/error.cpp
    rpc/client.cpp
    rpc/client/curl_client.cpp
    rpc/client/curl_context.cpp
    rpc/client/curl_proactor.cpp
    rpc/client/executor.cpp
    rpc/client/http_client.cpp
    rpc/client/http_settings.cpp
    rpc/client/message.cpp
    rpc/client/proactor.cpp
    rpc/client/message/call_method_async.cpp
    rpc/client/message/call_method_sync.cpp
    rpc/client/message/connect.cpp
    rpc/client/message/create_context.cpp
    rpc/client/message/destroy_context.cpp
    rpc/client/message/disconnect.cpp
    rpc/client/message/send_notification_sync.cpp
    rpc/client/message/send_notification_async.cpp
    rpc/client/message/set_error_to_exception.cpp
    rpc/client/message/set_http_settings.cpp
    rpc/client/message/set_id_builder.cpp
)

add_library(json-cxx-rpc-server
    rpc/error.cpp
    rpc/server.cpp
    rpc/server/http_server.cpp
    rpc/server/http_settings.cpp
    rpc/server/microhttpd_server.cpp
)


if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
    set_source_files_properties(value.cpp PROPERTIES
        COMPILE_FLAGS "-Wno-global-constructors"
    )
endif()

install (TARGETS json-cxx json-cxx-rpc-client json-cxx-rpc-server
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
