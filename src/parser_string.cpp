/*!
 * @copyright
 * Copyright (c) 2016, Tymoteusz Blazejczyk
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
 * @file parser_string.cpp
 *
 * @brief JSON parser string interface
 * */

#include "parser_string.hpp"

#include <json/parser_error.hpp>

#include <utility>

using json::Size;
using json::Char;
using json::ParserString;
using Error = json::ParserError;

using Uint16 = std::uint_fast16_t;
using Uint32 = std::uint_fast32_t;
using SurrogatePair = std::pair<Uint16, Uint16>;

static constexpr Size UNICODE_LENGTH = 4;

static constexpr SurrogatePair SURROGATE_MIN{0xD800, 0xDC00};
static constexpr SurrogatePair SURROGATE_MAX{0xDBFF, 0xDFFF};

static inline
int is_utf16_surrogate_pair(const SurrogatePair& pair) {
    return (pair >= SURROGATE_MIN) && (pair <= SURROGATE_MAX);
}

static inline
Uint32 decode_utf16_surrogate_pair(const SurrogatePair& pair) {
    return 0x10000 |
        ((0x3F & Uint32(pair.first)) << 10) |
        (0x3FF & Uint32(pair.second));
}

static
Uint16 read_unicode(const Char* pos, const Char* end) {
    if (pos + UNICODE_LENGTH > end) { throw Error{Error::END_OF_FILE, end}; }

    end = pos + UNICODE_LENGTH;
    Uint16 code{0};

    while (pos < end) {
        Uint16 ch = Uint16(*pos);
        if ((ch >= '0') && (ch <= '9')) {
            ch = ch - '0';
        }
        else if ((ch >= 'A') && (ch <= 'F')) {
            ch = (ch + 0xA - 'A');
        }
        else if ((ch >= 'a') && (ch <= 'f')) {
            ch = (ch + 0xA - 'a');
        }
        else {
            throw Error{Error::INVALID_UNICODE, pos};
        }
        code = (code << 4) | ch;
        ++pos;
    }

    return code;
}

void ParserString::parsing(Char* str) {
    m_str = str;

    int ch = *(m_pos++);
    while ('"' != ch) {
        if ('\\' == ch) {
            ch = *(m_pos++);
            switch (ch) {
            case '"':
            case '\\':
            case '/':
                break;
            case 'n':
                ch = '\n';
                break;
            case 'r':
                ch = '\r';
                break;
            case 't':
                ch = '\t';
                break;
            case 'b':
                ch = '\b';
                break;
            case 'f':
                ch = '\f';
                break;
            case 'u':
                ch = read_string_unicode();
                continue;
            default:
                throw Error{Error::INVALID_ESCAPE, m_pos};
            }
        }
        *(m_str++) = Char(ch);
        ch = *(m_pos++);
    }
}

int ParserString::read_string_unicode() {
    Uint32 unicode = read_unicode(m_pos, m_end);
    m_pos += UNICODE_LENGTH;

    if ((m_pos < m_end) && ('\\' == m_pos[0]) && ('u' == m_pos[1])) {
        SurrogatePair surrogate{unicode, read_unicode(m_pos + 2, m_end)};
        if (is_utf16_surrogate_pair(surrogate)) {
            unicode = decode_utf16_surrogate_pair(surrogate);
            m_pos += (2 + UNICODE_LENGTH);
        }
    }

    if (unicode < 0x80) {
        *(m_str++) = Char(unicode);
    }
    else if (unicode < 0x800) {
        *(m_str++) = Char(0xC0 | (0x1F & (unicode >>  6)));
        *(m_str++) = Char(0x80 | (0x3F & unicode));
    }
    else if (unicode < 0x10000) {
        *(m_str++) = Char(0xE0 | (0x0F & (unicode >> 12)));
        *(m_str++) = Char(0x80 | (0x3F & (unicode >>  6)));
        *(m_str++) = Char(0x80 | (0x3F & unicode));
    }
    else {
        *(m_str++) = Char(0xF0 | (0x07 & (unicode >> 18)));
        *(m_str++) = Char(0x80 | (0x3F & (unicode >> 12)));
        *(m_str++) = Char(0x80 | (0x3F & (unicode >>  6)));
        *(m_str++) = Char(0x80 | (0x3F & unicode));
    }

    return *(m_pos++);
}

Size ParserString::count_string_chars(const Char* pos, const Char* end) {
    Size count{0};

    for (; (pos < end) && ('"' != *pos); ++pos, ++count) {
        if ('\\' == *pos) {
            if ('u' == *(++pos)) {
                Uint16 code = read_unicode(++pos, end);
                if (code >= 0x80) {
                    if (code < 0x800) {
                        count += 1;
                    }
                    else if ((code < SURROGATE_MIN.first) ||
                             (code > SURROGATE_MAX.first)) {
                        count += 2;
                    }
                    else {
                        count += 3;
                    }
                }
                pos += 3;
            }
        }
    }

    if (pos >= end) { throw Error{Error::END_OF_FILE, end}; }

    return count;
}
