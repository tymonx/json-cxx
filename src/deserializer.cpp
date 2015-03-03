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
 * @file deserializer.cpp
 *
 * @brief JSON deserializer implementation
 * */

#include "json/deserializer.hpp"

#include <cmath>
#include <cstring>

using namespace json;

/*! Error parsing code */
using Code = Deserializer::Error::Code;

/*! UTF-16 surrogate pair */
using Surrogate = std::pair<unsigned, unsigned>;

/*! Maximu characters to parse per single JSON value. Stack protection */
const size_t Deserializer::MAX_LIMIT_PER_OBJECT = 0xFFFFFFFF;

static constexpr char JSON_NULL[] = "null";
static constexpr char JSON_TRUE[] = "true";
static constexpr char JSON_FALSE[] = "false";
static constexpr size_t ESCAPE_HEX_DIGITS_SIZE = 6;
static constexpr Surrogate SURROGATE_MIN(0xD800, 0xDC00);
static constexpr Surrogate SURROGATE_MAX(0xDBFF, 0xDFFF);

/*!
 * @brief   Get string length without null termination '\0'
 * @return  String length
 * */
template<size_t N>
constexpr size_t length(const char (&)[N]) { return (N - 1); }

/*!
 * @brief   Get array size based on the type of given object
 * @return  Array size, number of elements in the array
 * */
template<class T, size_t N>
constexpr size_t array_size(T (&)[N]) { return N; }

Deserializer::Deserializer() :
    m_array{},
    m_begin(nullptr),
    m_pos(nullptr),
    m_end(nullptr),
    m_limit(MAX_LIMIT_PER_OBJECT),
    m_error_code(Error::Code::NONE) { }

Deserializer::Deserializer(const Deserializer& deserializer) : Deserializer() {
    m_array = deserializer.m_array;
}

Deserializer::Deserializer(Deserializer&& deserializer) : Deserializer() {
    m_array = std::move(deserializer.m_array);
    deserializer.m_array.clear();

    deserializer.m_begin = nullptr;
    deserializer.m_pos = nullptr;
    deserializer.m_end = nullptr;
    deserializer.m_error_code = Error::Code::NONE;
}

Deserializer::Deserializer(const char* str) : Deserializer() {
    (*this) << str;
}

Deserializer::Deserializer(const String& str) : Deserializer() {
    (*this) << str;
}

Deserializer& Deserializer::operator=(const Deserializer& deserializer) {
    if (this == &deserializer) { return *this; }

    m_array = deserializer.m_array;

    m_begin = nullptr;
    m_pos = nullptr;
    m_end = nullptr;
    m_error_code = Error::Code::NONE;

    return *this;
}

Deserializer& Deserializer::operator=(Deserializer&& deserializer) {
    if (this == &deserializer) { return *this; }

    m_array = std::move(deserializer.m_array);
    deserializer.m_array.clear();

    m_begin = nullptr;
    m_pos = nullptr;
    m_end = nullptr;
    m_error_code = Error::Code::NONE;

    deserializer.m_begin = nullptr;
    deserializer.m_pos = nullptr;
    deserializer.m_end = nullptr;
    deserializer.m_error_code = Error::Code::NONE;

    return *this;
}

Deserializer& Deserializer::operator<<(const char* str) {
    clear_error();

    size_t str_size = 0;

    if (nullptr != str) {
        str_size = std::strlen(str);
    }

    m_begin = str;
    m_pos = str;
    m_end = str + str_size;

    parsing();

    return *this;
}

Deserializer& Deserializer::operator<<(const String& str) {
    clear_error();

    m_begin = str.cbegin().base();
    m_pos = m_begin;
    m_end = str.cend().base();

    parsing();

    return *this;
}

Deserializer& Deserializer::operator>>(Value& value) {
    if (m_array.empty()) {
        value = Value::Type::NIL;
    }
    else {
        value = std::move(m_array.back());
        m_array.pop_back();
    }

    return *this;
}

Deserializer json::operator>>(const char* str, Value& val) {
    return Deserializer(str) >> val;
}

Deserializer json::operator>>(const String& str, Value& val) {
    return Deserializer(str.c_str()) >> val;
}

bool Deserializer::empty() const {
    return m_array.empty();
}

size_t Deserializer::size() const {
    return m_array.size();
}

void Deserializer::set_limit(size_t limit) {
    m_limit = limit;
}

void Deserializer::parsing() {
    Value root;

    const char* store_end = m_end;

    m_end = m_begin + m_limit;
    while (read_object_or_array(root)) {
        m_array.push_back(std::move(root));

        const char* tmp_end = m_pos + m_limit;
        m_end = tmp_end < store_end ? tmp_end : store_end;
    }

    m_end = store_end;
}

inline void Deserializer::prev_char() {
    --m_pos;
}

inline void Deserializer::next_char() {
    ++m_pos;
}

inline void Deserializer::back_chars(size_t count) {
    m_pos -= count;
}

inline void Deserializer::skip_chars(size_t count) {
    m_pos += count;
}

inline char Deserializer::get_char() const {
    return *m_pos;
}

inline const char* Deserializer::get_position() const {
    return m_pos;
}

inline bool Deserializer::is_end() const {
    return m_pos >= m_end;
}

inline bool Deserializer::is_outbound(size_t offset) {
    return (m_pos + offset) > m_end;
}

inline void Deserializer::clear_error() {
    m_error_code = Error::Code::NONE;
}

Deserializer::Error Deserializer::get_error() const {
    Error error;

    error.code = m_error_code;
    error.line = 1;
    error.column = 1;
    error.offset = size_t(m_pos - m_begin);
    error.size = size_t(m_end - m_begin);

    const char* search = m_begin;

    while (search < m_pos) {
        if ('\n' == *search) {
            ++error.line;
            error.column = 1;
        }
        else {
            ++error.column;
        }
        search++;
    }

    return error;
}

bool Deserializer::is_invalid() const {
    return Error::Code::NONE != m_error_code;
}

inline bool Deserializer::read_object_or_array(Value& value) {
    if (!read_whitespaces()) { return false; }

    bool ok = false;

    switch (get_char()) {
    case '{':
        value = Value::Type::OBJECT;
        ok = read_object(value);
        break;
    case '[':
        value = Value::Type::ARRAY;
        ok = read_array(value);
        break;
    default:
        break;
    }

    return ok;
}

bool Deserializer::read_object(Value& value) {
    size_t capacity = 0;

    if (!read_curly_open()) { return false; }
    if (!count_object_members(capacity)) { return false; }

    value.m_type = Value::Type::OBJECT;
    new (&value.m_object) Object();
    value.m_object.reserve(capacity);

    if (read_curly_close()) { return true; }
    clear_error();

    String key;

    do {
        if (!read_string(key)) { return false; }
        if (!read_colon()) { return false; }
        if (!read_value(value[key])) { return false; }
        if (!read_comma()) { clear_error(); return read_curly_close(); }
        key.clear();
    } while (true);
}

bool Deserializer::read_string(String& str) {
    size_t capacity = 1;

    if (!read_quote()) { return false; }
    if (!count_string_chars(capacity)) { return false; }

    str.reserve(capacity);

    while (!is_end()) {
        if ('\\' == get_char()) {
            next_char();
            if (!read_string_escape(str)) { return false; };
        }
        else if ('"' != get_char()) {
            str.push_back(get_char());
            next_char();
        }
        else {
            next_char();
            return true;
        }
    }

    set_error(Code::END_OF_FILE);
    return false;
}

bool Deserializer::read_string_escape(String& str) {
    bool ok = true;

    switch (get_char()) {
    case '"':
    case '\\':
    case '/':
        str.push_back(get_char());
        next_char();
        break;
    case 'b':
        str.push_back('\b');
        next_char();
        break;
    case 'f':
        str.push_back('\f');
        next_char();
        break;
    case 'n':
        str.push_back('\n');
        next_char();
        break;
    case 'r':
        str.push_back('\r');
        next_char();
        break;
    case 't':
        str.push_back('\t');
        next_char();
        break;
    case 'u':
        prev_char();
        ok = read_string_escape_code(str);
        break;
    default:
        set_error(Code::INVALID_ESCAPE);
        ok = false;
        break;
    }

    return ok;
}

static inline
uint32_t decode_utf16_surrogate_pair(const Surrogate& surrogate) {
    return 0x10000
        | ((0x03F & surrogate.first) << 10)
        |  (0x3FF & surrogate.second);
}

inline bool Deserializer::read_string_escape_code(String& str) {
    Surrogate surrogate;
    uint32_t code;

    if (!read_unicode(code)) { return false; }
    if (read_unicode(surrogate.second)) {
        surrogate.first = code;
        if ((SURROGATE_MIN <= surrogate) && (surrogate <= SURROGATE_MAX)) {
            code = decode_utf16_surrogate_pair(surrogate);
        }
        else {
            back_chars(ESCAPE_HEX_DIGITS_SIZE);
        }
    }

    if (code < 0x80) {
        str.push_back(char(code));
    }
    else if (code < 0x800) {
        str.push_back(char(0xC0 | (0x1F & (code >>  6))));
        str.push_back(char(0x80 | (0x3F & (code >>  0))));
    }
    else if (code < 0x10000) {
        str.push_back(char(0xE0 | (0x0F & (code >> 12))));
        str.push_back(char(0x80 | (0x3F & (code >>  6))));
        str.push_back(char(0x80 | (0x3F & (code >>  0))));
    }
    else {
        str.push_back(char(0xF0 | (0x07 & (code >> 18))));
        str.push_back(char(0x80 | (0x3F & (code >> 12))));
        str.push_back(char(0x80 | (0x3F & (code >>  6))));
        str.push_back(char(0x80 | (0x3F & (code >>  0))));
    }

    return true;
}

inline bool Deserializer::read_unicode(uint32_t& code) {
    if (is_outbound(ESCAPE_HEX_DIGITS_SIZE)) {
        set_error(Code::END_OF_FILE);
        return false;
    }

    const char* ch = get_position();
    if ('\\' != ch[0]) { return false; }
    if ('u'  != ch[1]) { return false; }

    for (size_t i = 2; i < ESCAPE_HEX_DIGITS_SIZE; i++) {
        if (!std::isxdigit(ch[i])) {
            set_error(Code::INVALID_UNICODE);
            return false;
        }
    }

    size_t unused;
    code = unsigned(std::stoul(ch + 2, &unused, 16));

    skip_chars(ESCAPE_HEX_DIGITS_SIZE);

    return true;
}

bool Deserializer::read_value(Value& value) {
    bool ok = false;
    String str;

    if (!read_whitespaces()) { return false; }

    switch (get_char()) {
    case '"':
        ok = read_string(str);
        value.m_type = Value::Type::STRING;
        new (&value.m_string) String(std::move(str));
        break;
    case '{':
        ok = read_object(value);
        break;
    case '[':
        ok = read_array(value);
        break;
    case 't':
        ok = read_true(value);
        break;
    case 'f':
        ok = read_false(value);
        break;
    case 'n':
        ok = read_null(value);
        break;
    case '-':
        ok = read_number(value);
        break;
    default:
        if (std::isdigit(get_char())) {
            ok = read_number(value);
        } else { set_error(Code::MISS_VALUE); }
        break;
    }

    return ok;
}

bool Deserializer::read_array(Value& value) {
    size_t capacity = 0;

    if (!read_square_open()) { return false; }
    if (!count_array_values(capacity)) { return false; }

    value.m_type = Value::Type::ARRAY;
    new (&value.m_array) Array();
    value.m_array.reserve(capacity);

    if (read_square_close()) { return true; }
    clear_error();

    Value array_value;
    bool processing = true;

    do {
        if (!read_value(array_value)) { return false; }
        value.m_array.push_back(std::move(array_value));
        if (!read_comma()) {
            clear_error();
            processing = false;
        }
    } while (processing);

    return read_square_close();
}

inline bool Deserializer::read_colon() {
    if (!read_whitespaces()) { return false; }
    if (':' != get_char()) {
        set_error(Code::MISS_COLON);
        return false;
    }
    next_char();
    return true;
}

inline bool Deserializer::read_quote() {
    if (!read_whitespaces()) { return false; }
    if ('"' != get_char()) {
        set_error(Code::MISS_QUOTE);
        return false;
    }
    next_char();
    return true;
}

inline bool Deserializer::read_curly_open() {
    if (!read_whitespaces()) { return false; }
    if ('{' != get_char()) {
        set_error(Code::MISS_CURLY_OPEN);
        return false;
    }
    next_char();
    return true;
}

inline bool Deserializer::read_curly_close() {
    if (!read_whitespaces()) { return false; }
    if ('}' != get_char()) {
        set_error(Code::MISS_CURLY_CLOSE);
        return false;
    }
    next_char();
    return true;
}

inline bool Deserializer::read_square_open() {
    if (!read_whitespaces()) { return false; }
    if ('[' != get_char()) {
        set_error(Code::MISS_SQUARE_OPEN);
        return false;
    }
    next_char();
    return true;
}

inline bool Deserializer::read_square_close() {
    if (!read_whitespaces()) { return false; }
    if (']' != get_char()) {
        set_error(Code::MISS_SQUARE_CLOSE);
        return false;
    }
    next_char();
    return true;
}

inline bool Deserializer::read_comma() {
    if (!read_whitespaces()) { return false; }
    if (',' != get_char()) {
        set_error(Code::MISS_COMMA);
        return false;
    }
    next_char();
    return true;
}

inline bool Deserializer::read_whitespaces() {
    while (!is_end()) {
        switch (get_char()) {
        case ' ':
        case '\n':
        case '\r':
        case '\t':
            next_char();
            break;
        default:
            return true;
        }
    }

    set_error(Code::END_OF_FILE);
    return false;
}

inline bool Deserializer::read_number_digit(String& str) {
    bool ok = false;

    while (!is_end()) {
        if (std::isdigit(get_char())) {
            str.push_back(get_char());
            next_char();
            ok = true;
        } else { return ok; }
    }

    return ok;
}

inline bool Deserializer::read_number_integer(String& str) {
    if (is_end()) { set_error(Code::END_OF_FILE); return false; }

    if ('-' == get_char()) {
        str.push_back(get_char());
        next_char();
        if (is_end()) { set_error(Code::END_OF_FILE); return false; }
    }

    if ('0' == get_char()) {
        str.push_back(get_char());
        next_char();
        return true;
    }

    return read_number_digit(str);
}

inline bool Deserializer::read_number_fractional(String& str) {
    if (is_end()) { set_error(Code::END_OF_FILE); return false; }

    bool ok = true;

    if ('.' == get_char()) {
        str += "0.";
        next_char();
        ok = read_number_digit(str);
    }

    return ok;
}

inline bool Deserializer::read_number_exponent(String& str) {
    if (is_end()) { set_error(Code::END_OF_FILE); return false; }

    if (('e' != get_char()) && ('E' != get_char())) { return true; }

    next_char();
    if (is_end()) { set_error(Code::END_OF_FILE); return false; }

    if (('+' == get_char()) || ('-' == get_char())) {
        str.push_back(get_char());
        next_char();
    }

    return read_number_digit(str);
}

bool Deserializer::read_number(Value& value) {
    using std::pow;
    using std::stol;
    using std::stod;
    using std::signbit;

    String str_integer;
    String str_fractional;
    String str_exponent;

    read_whitespaces();
    if (!read_number_integer(str_integer)) {
        set_error(Code::INVALID_NUMBER_INTEGER);
        return false;
    }
    if (!read_number_fractional(str_fractional)) {
        set_error(Code::INVALID_NUMBER_FRACTION);
        return false;
    }
    if (!read_number_exponent(str_exponent)) {
        set_error(Code::INVALID_NUMBER_EXPONENT);
        return false;
    }

    long integer = long(stoull(str_integer));
    Double fractional = str_fractional.empty() ? 0.0 : stod(str_fractional);
    long exponent = str_exponent.empty() ? 0 : stoll(str_exponent);

    if (str_fractional.empty()) {
        if (signbit(exponent)) {
            value = Double(integer) * pow(10, exponent);
        } else {
            integer *= Int(pow(10, exponent));
            if (signbit(integer)) { value = Int(integer); }
            else { value = Uint(integer); }
        }
    } else {
        fractional = ('-' == str_integer[0]) ? -fractional : fractional;
        value = (Double(integer) + fractional) * pow(10, exponent);
    }

    return true;
}

inline bool Deserializer::read_true(Value& value) {
    if (is_outbound(length(JSON_TRUE))) {
        set_error(Code::END_OF_FILE);
        return false;
    }

    if (0 != std::strncmp(get_position(), JSON_TRUE, length(JSON_TRUE))) {
        set_error(Code::NOT_MATCH_TRUE);
        return false;
    }

    value = true;

    skip_chars(length(JSON_TRUE));
    return true;
}

inline bool Deserializer::read_false(Value& value) {
    if (is_outbound(length(JSON_FALSE))) {
        set_error(Code::END_OF_FILE);
        return false;
    }

    if (0 != std::strncmp(get_position(), JSON_FALSE, length(JSON_FALSE))) {
        set_error(Code::NOT_MATCH_FALSE);
        return false;
    }

    value = false;

    skip_chars(length(JSON_FALSE));
    return true;
}

inline bool Deserializer::read_null(Value& value) {
    if (is_outbound(length(JSON_NULL))) {
        set_error(Code::END_OF_FILE);
        return false;
    }

    if (0 != std::strncmp(get_position(), JSON_NULL, length(JSON_NULL))) {
        set_error(Code::NOT_MATCH_NULL);
        return false;
    }

    value = nullptr;

    skip_chars(length(JSON_NULL));
    return true;
}

inline bool Deserializer::count_array_values(size_t& count) {
    const char* pos = get_position();

    bool ok = false;
    bool processing = true;
    size_t braces = 0;

    while ((pos < m_end) && processing) {
        switch (*pos) {
        case '[':
            ++braces;
            break;
        case ',':
            if (0 == braces) { ++count; }
            break;
        case ']':
            if (0 == braces) { ok = true; processing = false; }
            else { --braces; }
            break;
        default:
            break;
        }
        ++pos;
    }

    return ok;
}

inline bool Deserializer::count_object_members(size_t& count) {
    const char* pos = get_position();

    bool ok = false;
    bool processing = true;
    size_t braces = 0;

    while ((pos < m_end) && processing) {
        switch (*pos) {
        case '{':
            ++braces;
            break;
        case ':':
            if (0 == braces) { ++count; }
            break;
        case '}':
            if (0 == braces) { ok = true; processing = false; }
            else { --braces; }
            break;
        default:
            break;
        }
        ++pos;
    }

    return ok;
}

inline bool Deserializer::count_string_chars(size_t& count) {
    const char* pos = get_position();

    bool ok = false;
    bool processing = true;

    while ((pos < m_end) && processing) {
        switch (*pos) {
        case '"':
            ok = true;
            processing = false;
            break;
        case '\\':
            count += 2;
            ++pos;
            break;
        default:
            ++count;
            break;
        }
        ++pos;
    }

    return ok;
}

inline void Deserializer::set_error(Error::Code error_code) {
    if (Error::Code::NONE == m_error_code) {
        m_error_code = error_code;
    }
}

/*! Internal error codes message */
struct ErrorCodes {
    Deserializer::Error::Code code; /*!< Error parsing code */
    const char* message;            /*!< Error parsing message */
};

static const struct ErrorCodes g_error_codes[] = {
    { Code::NONE,               "No error"},
    { Code::END_OF_FILE,        "End of file reached"},
    { Code::MISS_QUOTE,         "Missing quote '\"' for string"},
    { Code::MISS_COMMA,         "Missing comma ',' in array/members"},
    { Code::MISS_COLON,         "Missing colon ':' in member pair"},
    { Code::MISS_CURLY_OPEN,    "Missing curly '{' for object"},
    { Code::MISS_CURLY_CLOSE,   "Missing curly '}' for object"},
    { Code::MISS_SQUARE_OPEN,   "Missing curly '[' for array"},
    { Code::MISS_SQUARE_CLOSE,  "Missing curly ']' for array"},
    { Code::NOT_MATCH_NULL,     "Did you mean 'null'?"},
    { Code::NOT_MATCH_TRUE,     "Did you mean 'true'?"},
    { Code::NOT_MATCH_FALSE,    "Did you mean 'false'?"},
    { Code::MISS_VALUE,         "Missing value in array/member"},
    { Code::INVALID_ESCAPE,     "Invalid escape character"},
    { Code::INVALID_UNICODE,    "Invalid unicode"},
    { Code::INVALID_NUMBER_INTEGER, "Invalid number integer part"},
    { Code::INVALID_NUMBER_FRACTION,"Invalid number fractional part"},
    { Code::INVALID_NUMBER_EXPONENT,"Invalid number exponent part"}
};

const char* Deserializer::Error::decode() {
    for (const auto& error : g_error_codes) {
        if (error.code == code) {
            return error.message;
        }
    }
    return "Unknown error";
}
