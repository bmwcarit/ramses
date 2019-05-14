//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UNIXDOMAINSOCKET_H
#define RAMSES_UNIXDOMAINSOCKET_H


#include "RendererAPI/Types.h"
#include "Collections/String.h"
#include <sys/socket.h>
#include <sys/un.h>

namespace ramses_internal
{
    class UnixDomainSocket
    {
    public:
        UnixDomainSocket(const String& socketFilename, const String& xdgRuntimeDir);
        ~UnixDomainSocket();

        int createBoundFileDescriptor();
        int getBoundFileDescriptor() const;
        int createConnectedFileDescriptor(bool transferOwnership=false);

        void cleanup();

        static bool IsFileDescriptorForValidSocket(int fileDescriptor);

    private:
        bool            checkSocketFilePath() const;
        int             createSocketLockFile() const;
        static int      createAndOpenSocket();
        static int      setCloExecFlagInFD(int fd);
        bool            bindSocketToFile() const;
        bool            connectSocketToFile(int fd) const;
        socklen_t       fillSockaddrForUnixDomain(sockaddr_un& addrToFill) const;

        const String m_xdgRuntimeDir;
        const String m_socketFilename;
        const String m_socketFileLock;
        int          m_socketFileDescriptor = -1;
        int          m_lockFileDescriptor   = -1;
        std::vector<int>  m_connectedFileDescriptors;
    };
}

#endif //RAMSES_UNIXDOMAINSOCKET_H
