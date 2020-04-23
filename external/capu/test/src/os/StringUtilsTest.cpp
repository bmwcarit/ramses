/*
 * Copyright (C) 2012 BMW Car IT GmbH
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

#include "ramses-capu/os/StringUtils.h"
#include <gtest/gtest.h>

TEST(StringUtils, Strncpy)
{
    char string1[20] = "My String";
    char string2[30];
    ramses_capu::StringUtils::Strncpy(string2, sizeof(string2), string1);
    EXPECT_STREQ(string1, string2);
}

TEST(StringUtils, Strncpy2)
{
    char string1[20] = "My String";
    char string2[20];
    std::memset(string2, 42, sizeof(string2));
    ramses_capu::StringUtils::Strncpy(string2, 4, string1);
    EXPECT_STREQ("My ", string2);

    // check that only 4 bytes are written, an no more
    for (size_t i = 4; i < sizeof(string2); ++i)
    {
        EXPECT_EQ(42, string2[i]);
    }
}
