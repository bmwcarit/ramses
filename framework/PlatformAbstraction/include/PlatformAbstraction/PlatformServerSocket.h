//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMSERVERSOCKET_H
#define RAMSES_PLATFORMSERVERSOCKET_H

#include <ramses-capu/os/TcpServerSocket.h>
#include <PlatformAbstraction/PlatformError.h>
#include <PlatformAbstraction/PlatformSocket.h>

namespace ramses_internal
{
    class PlatformServerSocket
    {
    public:
        PlatformServerSocket();
        ~PlatformServerSocket();

        PlatformSocket* accept(UInt32 timeout = 0);
        EStatus close();
        EStatus bind(UInt16 port, const char* addr = nullptr);
        EStatus listen(UInt8 backlog);
        UInt16 getPort();
        const SocketDescription& getSocketDescription() const;

    private:
        ramses_capu::TcpServerSocket mServerSocket;
    };

    inline
    PlatformServerSocket::PlatformServerSocket()
    {
    }

    inline
    PlatformServerSocket::~PlatformServerSocket()
    {
    }

    inline
    PlatformSocket*
    PlatformServerSocket::accept(UInt32 timeout)
    {
        ramses_capu::TcpSocket* socket = mServerSocket.accept(timeout);
        if (socket)
        {
            return new PlatformSocket(socket);
        }
        return 0;
    }

    inline
    EStatus
    PlatformServerSocket::close()
    {
        return static_cast<EStatus>(mServerSocket.close());
    }

    inline
    EStatus
    PlatformServerSocket::bind(UInt16 port, const Char* addr)
    {
        return static_cast<EStatus>(mServerSocket.bind(port, addr));
    }

    inline
    EStatus
    PlatformServerSocket::listen(UInt8 backlog)
    {
        return static_cast<EStatus>(mServerSocket.listen(backlog));
    }

    inline
    const SocketDescription&
    PlatformServerSocket::getSocketDescription() const
    {
        return mServerSocket.getSocketDescription();
    }

    inline
    UInt16 PlatformServerSocket::getPort()
    {
        return mServerSocket.port();
    }
}

#endif
