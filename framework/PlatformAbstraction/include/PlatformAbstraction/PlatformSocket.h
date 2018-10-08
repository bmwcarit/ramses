//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMABSTRACTION_PLATFORMSOCKET_H
#define RAMSES_PLATFORMABSTRACTION_PLATFORMSOCKET_H

#include <ramses-capu/os/TcpSocket.h>
#include <PlatformAbstraction/PlatformTypes.h>
#include <PlatformAbstraction/PlatformError.h>

namespace ramses_internal
{
    typedef ramses_capu::os::SocketDescription SocketDescription;

    class PlatformSocket
    {
    public:
        friend class SocketOutputStream;
        friend class TcpSocketOutputStream;
        friend class TcpSocketInputStream;

        PlatformSocket();
        explicit PlatformSocket(ramses_capu::TcpSocket* socket);

        ~PlatformSocket();

        EStatus send(const Char* buffer, Int32 length, Int32& bytes);
        EStatus receive(Char* buffer, Int32 length, Int32& numBytes);
        EStatus close();
        EStatus connect(const Char* dest_addr, UInt16 port, int32_t timeoutMs = 0);
        EStatus setBufferSize(Int32 bufferSize);
        EStatus setNoDelay(Bool noDelay);
        EStatus setKeepAlive(Bool keepAlive);
        EStatus setTimeout(Int32 timeout);
        EStatus getBufferSize(Int32& bufferSize);
        EStatus getNoDelay(Bool& noDelay);
        EStatus getKeepAlive(Bool& keepAlive);
        EStatus getTimeout(Int32& timeout);
        const SocketDescription& getSocketDescription() const;
    protected:
    private:
        ramses_capu::TcpSocket* mSocket;

        PlatformSocket(const PlatformSocket& platformSocket);
    };

    inline
    PlatformSocket::PlatformSocket()
        : mSocket(new ramses_capu::TcpSocket())
    {
    }

    inline
    PlatformSocket::PlatformSocket(ramses_capu::TcpSocket* socket)
    {
        mSocket = socket;
    }

    inline
    PlatformSocket::~PlatformSocket()
    {
        if (mSocket)
        {
            close();
            delete mSocket;
        }
    }

    inline
    EStatus
    PlatformSocket::send(const Char* buffer, Int32 length, Int32& bytes)
    {
        return static_cast<EStatus>(mSocket->send(buffer, length, bytes));
    }

    inline
    EStatus
    PlatformSocket::receive(Char* buffer, Int32 length, Int32& numBytes)
    {
        return static_cast<EStatus>(mSocket->receive(buffer, length, numBytes));
    }

    inline
    EStatus
    PlatformSocket::close()
    {
        return static_cast<EStatus>(mSocket->close());
    }

    inline
    EStatus
    PlatformSocket::connect(const Char* dest_addr, UInt16 port, int32_t timeoutMs)
    {
        return static_cast<EStatus>(mSocket->connect(dest_addr, port, timeoutMs));
    }

    inline
    EStatus
    PlatformSocket::setBufferSize(Int32 bufferSize)
    {
        return static_cast<EStatus>(mSocket->setBufferSize(bufferSize));
    }

    inline
    EStatus
    PlatformSocket::setNoDelay(Bool noDelay)
    {
        return static_cast<EStatus>(mSocket->setNoDelay(noDelay));
    }

    inline
    EStatus
    PlatformSocket::setKeepAlive(Bool keepAlive)
    {
        return static_cast<EStatus>(mSocket->setKeepAlive(keepAlive));
    }

    inline
    EStatus
    PlatformSocket::setTimeout(Int32 timeout)
    {
        return static_cast<EStatus>(mSocket->setTimeout(timeout));
    }

    inline
    EStatus
    PlatformSocket::getBufferSize(Int32& bufferSize)
    {
        return static_cast<EStatus>(mSocket->getBufferSize(bufferSize));
    }

    inline
    EStatus
    PlatformSocket::getNoDelay(Bool& noDelay)
    {
        return static_cast<EStatus>(mSocket->getNoDelay(noDelay));
    }

    inline
    EStatus
    PlatformSocket::getKeepAlive(Bool& keepAlive)
    {
        return static_cast<EStatus>(mSocket->getKeepAlive(keepAlive));
    }

    inline
    EStatus
    PlatformSocket::getTimeout(Int32& timeout)
    {
        return static_cast<EStatus>(mSocket->getTimeout(timeout));
    }

    inline
    const SocketDescription&
    PlatformSocket::getSocketDescription() const
    {
        return mSocket->getSocketDescription();
    }
}

#endif
