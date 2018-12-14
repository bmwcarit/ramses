//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UNIXDOMAINSOCKETHELPER_H
#define RAMSES_UNIXDOMAINSOCKETHELPER_H


#include "RendererAPI/Types.h"
#include "Collections/String.h"
#include <sys/socket.h>
#include <sys/un.h>

namespace ramses_internal
{
    class UnixDomainSocketHelper
    {
    public:
        UnixDomainSocketHelper(const String& socketFilename);
        //TODO Mohamed: clean this up
        UnixDomainSocketHelper(const String& socketFilename, const String& xdgRuntimeDir);
        ~UnixDomainSocketHelper();

        int createBoundFileDescriptor();
        int getBoundFileDescriptor() const;
        int createConnectedFileDescriptor(Bool transferOwnership=false);

        void cleanup();
    private:

        //TODO Mohamed: clean this up
        const String m_xdgRuntimeDir        = getenv("XDG_RUNTIME_DIR");
        const String m_socketFilename;
        const String m_socketFileLock;
        int          m_socketFileDescriptor = -1;
        int          m_lockFileDescriptor   = -1;
        Vector<int>  m_connectedFileDescriptors;
    };

    namespace SocketHelperFunction
    {
        Bool      checkSocketFilePath(const String& socketFilename);
        int       createSocketLockFile(const String& socketLockFile);
        int       createAndOpenSocket();
        int       setCloExecFlagInFD(int fd);
        Bool      bindSocketToFile(int fd, const String& socketName);
        Bool      connectSocketToFile(int fd, const String& socketName);
        socklen_t fillSockaddrForUnixDomain(sockaddr_un& addrToFill, const String& socketName);
    }


}

#endif //RAMSES_UNIXDOMAINSOCKETHELPER_H
