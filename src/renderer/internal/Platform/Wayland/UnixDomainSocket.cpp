//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/UnixDomainSocket.h"
#include "internal/PlatformAbstraction/PlatformStringUtils.h"
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <cassert>
#include <utility>

namespace ramses::internal
{
    UnixDomainSocket::UnixDomainSocket(const std::string& _socketFilename, std::string xdgRuntimeDir)
    : m_xdgRuntimeDir(std::move(xdgRuntimeDir))
    , m_socketFilename(m_xdgRuntimeDir + "/" + _socketFilename)
    , m_socketFileLock(m_socketFilename + ".lock")
    {
    }


    UnixDomainSocket::~UnixDomainSocket()
    {
        cleanup();
    }

    int UnixDomainSocket::createBoundFileDescriptor()
    {
        assert(m_socketFileDescriptor < 0);

        if(!checkSocketFilePath())
        {
            return -1;
        }

        m_lockFileDescriptor = createSocketLockFile();
        if(m_lockFileDescriptor < 0)
        {
            return -1;
        }

        m_socketFileDescriptor = createAndOpenSocket();
        if( m_socketFileDescriptor < 0 )
        {
            cleanup();
            return -1;
        }

        if(!bindSocketToFile())
        {
            cleanup();
            return -1;
        }

        return m_socketFileDescriptor;
    }

    int UnixDomainSocket::getBoundFileDescriptor() const
    {
        return m_socketFileDescriptor;
    }

    int UnixDomainSocket::createConnectedFileDescriptor(bool transferOwnership)
    {
        int fileDescriptor = createAndOpenSocket();
        if(fileDescriptor < 0)
        {
            return -1;
        }

        if(!connectSocketToFile(fileDescriptor))
        {
            close(fileDescriptor);
            return -1;
        }

        if(!transferOwnership)
        {
            m_connectedFileDescriptors.push_back(fileDescriptor);
        }
        return fileDescriptor;
    }

    void UnixDomainSocket::cleanup()
    {
        if(m_socketFileDescriptor >= 0)
        {
            close(m_socketFileDescriptor);
            m_socketFileDescriptor = -1;
            unlink(m_socketFilename.c_str());
        }

        if(m_lockFileDescriptor >= 0)
        {
            close(m_lockFileDescriptor);
            m_socketFileDescriptor = -1;
            unlink(m_socketFileLock.c_str());
        }

        for( auto fileDescriptor : m_connectedFileDescriptors )
        {
            close(fileDescriptor);
        }
        m_connectedFileDescriptors.clear();

    }

    bool UnixDomainSocket::IsFileDescriptorForValidSocket(int fileDescriptor)
    {
        if (fileDescriptor < 0)
        {
            return false;
        }

        struct stat buf{};
        if (fstat(fileDescriptor, &buf) < 0)
        {
            return false;
        }

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        return S_ISSOCK(buf.st_mode);
    }

    bool UnixDomainSocket::checkSocketFilePath() const
    {
        struct stat socket_stat{};
        if (stat(m_socketFilename.c_str(), &socket_stat) < 0 )
        {
            if (errno != ENOENT)
            {
                return false;
            }
        }
        return true;
    }

    int UnixDomainSocket::createSocketLockFile() const
    {
        // create lock file, with close-on-exec and usr and grp RW rights
        // NOLINTNEXTLINE(hicpp-signed-bitwise, cppcoreguidelines-pro-type-vararg)
        int lockFileFileDescriptor = open(m_socketFileLock.c_str(), O_CREAT | O_CLOEXEC, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));

        if (lockFileFileDescriptor < 0)
        {
            return -1;
        }

        // lock the file
        if (flock(lockFileFileDescriptor, LOCK_EX | LOCK_NB) < 0)
        {
            close(lockFileFileDescriptor);
            return -1;
        }

        return lockFileFileDescriptor;
    }

    int UnixDomainSocket::createAndOpenSocket()
    {
        // create socket with close-on-exec flag
        int socketFileDescriptor = socket(PF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);

        if (socketFileDescriptor < 0)
        {
            if( errno != EINVAL )
            {
                return -1;
            }

            // if the socket couldn't be created try it again and
            // set the close-on-exec flag later
            socketFileDescriptor = socket(PF_LOCAL, SOCK_STREAM, 0);

            if (socketFileDescriptor < 0)
            {
                return -1;
            }

            if (setCloExecFlagInFD(socketFileDescriptor) < 0)
            {
                close(socketFileDescriptor);
                return -1;
            }
        }

        int enable = 1;
        if (setsockopt(socketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        {
            close(socketFileDescriptor);
            return -1;
        }

        return socketFileDescriptor;
    }

    int UnixDomainSocket::setCloExecFlagInFD(int socketFileDescriptor)
    {
        assert(socketFileDescriptor >= 0);

        // get the file descriptor flags
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg): false positive
        const auto flags = fcntl(socketFileDescriptor, F_GETFD);
        if (flags == -1)
        {
            return -1;
        }

        // now add the close-on-exec flag to the file descriptor's flags
        // NOLINTNEXTLINE(hicpp-signed-bitwise, cppcoreguidelines-pro-type-vararg)
        if (fcntl(socketFileDescriptor, F_SETFD, flags | FD_CLOEXEC) == -1)
        {
            return -1;
        }
        return socketFileDescriptor;
    }

    bool UnixDomainSocket::bindSocketToFile() const
    {
        sockaddr_un addr{};

        const size_t sizeOfSockAddr = fillSockaddrForUnixDomain(addr);
        if (sizeOfSockAddr == 0)
        {
            return false;
        }

        // bind and listen to the socket file
        if (bind(m_socketFileDescriptor, reinterpret_cast<sockaddr*>(&addr), sizeOfSockAddr) < 0)
        {
            return false;
        }

        const int NumberOfListenedConnections = 128;
        if (listen(m_socketFileDescriptor, NumberOfListenedConnections) < 0)
        {
            return false;
        }

        return true;
    }

    socklen_t UnixDomainSocket::fillSockaddrForUnixDomain(sockaddr_un& addrToFill) const
    {
        // set to AF_LOCAL because we want to
        // use a Unix domain socket
        addrToFill.sun_family = AF_LOCAL;

        // write the socket file path to addr and check whether we have no overflow
        PlatformStringUtils::Copy(addrToFill.sun_path, sizeof(addrToFill.sun_path), m_socketFilename.c_str());
        const size_t sizeOfSocketFilePath = std::strlen(addrToFill.sun_path) + 1;
        assert(sizeOfSocketFilePath > 1);
        if (sizeOfSocketFilePath > sizeof(addrToFill.sun_path))
        {
            return 0;
        }

        // return the used size of the sockaddr_un struct
        return offsetof(sockaddr_un, sun_path) + sizeOfSocketFilePath;
    }

    bool UnixDomainSocket::connectSocketToFile(int socketFileDescriptor) const
    {
        sockaddr_un addr{};

        const size_t sizeOfSockAddr = fillSockaddrForUnixDomain(addr);
        if (sizeOfSockAddr == 0)
        {
            return false;
        }

        if (connect(socketFileDescriptor, reinterpret_cast<sockaddr*>(&addr), sizeOfSockAddr) < 0)
        {
            return false;
        }

        return true;
    }
}
