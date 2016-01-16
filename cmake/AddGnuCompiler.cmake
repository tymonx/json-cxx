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

if (NOT CMAKE_CXX_COMPILER_ID MATCHES GNU)
    return()
endif ()

function (gnu_compiler_processing)
    set(C_FLAGS   -std=c11)
    set(CXX_FLAGS -std=c++11)

    set(COMMON_WARNINGS
        -Wabi
        -Waddress
        -Waggregate-return
        -Waggressive-loop-optimizations
        -Wall
        -Warray-bounds
        -Wattributes
        -Wbool-compare
        -Wbuiltin-macro-redefined
        -Wcast-align
        -Wcast-qual
        -Wchar-subscripts
        -Wclobbered
        -Wcomment
        -Wcomments
        -Wconversion
        -Wcoverage-mismatch
        -Wdate-time
        -Wdeprecated
        -Wdeprecated-declarations
        -Wdisabled-optimization
        -Wdiv-by-zero
        -Wdouble-promotion
        -Wempty-body
        -Wendif-labels
        -Wenum-compare
        -Wextra
        -Wfloat-conversion
        -Wfloat-equal
        -Wformat
        -Wformat-nonliteral
        -Wformat-security
        -Wformat-y2k
        -Wformat=2
        -Wfree-nonheap-object
        -Wignored-qualifiers
        -Wimport
        -Winherited-variadic-ctor
        -Winit-self
        -Winline
        -Wint-to-pointer-cast
        -Winvalid-pch
        -Wlogical-not-parentheses
        -Wlogical-op
        -Wlong-long
        -Wmain
        -Wmaybe-uninitialized
        -Wmemset-transposed-args
        -Wmissing-braces
        -Wmissing-declarations
        -Wmissing-field-initializers
        -Wmissing-format-attribute
        -Wmissing-include-dirs
        -Wmissing-noreturn
        -Wmultichar
        -Wno-long-long
        -Wnonnull
        -Wodr
        -Wopenmp-simd
        -Woverflow
        -Woverlength-strings
        -Wpacked
        -Wpacked-bitfield-compat
        -Wparentheses
        -Wpedantic
        -Wpointer-arith
        -Wpragmas
        -Wredundant-decls
        -Wreturn-local-addr
        -Wreturn-type
        -Wsequence-point
        -Wshadow
        -Wshift-count-negative
        -Wshift-count-overflow
        -Wsign-compare
        -Wsign-conversion
        -Wsizeof-array-argument
        -Wsizeof-pointer-memaccess
        -Wstack-protector
        -Wstrict-aliasing
        -Wstrict-aliasing=2
        -Wstrict-overflow
        -Wstrict-overflow=5
        -Wsuggest-attribute=const
        -Wsuggest-attribute=format
        -Wsuggest-attribute=noreturn
        -Wsuggest-final-methods
        -Wsuggest-final-types
        -Wsuggest-override
        -Wswitch
        -Wswitch-bool
        -Wswitch-default
        -Wswitch-enum
        -Wsync-nand
        -Wtrampolines
        -Wtrigraphs
        -Wtype-limits
        -Wundef
        -Wuninitialized
        -Wunknown-pragmas
        -Wunreachable-code
        -Wunsafe-loop-optimizations
        -Wunused
        -Wunused-but-set-parameter
        -Wunused-but-set-variable
        -Wunused-function
        -Wunused-label
        -Wunused-macros
        -Wunused-parameter
        -Wunused-result
        -Wunused-value
        -Wunused-variable
        -Wvarargs
        -Wvariadic-macros
        -Wvector-operation-performance
        -Wvirtual-move-assign
        -Wvla
        -Wvolatile-register-var
        -Wwrite-strings
    )

    set(C_WARNINGS
        -Waggregate-return
        -Wbad-function-cast
        -Wc++-compat
        -Wc90-c99-compat
        -Wc99-c11-compat
        -Wdeclaration-after-statement
        -Wimplicit
        -Wimplicit-function-declaration
        -Wimplicit-int
        -Wmissing-parameter-type
        -Wmissing-prototypes
        -Wnested-externs
        -Wdesignated-init
        -Wdiscarded-array-qualifiers
        -Wdiscarded-qualifiers
        -Wincompatible-pointer-types
        -Wint-conversion
        -Wpointer-to-int-cast
        -Wold-style-declaration
        -Wold-style-definition
        -Woverride-init
        -Wpointer-sign
        -Wstrict-prototypes
        -Wtraditional
        -Wtraditional-conversion
        -Wunsuffixed-float-constants
    )

    set(CXX_WARNINGS
        -Wabi-tag
        -Wc++11-compat
        -Wc++14-compat
        -Wconditionally-supported
        -Wconversion-null
        -Wctor-dtor-privacy
        -Wdelete-incomplete
        -Wdelete-non-virtual-dtor
        -Weffc++
        -Winvalid-pch
        -Wliteral-suffix
        -Wmissing-field-initializers
        -Wnarrowing
        -Wconversion-null
        -Winvalid-offsetof
        -Wnon-template-friend
        -Wpmf-conversions
        -Wnoexcept
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wreorder
        -Wsign-promo
        -Wsized-deallocation
        -Wstack-protector
        -Wstrict-null-sentinel
        -Wuseless-cast
        -Wzero-as-null-pointer-constant
        -fstack-protector-all
    )

    if (NOT WARNINGS)
        set(COMMON_WARNINGS)
        set(C_WARNINGS)
        set(CXX_WARNINGS)
    endif()

    if (WARNINGS_ERROR)
        set(COMMON_WARNINGS -Werror ${COMMON_WARNINGS})
    endif()

    set(EXTRA_FLAGS
        -fPIE
        -fPIC
    )

    set(C_FLAGS     ${C_FLAGS}   ${COMMON_WARNINGS} ${C_WARNINGS}   ${EXTRA_FLAGS})
    set(CXX_FLAGS   ${CXX_FLAGS} ${COMMON_WARNINGS} ${CXX_WARNINGS} ${EXTRA_FLAGS})

    set(C_FLAGS     ${C_FLAGS}   ${CMAKE_C_FLAGS})
    set(CXX_FLAGS   ${CXX_FLAGS} ${CMAKE_CXX_FLAGS})
    set(LD_FLAGS    ${LD_FLAGS}  ${CMAKE_EXE_LINKER_FLAGS})

    string(REGEX REPLACE ";" " " C_FLAGS    "${C_FLAGS}")
    string(REGEX REPLACE ";" " " CXX_FLAGS  "${CXX_FLAGS}")
    string(REGEX REPLACE ";" " " LD_FLAGS   "${LD_FLAGS}")

    set(COMPILER_DEBUG      "-O0 -g3 -ggdb")
    set(COMPILER_RELEASE    "-O3 -DNDEBUG -pg -fdata-sections -ffunction-sections -fstack-protector-strong")
    set(COMPILER_COVERAGE   "-O0 -g --coverage")

    set(LINKER_DEBUG        "")
    set(LINKER_RELEASE      "-Wl,--gc-sections -z noexecstack -z relro -z now")
    set(LINKER_COVERAGE     "--coverage")

    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" PARENT_SCOPE)
    endif()

    set(CMAKE_C_FLAGS_DEBUG "${COMPILER_DEBUG} ${C_FLAGS}" PARENT_SCOPE)
    set(CMAKE_C_FLAGS_RELEASE "${COMPILER_RELEASE} ${C_FLAGS}" PARENT_SCOPE)
    set(CMAKE_C_FLAGS_COVERAGE "${COMPILER_COVERAGE} ${C_FLAGS}" PARENT_SCOPE)

    set(CMAKE_CXX_FLAGS_DEBUG "${COMPILER_DEBUG} ${CXX_FLAGS}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_RELEASE "${COMPILER_RELEASE} ${CXX_FLAGS}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_COVERAGE "${COMPILER_COVERAGE} ${CXX_FLAGS}" PARENT_SCOPE)

    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${LINKER_DEBUG} ${LD_FLAGS}" PARENT_SCOPE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${LINKER_RELEASE} ${LD_FLAGS}" PARENT_SCOPE)
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${LINKER_COVERAGE} ${LD_FLAGS}" PARENT_SCOPE)
endfunction ()

gnu_compiler_processing()
