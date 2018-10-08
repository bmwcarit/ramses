/*
 * Copyright (C) 2013 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "ramses-capu/util/StringTokenizer.h"
#include "ramses-capu/Error.h"
#include "ramses-capu/Config.h"

TEST(StringTokenizer, TokenizeString)
{
    //a simple example
    ramses_capu::String str("Hello World!");
    ramses_capu::StringTokenizer tokenizer(str, " ");
    ramses_capu::StringTokenizer::iterator it = tokenizer.begin();

    EXPECT_EQ(2u, tokenizer.tokenCount());
    EXPECT_EQ(ramses_capu::String("Hello"), *it);
    it++;
    EXPECT_EQ(ramses_capu::String("World!"), *it);
    it++;
    EXPECT_EQ(tokenizer.end(), it);


    //a more complicated example, testing also the reuse of the tokenizer object.
    str = "HellostopWorldstopthisstopisstopme ";
    tokenizer.tokenize(str, "stop");
    it = tokenizer.begin();

    EXPECT_EQ(5u, tokenizer.tokenCount());
    EXPECT_EQ(ramses_capu::String("Hello"), *it);
    it++;
    EXPECT_EQ(ramses_capu::String("World"), *it);
    it++;
    EXPECT_EQ(ramses_capu::String("this"), *it);
    it++;
    EXPECT_EQ(ramses_capu::String("is"), *it);
    it++;
    EXPECT_EQ(ramses_capu::String("me "), *it);
    it++;

}

TEST(StringTokenizer, TokenizeStringWithoutDelimiter)
{
    ramses_capu::String str("HelloWorld");

    ramses_capu::StringTokenizer tokenizer; //parameterless constructor
    EXPECT_EQ(0u, tokenizer.tokenCount());
    EXPECT_EQ(tokenizer.end(), tokenizer.begin());

    tokenizer.tokenize(str, "stop");

    EXPECT_EQ(1u, tokenizer.tokenCount());
    EXPECT_EQ(ramses_capu::String("HelloWorld"), *(tokenizer.begin()));
}
TEST(StringTokenizer, TokenizeEmptyString)
{
    ramses_capu::String str("");
    ramses_capu::StringTokenizer tokenizer(str, "stop");

    EXPECT_EQ(0u, tokenizer.tokenCount());
}

TEST(StringTokenizer, TokenizeEmptyDelimiter)
{
    ramses_capu::String str("Hello World");
    ramses_capu::StringTokenizer tokenizer(str, "");

    EXPECT_EQ(1u, tokenizer.tokenCount());
    EXPECT_EQ(ramses_capu::String("Hello World"), *(tokenizer.begin()));
}
