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
 * */

#include "gtest/gtest.h"

#include "json/value.hpp"
#include "json/iterator.hpp"
#include "json/deserializer.hpp"
#include "json/deserializer_error.hpp"

#include <iostream>

using json::Value;
using json::Deserializer;
using json::DeserializerError;

class DeserializerTest : public ::testing::Test {
protected:
    virtual void SetUp() override;

    virtual void TearDown() override;

    virtual ~DeserializerTest();
};

void DeserializerTest::SetUp() { }

void DeserializerTest::TearDown() { }

DeserializerTest::~DeserializerTest() { }

TEST_F(DeserializerTest, PositiveSimpleObject) {
    Value value;

    R"({"key":"test"})" >> value;
    EXPECT_TRUE(value.is_object());
    EXPECT_EQ(value.size(), 1);
    EXPECT_EQ(value["key"], "test");
}

TEST_F(DeserializerTest, PositiveSimpleArray) {
    Value value;

    ASSERT_NO_THROW("[0, 1, 2]" >> value);
    EXPECT_TRUE(value.is_array());
    EXPECT_EQ(value.size(), 3);
    EXPECT_EQ(value[0], 0);
    EXPECT_EQ(value[1], 1);
    EXPECT_EQ(value[2], 2);
}

TEST_F(DeserializerTest, PositiveSimpleString) {
    Value value;

    ASSERT_NO_THROW(R"("test")" >> value);
    EXPECT_TRUE(value.is_string());
    EXPECT_EQ(value.size(), 0);
    EXPECT_EQ(value.as_string().size(), 4);
    EXPECT_EQ(value, "test");
}

TEST_F(DeserializerTest, PositiveSimpleNumberUnsignedInteger) {
    Value value;

    ASSERT_NO_THROW("13" >> value);
    EXPECT_TRUE(value.is_number());
    EXPECT_TRUE(value.is_uint());
    EXPECT_EQ(value.size(), 0);
    EXPECT_EQ(value, 13);
}

TEST_F(DeserializerTest, PositiveSimpleNumberSignedInteger) {
    Value value;

    ASSERT_NO_THROW("-241" >> value);
    EXPECT_TRUE(value.is_number());
    EXPECT_TRUE(value.is_int());
    EXPECT_EQ(value.size(), 0);
    EXPECT_EQ(value, -241);
}

TEST_F(DeserializerTest, PositiveSimpleNumberUnsignedDouble) {
    for (const char* test : {
        "3.17",
        "3.17\n",
        "3.17\n ",
        "3.17\n\t",
        "3.17\t\n",
        "3.17\n\t ",
        "3.17 \n \t ",
        "3.17  ",
        "   3.17",
        "\n\n\n3.17",
        "\t\t3.17",
        " \n  \t 3.17",
        " \n  \t 3.17 \n\t "
    }) {
        Value value;
        ASSERT_NO_THROW(test >> value);
        EXPECT_TRUE(value.is_number());
        EXPECT_TRUE(value.is_double());
        EXPECT_EQ(value.size(), 0);
        EXPECT_DOUBLE_EQ(value.as_double(), 3.17);
    }
}

TEST_F(DeserializerTest, NegativeSimpleNumberUnsignedDouble) {
    Value value;

    ASSERT_THROW("24." >> value, DeserializerError);
}

TEST_F(DeserializerTest, PositiveSimpleNumberSignedDouble) {
    Value value;

    ASSERT_NO_THROW("-9.36" >> value);
    EXPECT_TRUE(value.is_number());
    EXPECT_TRUE(value.is_double());
    EXPECT_EQ(value.size(), 0);
    EXPECT_DOUBLE_EQ(value.as_double(), -9.36);
}

TEST_F(DeserializerTest, NegativeSimpleNumberSignedDouble) {
    Value value;

    ASSERT_THROW("-58." >> value, DeserializerError);
}

TEST_F(DeserializerTest, PositiveSimpleTrue) {
    Value value;

    ASSERT_NO_THROW("true" >> value);
    EXPECT_TRUE(value.is_boolean());
    EXPECT_EQ(value.size(), 0);
    EXPECT_EQ(value, true);
}

TEST_F(DeserializerTest, PositiveSimpleFalse) {
    Value value;

    ASSERT_NO_THROW("false" >> value);
    EXPECT_TRUE(value.is_boolean());
    EXPECT_EQ(value.size(), 0);
    EXPECT_EQ(value, false);
}

TEST_F(DeserializerTest, PositiveSimpleNull) {
    Value value;

    ASSERT_NO_THROW("null" >> value);
    EXPECT_TRUE(value.is_null());
    EXPECT_EQ(value.size(), 0);
    EXPECT_EQ(value, nullptr);
}

TEST_F(DeserializerTest, NegativeExtTruee) {
    Value value;

    ASSERT_THROW("truee" >> value, DeserializerError);
    EXPECT_EQ(value, nullptr);
}

TEST_F(DeserializerTest, NegativeExtFalsee) {
    Value value;

    ASSERT_THROW("falsee" >> value, DeserializerError);
    EXPECT_EQ(value, nullptr);
}

TEST_F(DeserializerTest, NegativeExtNulll) {
    Value value;

    ASSERT_THROW("nulll" >> value, DeserializerError);
    EXPECT_EQ(value, nullptr);
}
