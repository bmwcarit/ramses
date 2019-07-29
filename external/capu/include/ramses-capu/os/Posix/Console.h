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

#ifndef RAMSES_CAPU_POSIX_CONSOLE_H
#define RAMSES_CAPU_POSIX_CONSOLE_H

#include <ramses-capu/os/Memory.h>
#include <ramses-capu/Error.h>
#include <stdio.h>
#include <mutex>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <termios.h>
#include <errno.h>

namespace ramses_capu
{
    namespace posix
    {
        class Console
        {
        public:
            static bool IsInputAvailable();
            static void Print(const char* format, va_list values);
            static void Print(uint32_t color, const char* format, va_list values);
            static status_t ReadChar(char& buffer);
            static const char Colors[6][8];
            static void Flush();
            static void InterruptReadChar();
        private:
            static int32_t GetReadEndOfPipe();
            static int32_t GetWriteEndOfPipe();
            static void SetReadEndOfPipe(int32_t descriptor);
            static void SetWriteEndOfPipe(int32_t descriptor);
            static int32_t initializePipe();

            static int32_t pipeDescriptorsForInterruption[2];
            static std::mutex interruptMutex;
        };

        inline
        int32_t Console::initializePipe()
        {
            const int32_t writeEndOfPipe = GetWriteEndOfPipe();
            if (writeEndOfPipe == -1)
            {
                int32_t ret = ::pipe(pipeDescriptorsForInterruption);
                return ret;
            }
            return 0;
        }

        inline
        void
        Console::Print(const char* format, va_list values)
        {
            vprintf(format, values);
        }

        inline
        void
        Console::Print(uint32_t color, const char* format, va_list values)
        {
            if (color < 6)
            {
                fputs(ramses_capu::posix::Console::Colors[color], stdout);
                vprintf(format, values);
                const char ca_end[] = { 0x1b, 0x5b, 0x30, 0x6d, 0x00 }; // hex representation of ansi codes for "\e[0m"
                fputs(ca_end, stdout);
            }
            else
            {
                vprintf(format, values);
            }
        }

        inline
        status_t Console::ReadChar(char& buffer)
        {
            struct termios oldTerminalSettings, temporaryWithoutEcho;
            bool ttyConnected = true;

            // save previous settings
            if (0 != tcgetattr(fileno(stdin), &oldTerminalSettings))
            {
                if (errno == EBADF)
                {
                    return CAPU_EOF;
                }
                if (errno == ENOTTY)
                {
                    ttyConnected = false;
                }
            }

            if (ttyConnected)
            {
                // create new settings on top of previous settings
                Memory::Copy(&temporaryWithoutEcho, &oldTerminalSettings, sizeof(struct termios));
                temporaryWithoutEcho.c_lflag &= ~(ECHO | ICANON);
                temporaryWithoutEcho.c_cc[VTIME] = 0;
                temporaryWithoutEcho.c_cc[VMIN] = 1;

                // use new settings
                tcsetattr(fileno(stdin), TCSANOW, &temporaryWithoutEcho);
            }

            ssize_t bytesRead = 0;
            interruptMutex.lock();
            const int32_t ret = initializePipe();
            interruptMutex.unlock();
            status_t status = CAPU_OK;
            if(0 == ret)
            {
                const int32_t stdinHandle = fileno(stdin);
                if (-1 == stdinHandle)
                {
                    return CAPU_ERROR;
                }
                bool hasReadCharacter = false;
                while (status == CAPU_OK && !hasReadCharacter) {
                    fd_set fdset;
                    FD_ZERO(&fdset);
                    FD_SET(GetReadEndOfPipe(), &fdset); // read end of pipe;
                    FD_SET(stdinHandle, &fdset);
                    int32_t highestFileDesciptor = GetReadEndOfPipe();
                    if (stdinHandle > highestFileDesciptor)
                    {
                        highestFileDesciptor = stdinHandle;
                    }
                    const int_t result = select(highestFileDesciptor + 1, &fdset, nullptr, nullptr, nullptr);
                    if (result > 0)
                    {
                        if (FD_ISSET(stdinHandle, &fdset))
                        {
                            char readBuffer;
                            bytesRead = read(stdinHandle, &readBuffer, 1);
                            if (bytesRead <= 0)
                            {
                                status = CAPU_ERROR;
                            }
                            else
                            {
                                buffer = readBuffer;
                                hasReadCharacter = true;
                            }
                        }
                        if (FD_ISSET(GetReadEndOfPipe(), &fdset))
                        {
                            // explicit user interrupt
                            status = CAPU_INTERRUPTED;
                            char readBuffer;
                            const int32_t readStatus = read(GetReadEndOfPipe(), &readBuffer, 1);
                            UNUSED(readStatus);
                        }
                    }
                    else
                    {
                        status = CAPU_ERROR;
                        if (-1 == result && errno == EINTR)
                        {
                            // read another time, if it was only a signal interrupt
                            status = CAPU_OK;
                        }
                    }
                };
            }
            else
            {
                status = CAPU_ERROR;
            }

            if (ttyConnected)
            {
                tcsetattr(fileno(stdin), TCSANOW, &oldTerminalSettings);
            }
            return status;
        }

        inline
        int32_t Console::GetReadEndOfPipe()
        {
            return pipeDescriptorsForInterruption[0];
        }

        inline int32_t Console::GetWriteEndOfPipe()
        {
            return pipeDescriptorsForInterruption[1];
        }

        inline
        void Console::SetReadEndOfPipe(int32_t descriptor)
        {
            pipeDescriptorsForInterruption[0] = descriptor;
        }

        inline
        void Console::SetWriteEndOfPipe(int32_t descriptor)
        {
            pipeDescriptorsForInterruption[1] = descriptor;
        }

        inline
        void
        Console::InterruptReadChar()
        {
            std::lock_guard<std::mutex> l(interruptMutex);
            const int32_t ret = initializePipe();
            if (0 == ret)
            {
                const int32_t writeEndOfPipe = GetWriteEndOfPipe();
                ssize_t result = ::write(writeEndOfPipe,"#",1u);
                UNUSED(result);
            }
        }

        inline
        bool
        Console::IsInputAvailable()
        {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 0;

            int ret = select(STDIN_FILENO + 1, &fds, 0, 0, &tv);
            if (-1 == ret)
            {
                return false;
            }

            return FD_ISSET(STDIN_FILENO, &fds) != 0;
        }

        inline
        void
        Console::Flush()
        {
            fflush(stdout);
            fflush(stderr);
        }

    }
}

#endif //RAMSES_CAPU_POSIX_CONSOLE_H
