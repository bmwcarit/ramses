//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_NAMEDPIPE_H
#define RAMSES_NAMEDPIPE_H

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/LogMacros.h"
#include <cassert>

namespace ramses_internal
{
    enum EReadFromPipeStatus
    {
        EReadFromPipeStatus_Success = 0,
        EReadFromPipeStatus_Empty,
        EReadFromPipeStatus_Failure,
        EReadFromPipeStatus_Closed
    };

    class NamedPipe
    {
    public:
        NamedPipe(const String& pipeName, bool createPipe)
            : m_pipeName(pipeName)
            , m_createPipe(createPipe)
            , m_pipeFileDescriptor(-1)
        {
            if(m_createPipe)
            {
                if (::mkfifo(pipeName.c_str(), 0666) != 0)
                {
                    LOG_ERROR(CONTEXT_RENDERER, "NamedPipe::NamedPipe mkfifo for pipe " << pipeName << " failed with errno: " << getSystemErrorStatus() << "!");
                }
            }
        }

        NamedPipe(const NamedPipe&) = delete;
        NamedPipe& operator=(const NamedPipe&) = delete;

        NamedPipe(NamedPipe&&) = delete;
        NamedPipe& operator=(NamedPipe&&) = delete;

        ~NamedPipe()
        {
            LOG_INFO(CONTEXT_RENDERER, "NamedPipe::~NamedPipe closing pipe [name=" << m_pipeName << ", FD=" << m_pipeFileDescriptor << ", createPipe=" << m_createPipe << "]");
            if(m_createPipe)
            {
                ::close(m_pipeFileDescriptor);
                ::unlink(m_pipeName.c_str());
            }
        }

        void open()
        {
            assert(-1 == m_pipeFileDescriptor);
            m_pipeFileDescriptor = ::open(m_pipeName.c_str(), O_RDWR);
            assert(-1 != m_pipeFileDescriptor);

            if (m_pipeFileDescriptor == -1)
            {
                LOG_ERROR(CONTEXT_RENDERER, "NamedPipe::open open failed for pipe " << m_pipeName << " with errno: " << getSystemErrorStatus() << "!");
            }
        }

        bool write(const void* sourcePointer, ssize_t totalBytesToWrite) const
        {
            assert(-1 != m_pipeFileDescriptor);
            ssize_t bytesLeftToWrite = totalBytesToWrite;
            const UInt8* writingLocation = static_cast<const UInt8*>(sourcePointer);

            while(bytesLeftToWrite > 0)
            {
                const ssize_t writtenBytesCount = ::write(m_pipeFileDescriptor, writingLocation, bytesLeftToWrite);

                if(-1 == writtenBytesCount )
                {
                    LOG_ERROR(CONTEXT_RENDERER, "NamedPipe::write write failed for pipe " << m_pipeName << " with errno: " << getSystemErrorStatus() << "!");
                    return false;
                }

                bytesLeftToWrite -= writtenBytesCount ;
                writingLocation += writtenBytesCount ;
            }

            return true;
        }

        EReadFromPipeStatus read(void* destinationPointer, ssize_t totalBytesToRead) const
        {
            assert(-1 != m_pipeFileDescriptor);
            return readExactBytesFromPipe(destinationPointer, totalBytesToRead);
        }

        [[nodiscard]] const String& getName() const
        {
            return m_pipeName;
        }

        static int getSystemErrorStatus()
        {
            return errno;
        }

        bool waitOnData(uint32_t timeoutInMilliSeconds)
        {
            pollfd pfd;
            pfd.fd      = m_pipeFileDescriptor;
            pfd.events  = POLLIN;
            pfd.revents = 0;

            return poll(&pfd, 1, static_cast<int>(timeoutInMilliSeconds)) > 0;
        }

    private:
        EReadFromPipeStatus readExactBytesFromPipe(void *writingLocation, ssize_t totalBytesToRead) const
        {
            ssize_t bytesRemaining = totalBytesToRead;
            uint8_t* currentWritingLocation = static_cast<uint8_t*>(writingLocation);

            while (true)
            {
                const ssize_t bytesRead = ::read(m_pipeFileDescriptor, currentWritingLocation, bytesRemaining);

                if (0 == bytesRead)
                {
                    LOG_ERROR(CONTEXT_RENDERER, "NamedPipe::ReadExactBytesFromPipe read failed for pipe " << m_pipeName << " bytesRead: " << bytesRead << " with errno: " << getSystemErrorStatus() << "!");
                    return EReadFromPipeStatus_Closed;
                }

                if (bytesRead > 0)
                {
                    bytesRemaining -= bytesRead;
                    if (bytesRemaining == 0)
                    {
                        return EReadFromPipeStatus_Success;
                    }
                    else
                    {
                        currentWritingLocation += bytesRead;
                    }
                }
                else
                {
                    // read return negative value in one of two cases:
                    // 1. pipe is empty (happens only iff pipe is non-blocking)
                    // 2. there is (a real) error
                    const int readErrorStatus = getSystemErrorStatus();
                    const bool isPipeEmpty = (readErrorStatus == EAGAIN || readErrorStatus == EWOULDBLOCK);
                    const bool isInTheMiddleOfReadingOperation = totalBytesToRead != bytesRemaining;

                    if(isPipeEmpty)
                    {
                        if(isInTheMiddleOfReadingOperation)
                        {
                            continue;
                        }
                        else
                        {
                            LOG_ERROR(CONTEXT_RENDERER, "NamedPipe::ReadExactBytesFromPipe read failed for pipe " << m_pipeName << " bytesRead: " << bytesRead << " with errno: " << readErrorStatus << "!");
                            return EReadFromPipeStatus_Empty;
                        }

                    }
                    else
                    {
                        LOG_ERROR(CONTEXT_RENDERER, "NamedPipe::ReadExactBytesFromPipe read failed for pipe " << m_pipeName << " bytesRead: " << bytesRead << " with errno: " << readErrorStatus << "!");
                        return EReadFromPipeStatus_Failure;
                    }
                }
            }

            //should never reach here
            assert(false);
            return EReadFromPipeStatus_Failure;
        }

        const String m_pipeName;
        const bool m_createPipe;
        int m_pipeFileDescriptor;
    };

}

#endif
