//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMUDPSOCKET_H
#define RAMSES_PLATFORMUDPSOCKET_H

#include <ramses-capu/os/UdpSocket.h>
#include <PlatformAbstraction/PlatformError.h>
#include <PlatformAbstraction/PlatformTypes.h>

namespace ramses_internal
{
    typedef ramses_capu::SocketAddrInfo SocketAddrInfo;
    typedef ramses_capu::os::SocketDescription SocketDescription;

    class PlatformUdpSocket
    {
    public:
        EStatus bind(UInt16 port, const Char* addr = NULL);
        EStatus send(const Char* buffer, Int32 length, SocketAddrInfo& receiverAddr);
        EStatus send(const Char* buffer, Int32 length, const Char* receiverAddr, UInt16 receiverPort);
        EStatus receive(Char* buffer, Int32 length, Int32& numBytes, SocketAddrInfo* sender);
        EStatus close();
        EStatus setBufferSize(Int32 bufferSize);
        EStatus setTimeout(Int32 timeout);
        EStatus allowBroadcast(Bool broadcast);
        EStatus getBufferSize(Int32& bufferSize);
        EStatus getTimeout(Int32& timeout);
        const SocketAddrInfo& getSocketAddrInfo() const;
        const SocketDescription& getSocketDescription() const;

    protected:
    private:
        ramses_capu::UdpSocket m_socket;
    };

    inline
    EStatus PlatformUdpSocket::bind(UInt16 port, const Char* addr)
    {
        return static_cast<EStatus>(m_socket.bind(port, addr));
    }

    inline
    EStatus PlatformUdpSocket::send(const Char* buffer, Int32 length, SocketAddrInfo& receiverAddr)
    {
        return static_cast<EStatus>(m_socket.send(buffer, length, receiverAddr.addr.c_str(), receiverAddr.port));
    }

    inline
    EStatus PlatformUdpSocket::send(const Char* buffer, Int32 length, const Char* receiverAddr, UInt16 receiverPort)
    {
        return static_cast<EStatus>(m_socket.send(buffer, length, receiverAddr, receiverPort));
    }

    inline
    EStatus PlatformUdpSocket::receive(Char* buffer, Int32 length, Int32& numBytes, SocketAddrInfo* sender)
    {
        return static_cast<EStatus>(m_socket.receive(buffer, length, numBytes, sender));
    }

    inline
    EStatus PlatformUdpSocket::close()
    {
        return static_cast<EStatus>(m_socket.close());
    }

    inline
    EStatus PlatformUdpSocket::setBufferSize(Int32 bufferSize)
    {
        return static_cast<EStatus>(m_socket.setBufferSize(bufferSize));
    }

    inline
    EStatus PlatformUdpSocket::setTimeout(Int32 timeout)
    {
        return static_cast<EStatus>(m_socket.setTimeout(timeout));
    }

    inline
    EStatus PlatformUdpSocket::allowBroadcast(Bool broadcast)
    {
        return static_cast<EStatus>(m_socket.allowBroadcast(broadcast));
    }

    inline
    EStatus PlatformUdpSocket::getBufferSize(Int32& bufferSize)
    {
        return static_cast<EStatus>(m_socket.getBufferSize(bufferSize));
    }

    inline
    EStatus PlatformUdpSocket::getTimeout(Int32& timeout)
    {
        return static_cast<EStatus>(m_socket.getTimeout(timeout));
    }

    inline
    const SocketAddrInfo& PlatformUdpSocket::getSocketAddrInfo() const
    {
        return m_socket.getSocketAddrInfo();
    }

    inline
    const SocketDescription& PlatformUdpSocket::getSocketDescription() const
    {
        return m_socket.getSocketDescription();
    }
}

namespace ramses_capu
{
    template<>
    struct Hash<ramses_internal::SocketAddrInfo>
    {
        uint_t operator()(const ramses_internal::SocketAddrInfo& key)
        {
            return Hash<uint_t>()(static_cast<uint_t>(key.port));
        }
    };
}

#endif
