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

if (${TESTS})
    include_directories(SYSTEM gtest/include)

    add_subdirectory(gtest)

    add_executable(tests_runner
        tests_runner.cpp
        test_deserializer.cpp
    )

    target_link_libraries(tests_runner
        json-cxx
        gtest
        pthread
    )

    if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
        set_target_properties(tests_runner PROPERTIES
            COMPILE_FLAGS "-Wno-global-constructors"
        )
    endif()

    if (MEMORY_CHECK)
        add_custom_target(memory_check
            COMMAND ${MEMORY_CHECK_COMMAND} ${MEMORY_CHECK_ARGS} bin/tests_runner
            DEPENDS tests_runner
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    else()
        add_custom_target(memory_check
            COMMAND ${CMAKE_COMMAND} -E echo "Memory check option is disabled!"
        )
    endif()

    if (CODE_COVERAGE)
        add_custom_target(code_coverage
            COMMAND bin/tests_runner
            ${CODE_COVERAGE_COMMANDS}
            DEPENDS tests_runner
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    else()
        add_custom_target(code_coverage
            COMMAND ${CMAKE_COMMAND} -E echo "Code coverage option is disabled!"
        )
    endif()

    message(STATUS "Enabled tests")
else()
    message(STATUS "Disabled tests")
endif()
