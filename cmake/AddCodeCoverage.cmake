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

if (CODE_COVERAGE)
    if (NOT ${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
        message(FATAL_ERROR "Code coverage is only supported by GNU compiler. "
        "Please disable CODE_COVERAGE option or switch to GNU compiler")
    endif()

    if (NOT ${CMAKE_BUILD_TYPE} MATCHES Coverage)
        message(FATAL_ERROR "Code coverage is only supported for Coverage build. "
        "Please disable CODE_COVERAGE option or set CMAKE_BUILD_TYPE=Coverage")
    endif()

    find_program(LCOV_COMMAND lcov)
    if (NOT LCOV_COMMAND)
        message(FATAL_ERROR "Cannot find lcov command. "
        "Please disable CODE_COVERAGE option or install lcov")
    endif()

    find_program(GENHTML_COMMAND genhtml)
    if (NOT GENHTML_COMMAND)
        message(FATAL_ERROR "Cannot find genhtml command. "
        "Please disable CODE_COVERAGE option or install genhtml")
    endif()

    set(CODE_COVERAGE_COMMANDS
        COMMAND ${LCOV_COMMAND}
            --capture
            --initial
            --directory .
            --output-file coverage.base
        COMMAND ${LCOV_COMMAND}
            --capture
            --directory .
            --output-file coverage.run
        COMMAND ${LCOV_COMMAND}
            --add-tracefile coverage.base
            --add-tracefile coverage.run
            --directory .
            --output-file coverage.total
        COMMAND ${LCOV_COMMAND}
            --extract coverage.total */src/*.cpp
            --extract coverage.total */src/*.hpp
            --extract coverage.total */include/json/*.hpp
            --output-file coverage.info
        COMMAND ${GENHTML_COMMAND}
            --legend
            --title "JSON C++ library"
            --demangle-cpp
            --output-directory coverage-html
            coverage.info
    )

    message(STATUS "Enabled code coverage support")
else ()
    message(STATUS "Disabled code coverage support")
endif()
