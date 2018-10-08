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

#include <gtest/gtest.h>
#include "ramses-capu/container/Pair.h"
#include "ramses-capu/Error.h"
#include "ramses-capu/Config.h"


TEST(Pair, DefaultInitialized)
{
    ramses_capu::Pair<int, int> p;

    EXPECT_EQ(0, p.first);
    EXPECT_EQ(0, p.second);
}

TEST(Pair, Functionality)
{
    ramses_capu::Pair<int, int> p(1, 2);

    EXPECT_EQ(p.first, 1);
    EXPECT_EQ(p.second, 2);
}
