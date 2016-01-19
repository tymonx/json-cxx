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
    array.cpp
    allocator.cpp
    default_allocator.cpp
    exception.cpp
    number.cpp
    object.cpp
    pair.cpp
    parser.cpp
    parser_error.cpp
    string.cpp
    value.cpp
)

if (CMAKE_CXX_COMPILER_ID MATCHES GNU)
    set_source_files_properties(array.cpp object.cpp  parser.cpp PROPERTIES
        COMPILE_FLAGS "-Wno-strict-overflow -Wno-unsafe-loop-optimizations"
    )
    set_source_files_properties(allocator.cpp PROPERTIES
        COMPILE_FLAGS "-Wno-suggest-attribute=const"
    )
    set_source_files_properties(parser.cpp PROPERTIES
        COMPILE_FLAGS "-Wno-inline"
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
    set_source_files_properties(default_allocator.cpp  parser.cpp PROPERTIES
        COMPILE_FLAGS "-Wno-exit-time-destructors"
    )
endif()

add_executable(main
    #/home/tymon/workspace/arm/gcc-arm-none-eabi-5_2-2015q4/share/gcc-arm-none-eabi/samples/startup/startup_ARMCM0.S
    main.cpp
)
target_link_libraries(main json-cxx)

install (TARGETS json-cxx
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
