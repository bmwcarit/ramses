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

namespace ramses_capu
{
    namespace posix
    {
        const char Console::Colors[6][8] = {
            { 0x1b, 0x5b, 0x31, 0x3b, 0x33, 0x31, 0x6d, 0x00 }, // hex representation of ansi codes for "\e[1;31m" which is RED
            { 0x1b, 0x5b, 0x31, 0x3b, 0x33, 0x34, 0x6d, 0x00 }, // hex representation of ansi codes for "\e[1;34m" which is BLUE
            { 0x1b, 0x5b, 0x31, 0x3b, 0x33, 0x32, 0x6d, 0x00 }, // hex representation of ansi codes for "\e[1;32m" which is GREEN
            { 0x1b, 0x5b, 0x31, 0x3b, 0x33, 0x33, 0x6d, 0x00 }, // hex representation of ansi codes for "\e[1;33m" which is YELLOW
            { 0x1b, 0x5b, 0x31, 0x3b, 0x33, 0x37, 0x6d, 0x00 }, // hex representation of ansi codes for "\e[1;37m" which is WHITE
            { 0x1b, 0x5b, 0x31, 0x3b, 0x33, 0x36, 0x6d, 0x00 }, // hex representation of ansi codes for "\e[1;36m" which is AQUA
        };

        int32_t Console::pipeDescriptorsForInterruption[2] = {-1,-1};
        std::mutex Console::interruptMutex;
    }
}
