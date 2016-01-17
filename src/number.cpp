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
 * @file json/number.cpp
 *
 * @brief JSON number interface
 * */

#include <json/number.hpp>

#include <cmath>
#include <limits>

using json::Number;

Number::operator Uint() const {
    Uint value = 0;

    switch (m_type) {
    case Type::INT:
        value = Uint(m_int);
        break;
    case Type::UINT:
        value = m_uint;
        break;
    case Type::DOUBLE:
        value = Uint(std::round(m_double));
        break;
    default:
        break;
    }

    return Uint(value);
}

Number::operator Int() const {
    Int value = 0;

    switch (m_type) {
    case Type::INT:
        value = m_int;
        break;
    case Type::UINT:
        value = Int(m_uint);
        break;
    case Type::DOUBLE:
        value = Int(std::round(m_double));
        break;
    default:
        break;
    }

    return Int(value);
}

Number::operator Double() const {
    Double value = 0;

    switch (m_type) {
    case Type::INT:
        value = Double(m_int);
        break;
    case Type::UINT:
        value = Double(m_uint);
        break;
    case Type::DOUBLE:
        value = m_double;
        break;
    default:
        break;
    }

    return value;
}

Number& Number::operator+=(const Number& number) {
    switch (m_type) {
    case Type::INT:
        m_int += Int(number);
        break;
    case Type::UINT:
        m_uint += Uint(number);
        break;
    case Type::DOUBLE:
        m_double += Double(number);
        break;
    default:
        break;
    }

    return *this;
}

bool Number::operator==(const Number& other) const {
    bool result;

    switch (get_type()) {
    case Number::Type::INT:
        result = (m_int == Int(other));
        break;
    case Number::Type::UINT:
        result = (m_uint == Uint(other));
        break;
    case Number::Type::DOUBLE:
        result = std::fabs(m_double - Double(other)) <
            std::numeric_limits<Double>::epsilon();
        break;
    default:
        result = false;
        break;
    }

    return result;
}

bool Number::operator<(const Number& other) const {
    bool result;

    switch (get_type()) {
    case Number::Type::INT:
        result = (m_int < Int(other));
        break;
    case Number::Type::UINT:
        result = (m_uint < Uint(other));
        break;
    case Number::Type::DOUBLE:
        result = (m_double < Double(other));
        break;
    default:
        result = false;
        break;
    }

    return result;
}

bool Number::operator!() const {
    bool result;

    switch (get_type()) {
    case Number::Type::INT:
        result = !m_int;
        break;
    case Number::Type::UINT:
        result = !m_uint;
        break;
    case Number::Type::DOUBLE:
        result = std::fabs(m_double) < std::numeric_limits<Double>::epsilon();
        break;
    default:
        result = false;
        break;
    }

    return result;
}
