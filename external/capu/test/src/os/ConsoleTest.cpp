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

#include "ramses-capu/os/Console.h"
#include "gmock/gmock.h"
#include <thread>

namespace ramses_capu
{
    class ConsoleTest : public testing::Test
    {
    };

    // Disabled tests can be forcefully enabled from commandline to test this
    TEST_F(ConsoleTest, DISABLED_ReadChar)
    {
        char readChar = '\0';
        status_t status = Console::ReadChar(readChar);
        printf("I have read: %c\n",readChar);
        EXPECT_EQ(CAPU_OK, status);
        status = Console::ReadChar(readChar);
        printf("I have read: %c\n", readChar);
        EXPECT_EQ(CAPU_OK, status);
        status = Console::ReadChar(readChar);
        printf("I have read: %c\n", readChar);
        EXPECT_EQ(CAPU_OK, status);
        status = Console::ReadChar(readChar);
        printf("I have read: %c\n", readChar);
        EXPECT_EQ(CAPU_OK, status);
        status = Console::ReadChar(readChar);
        printf("I have read: %c\n", readChar);
        EXPECT_EQ(CAPU_OK, status);
    }

    // Disabled tests can be forcefully enabled from commandline to test this
    TEST_F(ConsoleTest, DISABLED_ReadCharFromPreviouslyClosedStdin)
    {
            fclose(stdin);
            char buffer;
            status_t status = Console::ReadChar(buffer);
            EXPECT_NE(CAPU_OK, status);
    }

    // Disabled tests can be forcefully enabled from commandline to test this
    TEST_F(ConsoleTest, DISABLED_ReadChar_Interupt)
    {
        std::thread t([]() {
                          std::this_thread::sleep_for(std::chrono::seconds(1));
                          Console::InterruptReadChar();
                      });
        char buffer;
        status_t status = Console::ReadChar(buffer);
        EXPECT_EQ(CAPU_INTERRUPTED, status);
        t.join();
    }

}
