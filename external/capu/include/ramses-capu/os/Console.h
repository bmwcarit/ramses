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

#ifndef RAMSES_CAPU_CONSOLE_H
#define RAMSES_CAPU_CONSOLE_H

#include <ramses-capu/Config.h>

#include <ramses-capu/os/PlatformInclude.h>
#include RAMSES_CAPU_PLATFORM_INCLUDE(Console)


namespace ramses_capu
{
    /**
     * Allows platform independent handling of console IO
     */
    class Console
    {
    public:
        /**
         * Reads a single char from stdin
         * @param buffer on success a character is read into provided variable
         * @return CAPU_OK on success,
         *         CAPU_INTERRUPT on interrupt,
         *         CAPU_EOF if stdin is closed,
         *         CAPU_ERROR when other errors ocurred
         */
        static status_t ReadChar(char& buffer);

        /**
         * Interrupts a console read (call to Console::ReadChar)
         *
         * Calling InterruptReadChar before ReadChar will cause the next call to ReadChar not to block.
         * If another call is currently in the blocking read, that will abort and give back -1.
         * Calling InterruptReadChar from other threads than ReadChar is safe.
         */
        static void InterruptReadChar();
    };

    inline
    status_t
    Console::ReadChar(char& buffer)
    {
        return ramses_capu::os::Console::ReadChar(buffer);
    }

    inline
    void
    Console::InterruptReadChar()
    {
        ramses_capu::os::Console::InterruptReadChar();
    }
}

#endif // RAMSES_CAPU_CONSOLE_H
