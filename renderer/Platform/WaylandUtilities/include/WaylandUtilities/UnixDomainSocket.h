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
#include <sys/socket.h>
#include <sys/un.h>

#include <string>

namespace ramses_internal
{
    class UnixDomainSocket
    {
    public:
        UnixDomainSocket(const std::string& socketFilename, const std::string& xdgRuntimeDir);
        ~UnixDomainSocket();

        int createBoundFileDescriptor();
        [[nodiscard]] int getBoundFileDescriptor() const;
        int createConnectedFileDescriptor(bool transferOwnership=false);

        void cleanup();

        static bool IsFileDescriptorForValidSocket(int fileDescriptor);

    private:
        [[nodiscard]] bool            checkSocketFilePath() const;
        [[nodiscard]] int             createSocketLockFile() const;
        static int      createAndOpenSocket();
        static int      setCloExecFlagInFD(int socketFileDescriptor);
        [[nodiscard]] bool            bindSocketToFile() const;
        [[nodiscard]] bool            connectSocketToFile(int socketFileDescriptor) const;
        socklen_t       fillSockaddrForUnixDomain(sockaddr_un& addrToFill) const;

        const std::string m_xdgRuntimeDir;
        const std::string m_socketFilename;
        const std::string m_socketFileLock;
        int               m_socketFileDescriptor = -1;
        int               m_lockFileDescriptor   = -1;
        std::vector<int>  m_connectedFileDescriptors;
    };
}

#endif //RAMSES_UNIXDOMAINSOCKET_H
