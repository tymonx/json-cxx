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

if (NOT GTEST_DIR)
    set(GTEST_DIR ${CMAKE_SOURCE_DIR}/gtest)
endif()

if (NOT GTEST_FOUND)
    if (NOT EXISTS ${GTEST_DIR})
        include(ExternalProject)

        ExternalProject_Add(
            googletest
            URL https://googletest.googlecode.com/files/gtest-1.7.0.zip
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
            # Disable install step
            INSTALL_COMMAND ""
        )

        # Create a gtest target to be used as a dependency by test programs
        add_library(gtest IMPORTED STATIC GLOBAL)
        add_dependencies(gtest googletest)

        # Set gtest properties
        ExternalProject_Get_Property(googletest source_dir binary_dir)
        set_target_properties(gtest PROPERTIES
            IMPORTED_LOCATION ${binary_dir}/libgtest.a
        )

        set(GTEST_DIR ${source_dir})
    else ()
        add_subdirectory(${GTEST_DIR})
    endif()

    set(GTEST_FOUND TRUE)
    set(GTEST_LIB gtest)
    set(GTEST_MAIN gtest-main)
    set(GTEST_INCLUDE ${GTEST_DIR}/include)

    enable_testing()

    function(add_gtest test_target)
        set(test_sources)
        foreach (arg ${ARGN})
            set(test_sources ${test_sources} ${arg})
        endforeach()

        foreach (suppression ${VALGRIND_SUPPRESSIONS})
            set(VALGRIND_EXTRA_ARGS ${VALGRIND_EXTRA_ARGS}
                --suppressions=${suppression}
            )
        endforeach()

        set(VALGRIND_ARGS
            --tool=memcheck
            --leak-check=full
            --show-leak-kinds=all
            --error-exitcode=1
            --errors-for-leak-kinds=all
            ${VALGRIND_EXTRA_ARGS}
        )

        add_executable(${test_target} ${test_sources})
        target_link_libraries(${test_target}
            ${GTEST_LIB}
            pthread
        )
        target_include_directories(${test_target} SYSTEM PUBLIC
            ${GTEST_INCLUDE}
        )

        if (TARGET code-coverage)
            add_dependencies(code-coverage ${test_target})
        endif()

        set_target_properties(${test_target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY
            ${CMAKE_BINARY_DIR}/bin/tests
        )

        if (CMAKE_CXX_COMPILER_ID MATCHES GNU)
            set_target_properties(${test_target} PROPERTIES
                COMPILE_FLAGS "-Wno-useless-cast -Wno-effc++ -Wno-inline -Wno-zero-as-null-pointer-constant"
            )
        endif()

        if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
                set_target_properties(${test_target} PROPERTIES
                COMPILE_FLAGS "-Wno-global-constructors"
            )
        endif()

        find_program(VALGRIND_COMMAND valgrind)

        if (VALGRIND_COMMAND)
            add_test(
                NAME ${test_target}
                COMMAND valgrind ${VALGRIND_ARGS} $<TARGET_FILE:${test_target}>
            )
        else ()
            add_test(NAME ${test_target} COMMAND $<TARGET_FILE:${test_target}>)
        endif ()
    endfunction()
endif()
