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

#ifndef RAMSES_CAPU_WINDOWS_CONSOLE_H
#define RAMSES_CAPU_WINDOWS_CONSOLE_H

#include "ramses-capu/os/Windows/MinimalWindowsH.h"
#include "ramses-capu/Error.h"
#include <conio.h>
#include <mutex>

namespace ramses_capu
{
    namespace os
    {
        class Console
        {
        public:
            static bool IsInputAvailable();
            static void Print(const char* format, va_list values);
            static void Print(uint32_t color, const char* format, va_list values);
            static status_t ReadChar(char& buffer);
            static void Flush();
            static void InterruptReadChar();

        private:
            static void InitializeInterruptEvent();
            static status_t ReadOneCharacter(HANDLE fileHandle, char& buffer);
            static void SetEventHandle(HANDLE eventHandle);
            static HANDLE GetInterruptEventHandle();

            static HANDLE m_event;
            static const uint8_t Colors[];
            static std::mutex interruptMutex;
        };

        inline
        bool
        Console::IsInputAvailable()
        {
            HANDLE inHandle = GetStdHandle(STD_INPUT_HANDLE);
            switch (GetFileType(inHandle))
            {
                case FILE_TYPE_CHAR:
                {
                    return _kbhit() != 0;
                }
                case FILE_TYPE_DISK:
                case FILE_TYPE_PIPE:
                {
                    const uint32_t zeroTimeOut = 0u;
                    DWORD ret = WaitForSingleObject(inHandle, zeroTimeOut);
                    return (WAIT_OBJECT_0 == ret);
                }
            };
            return false;
        }

        inline
        void Console::Print(const char* format, va_list values)
        {
            vprintf(format, values);
        }

        inline
        void Console::Print(uint32_t color,  const char* format, va_list values)
        {
            uint8_t colorCode = Colors[color];

            HANDLE hstdout = GetStdHandle( STD_OUTPUT_HANDLE );

            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo( hstdout, &csbi );
            SetConsoleTextAttribute( hstdout, colorCode );

            Console::Print(format, values);

            SetConsoleTextAttribute( hstdout, csbi.wAttributes);
        }

        inline
        status_t Console::ReadChar(char& buffer)
        {
            const HANDLE fileHandle = GetStdHandle(STD_INPUT_HANDLE);
            {
                std::lock_guard<std::mutex> l(interruptMutex);
                InitializeInterruptEvent();
            }

            DWORD previousConsoleMode;

            const DWORD inputType = GetFileType(fileHandle);
            if (inputType == FILE_TYPE_CHAR)
            {
                GetConsoleMode(fileHandle, &previousConsoleMode);
                SetConsoleMode(fileHandle, previousConsoleMode & ~(ENABLE_LINE_INPUT | ENABLE_MOUSE_INPUT));
            };

            const int_t STDIN_INDEX = 0;
            const int_t INTERRUPT_EVENT_INDEX = 1;

            HANDLE handles[2];
            handles[STDIN_INDEX] = fileHandle;
            handles[INTERRUPT_EVENT_INDEX] = GetInterruptEventHandle();
            const DWORD numberOfObjectsToWaitOn = 2;

            status_t status = CAPU_OK;
            bool haveReadCharacter = false;
            while (CAPU_OK == status && !haveReadCharacter)
            {
                if (inputType == FILE_TYPE_PIPE)
                {
                    // pipe is not handleable by WaitForMultipleObjects, must be handled separately
                    DWORD bytesAvailable = 0;
                    DWORD peekStat = PeekNamedPipe(fileHandle, NULL, 0, NULL, &bytesAvailable, NULL);
                    if (peekStat != 0 && bytesAvailable > 0)
                    {
                        status = ReadOneCharacter(fileHandle, buffer);
                        if (CAPU_OK == status)
                        {
                            haveReadCharacter = true;
                        }
                    }
                    else
                    {
                        const DWORD ret = WaitForSingleObject(GetInterruptEventHandle(), 500);
                        if (ret == WAIT_OBJECT_0)
                        {
                            status = CAPU_INTERRUPTED;
                        }
                    }
                }
                else
                {
                    const DWORD ret = WaitForMultipleObjects(numberOfObjectsToWaitOn, handles, false, INFINITE);
                    if (ret == WAIT_OBJECT_0 + INTERRUPT_EVENT_INDEX)
                    {
                        status = CAPU_INTERRUPTED;
                    }
                    else if (ret == WAIT_OBJECT_0 + STDIN_INDEX)
                    {
                        if (inputType == FILE_TYPE_CHAR)
                        {
                            const uint32_t numberOfInputEventsToRead = 1u;
                            DWORD numberOfReadEvents = 0;
                            INPUT_RECORD inputRecord;
                            ZeroMemory(&inputRecord, sizeof(INPUT_RECORD));
                            const BOOL peekStatus = PeekConsoleInput(fileHandle, &inputRecord, numberOfInputEventsToRead, &numberOfReadEvents);
                            if (peekStatus)
                            {
                                if (numberOfReadEvents > 0)
                                {
                                    if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown)
                                    {
                                        // next available event is keyboard event
                                        status = ReadOneCharacter(fileHandle, buffer);
                                        if (CAPU_OK == status)
                                        {
                                            haveReadCharacter = true;
                                        }
                                    }
                                    else
                                    {
                                        // prune non keyboard event
                                        ReadConsoleInput(fileHandle, &inputRecord, numberOfInputEventsToRead, &numberOfReadEvents);
                                    }
                                }
                            }
                            else
                            {
                                const DWORD error = GetLastError();
                                if (ERROR_INVALID_HANDLE == error)
                                {
                                    status = CAPU_EOF;
                                }
                                else
                                {
                                    status = CAPU_ERROR;
                                }
                            }
                        }
                        else if (inputType == FILE_TYPE_DISK)
                        {
                            status = ReadOneCharacter(fileHandle, buffer);
                            if (CAPU_OK == status)
                            {
                                haveReadCharacter = true;
                            }
                        }
                    }
                    else if (ret == WAIT_FAILED)
                    {
                        const DWORD error = GetLastError();
                        if (ERROR_INVALID_HANDLE == error)
                        {
                            status = CAPU_EOF;
                        }
                        else
                        {
                            status = CAPU_ERROR;
                        }
                    }
                }
            }
            return status;
        }

        inline
        void Console::Flush()
        {
            fflush(stdout);
            fflush(stderr);
        }

        inline
        void Console::InterruptReadChar()
        {
            std::lock_guard<std::mutex> l(interruptMutex);
            InitializeInterruptEvent();
            SetEvent(GetInterruptEventHandle());
        }
    }
}

#endif //RAMSES_CAPU_WINDOWS_CONSOLE_H
