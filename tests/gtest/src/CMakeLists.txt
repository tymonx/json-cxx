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

add_library(gtest STATIC
    gtest.cc
    gtest-death-test.cc
    gtest-filepath.cc
    gtest-port.cc
    gtest-printers.cc
    gtest-test-part.cc
    gtest-typed-test.cc
)

if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
    set_target_properties(gtest PROPERTIES
        COMPILE_FLAGS "\
        -Wno-global-constructors \
        -Wno-undef \
        -Wno-sign-conversion \
        -Wno-exit-time-destructors \
        -Wno-missing-variable-declarations \
        -Wno-missing-prototypes \
        -Wno-used-but-marked-unused \
        -Wno-switch-enum \
        -Wno-format-nonliteral \
        -Wno-weak-vtables \
        -Wno-unused-member-function \
        -Wno-disabled-macro-expansion
        "
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES GNU)
    set_target_properties(gtest PROPERTIES
        COMPILE_FLAGS "\
        -Wno-global-constructors \
        -Wno-undef \
        -Wno-sign-conversion \
        -Wno-exit-time-destructors \
        -Wno-missing-variable-declarations \
        -Wno-used-but-marked-unused \
        -Wno-switch-enum \
        -Wno-format-nonliteral \
        -Wno-weak-vtables \
        -Wno-unused-member-function \
        -Wno-disabled-macro-expansion \
        -Wno-effc++ \
        -Wno-zero-as-null-pointer-constant \
        -Wno-missing-declarations \
        -Wno-useless-cast \
        -Wno-switch-default \
        -Wno-suggest-attribute=format \
        -Wno-strict-overflow \
        -Wno-suggest-attribute=noreturn
        "
    )
endif()
