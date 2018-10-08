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

#include "ramses-capu/os/Windows/Console.h"

namespace ramses_capu
{
    const uint8_t ramses_capu::os::Console::Colors[] = {
        0x0C, // RED
        0x01, // BLUE
        0x0A, // GREEN
        0x0E, // YELLOW
        0x0F, // WHITE
        0x0B, // AQUA
    };

    HANDLE os::Console::m_event = INVALID_HANDLE_VALUE;
    os::LightweightMutex os::Console::interruptMutex;

    ramses_capu::status_t os::Console::ReadOneCharacter(HANDLE fileHandle, char& buffer)
    {
        char readBuffer;
        DWORD bytesRead = 0;
        const DWORD numberOfBytesToRead = 1;
        const BOOL ret = ReadFile(fileHandle, &readBuffer, numberOfBytesToRead, &bytesRead, NULL);
        if (ret)
        {
            if (0 == bytesRead)
            {
                return CAPU_EOF;
            }
            buffer = readBuffer;
            return CAPU_OK;
        }
        else
        {
            const DWORD error = GetLastError();
            if (ERROR_INVALID_HANDLE == error)
            {
                return CAPU_EOF;
            }
            else
            {
                return CAPU_ERROR;
            }
        }
    }

    HANDLE os::Console::GetInterruptEventHandle()
    {
        return m_event;
    }

    void os::Console::SetEventHandle(HANDLE eventHandle)
    {
        m_event = eventHandle;
    }

    void os::Console::InitializeInterruptEvent()
    {
        HANDLE previouslyCreatedEventHandle = GetInterruptEventHandle();
        if (previouslyCreatedEventHandle == INVALID_HANDLE_VALUE)
        {
            HANDLE createdEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
            SetEventHandle(createdEventHandle);
        }
    }

}
