//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/PlatformAbstraction/ConsoleInput.h"
#include <mutex>
#include <array>

#ifdef _MSC_VER

#include "internal/PlatformAbstraction/MinimalWindowsH.h"
#include <conio.h>
#include <array>

namespace
{
    class ConsoleInputImpl final
    {
    public:
        bool init()
        {
            // create event for interrupting
            const HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (event == nullptr)
                return false;
            m_event = event;

            // set desired console mode when is real console (character device)
            const HANDLE fileHandle = GetStdHandle(STD_INPUT_HANDLE);
            const DWORD inputType = GetFileType(fileHandle);
            if (inputType == FILE_TYPE_CHAR)
            {
                DWORD consoleMode;
                GetConsoleMode(fileHandle, &consoleMode);
                SetConsoleMode(fileHandle, consoleMode & ~(ENABLE_LINE_INPUT | ENABLE_MOUSE_INPUT));
            }
            return true;
        }

        ~ConsoleInputImpl()
        {
            if (m_event != INVALID_HANDLE_VALUE)
                CloseHandle(m_event);
        }

        bool readChar(char& c)
        {
            const HANDLE fileHandle = GetStdHandle(STD_INPUT_HANDLE);
            const DWORD inputType = GetFileType(fileHandle);

            while (true)
            {
                if (inputType == FILE_TYPE_PIPE)
                {
                    // pipe is not handleable by WaitForMultipleObjects, must use polling
                    DWORD bytesAvailable = 0;
                    DWORD peekStat = PeekNamedPipe(fileHandle, nullptr, 0, nullptr, &bytesAvailable, nullptr);
                    if (peekStat != 0 && bytesAvailable > 0)
                        return ReadOneCharacter(fileHandle, c);
                    else if (peekStat == 0)
                        return false;

                    // block 500ms on interrupt event
                    if (WaitForSingleObject(m_event, 500) == WAIT_OBJECT_0)
                        return false;
                }
                else
                {
                    std::array<HANDLE, 2> handles = {fileHandle, m_event};
                    const DWORD ret = WaitForMultipleObjects(static_cast<DWORD>(handles.size()), handles.data(), false, INFINITE);
                    if (ret == WAIT_OBJECT_0 + 1)
                    {
                        // interrupt event
                        return false;
                    }
                    else if (ret == WAIT_OBJECT_0 + 0)
                    {
                        // stdin event
                        // read from real console
                        if (inputType == FILE_TYPE_CHAR)
                        {
                            // read special INPUT_RECORD and check if is keydown event
                            INPUT_RECORD inputRecord;
                            ZeroMemory(&inputRecord, sizeof(INPUT_RECORD));
                            DWORD numberOfReadEvents = 0;
                            if (ReadConsoleInput(fileHandle, &inputRecord, 1, &numberOfReadEvents) == 0)
                                return false;
                            if (numberOfReadEvents != 1)
                                return false;

                            // ignore all other input types and try again next loop
                            if (inputRecord.EventType == KEY_EVENT &&   // from keyboard
                                inputRecord.Event.KeyEvent.bKeyDown &&   // key pressed
                                inputRecord.Event.KeyEvent.uChar.AsciiChar) // no control key (e.g. shift)
                            {
                                c = inputRecord.Event.KeyEvent.uChar.AsciiChar;
                                return true;
                            }
                        }
                        else if (inputType == FILE_TYPE_DISK)
                        {
                            return ReadOneCharacter(fileHandle, c);
                        }
                        else
                        {
                            // unknown type
                            return false;
                        }
                    }
                    else
                    {
                        // something failed
                        return false;
                    }
                }
            }
        }

        void interruptReadChar()
        {
            SetEvent(m_event);
        }

    private:
        bool ReadOneCharacter(HANDLE fileHandle, char& buffer)
        {
            DWORD bytesRead = 0;
            if (!ReadFile(fileHandle, &buffer, 1, &bytesRead, nullptr))
                return false;
            if (bytesRead == 0)
                return false;  // eof
            return true;
        }

        HANDLE m_event = INVALID_HANDLE_VALUE;
    };
}

#else

#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <algorithm>

namespace
{
    class ConsoleInputImpl final
    {
    public:
        bool init()
        {
            // select with zero timeout on stdin to check if fd is valid and open
            const int stdinFd = ::fileno(stdin);
            fd_set fdset;
            FD_ZERO(&fdset);  // NOLINT macro does unsafe stuff inside
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            FD_SET(stdinFd, &fdset);
            struct timeval timeout = {0, 0};
            if (::select(stdinFd + 1, &fdset, nullptr, nullptr, &timeout) < 0)
                return false;

            // create interrupt pipes
            std::array<int, 2> tmpPipes{};
            if (::pipe(tmpPipes.data()) != 0)
                return false;
            m_readPipe = tmpPipes[0];
            m_WritePipe = tmpPipes[1];

            m_ttyConnected = true;
            // get current tty settings for recovery
            if (::tcgetattr(stdinFd, &m_oldTerminalSettings) != 0)
            {
                if (errno == ENOTTY)
                {
                    m_ttyConnected = false;
                }
                else
                {
                    // filedescriptor closed or broken
                    return false;
                }
            }
            return true;
        }

        ~ConsoleInputImpl()
        {
            if (m_ttyConnected)
            {
                // restore terminal settings
                const int stdinFd = ::fileno(stdin);
                if (stdinFd != -1)
                {
                    tcsetattr(stdinFd, TCSANOW, &m_oldTerminalSettings);
                }
            }
            if (m_readPipe != 0)
                ::close(m_readPipe);
            if (m_WritePipe != 0)
                ::close(m_WritePipe);
        }

        bool readChar(char& c)
        {
            const int stdinFd = ::fileno(stdin);
            if (stdinFd == -1)
                return false;

            if (m_ttyConnected)
            {
                // disable echo
                struct termios temporaryWithoutEcho{};
                std::memcpy(&temporaryWithoutEcho, &m_oldTerminalSettings, sizeof(struct termios));
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                temporaryWithoutEcho.c_lflag &= ~(ECHO | ICANON);
                temporaryWithoutEcho.c_cc[VTIME] = 0;
                temporaryWithoutEcho.c_cc[VMIN] = 1;
                tcsetattr(stdinFd, TCSANOW, &temporaryWithoutEcho);
            }

            bool ok = false;
            while (true)
            {
                // select infinitly on stdin and pipe
                fd_set fdset;
                FD_ZERO(&fdset);  // NOLINT macro does unsafe stuff inside
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                FD_SET(m_readPipe, &fdset); // read end of pipe;
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                FD_SET(stdinFd, &fdset);
                if (::select(std::max(stdinFd, m_readPipe) + 1, &fdset, nullptr, nullptr, nullptr) > 0)
                {
                    // check which fd triggered
                    // NOLINTNEXTLINE(hicpp-signed-bitwise)
                    if (FD_ISSET(stdinFd, &fdset))
                    {
                        if (::read(stdinFd, &c, 1) == 1)
                            ok = true;
                    }
                    // NOLINTNEXTLINE(hicpp-signed-bitwise)
                    if (FD_ISSET(m_readPipe, &fdset))
                    {
                        // user interrupt
                        char readBuffer = 0;
                        // no need to check result, failing anyway
                        // must use assignment here because gcc is broken and does not care: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=25509
                        const auto dummy = ::read(m_readPipe, &readBuffer, 1);
                        (void)dummy;
                    }
                    break;
                }
                // ignore interrupted by signal, otherwise fail
                if (errno != EINTR)
                    break;
            }

            if (m_ttyConnected)
            {
                // restore terminal settings
                tcsetattr(stdinFd, TCSANOW, &m_oldTerminalSettings);
            }

            return ok;
        }

        void interruptReadChar()
        {
            // write to pipe (ingore errors, no way to handle them properly)
            // must use assignment here because gcc is broken and does not care: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=25509
            const auto dummy = ::write(m_WritePipe, "R", 1);
            (void)dummy;
        }

    private:
        int m_readPipe = 0;
        int m_WritePipe = 0;
        struct termios m_oldTerminalSettings{};
        bool m_ttyConnected = false;
    };
}
#endif

    namespace ramses::internal
{
    namespace
    {
        std::mutex implMutex;
        std::unique_ptr<ConsoleInputImpl> impl;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static): design decision
    bool ConsoleInput::readChar(char& c)
    {
        return impl->readChar(c);
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static): design decision
    void ConsoleInput::interruptReadChar()
    {
        impl->interruptReadChar();
    }

    ConsoleInput::ConsoleInput() = default;

    ConsoleInput::~ConsoleInput()
    {
        std::lock_guard<std::mutex> g(implMutex);
        impl.reset();
    }

    std::unique_ptr<ConsoleInput> ConsoleInput::TryGetUniqueConsoleInput()
    {
        std::lock_guard<std::mutex> g(implMutex);
        if (impl)
            return nullptr;
        std::unique_ptr<ConsoleInputImpl> tmp(new ConsoleInputImpl);
        if (!tmp->init())
            return nullptr;
        impl = std::move(tmp);
        return std::unique_ptr<ConsoleInput>{new ConsoleInput};
    }
}
