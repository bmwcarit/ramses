//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportTCP/SocketManager.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    SocketManager::SocketManager()
    {
        do
        {
        } while (EStatus_RAMSES_OK != m_interruptSocket.bind(0, "127.0.0.1"));

        m_socketinfos.push_back(SocketInfoPair(m_interruptSocket.getSocketDescription(), SocketDelegate::Create<SocketManager, &SocketManager::waitCallInterrupted>(*this)));
    }

    SocketManager::~SocketManager()
    {
        exit();
    }

    void SocketManager::exit()
    {
        removeSocketInfoPair(m_interruptSocket.getSocketDescription());
        m_interruptSocket.close();
    }


    void SocketManager::waitCallInterrupted(const SocketDescription& socketDescription)
    {
        UNUSED(socketDescription);
        UInt8 data = 0;
        Int32 numBytes = 0;

        EStatus status = m_interruptSocket.receive(reinterpret_cast<Char*>(&data), sizeof(data), numBytes, 0);
        if (status != EStatus_RAMSES_OK)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "SocketManager::waitCallInterrupted: receive failed with " << status);
        }
        if (0xde != data)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "Problem during read on interrupt socket");
        }
    }

    void SocketManager::interruptWaitCall()
    {
        UInt8 data = 0xde;
        SocketAddrInfo info = m_interruptSocket.getSocketAddrInfo();
        EStatus status = m_interruptSocket.send(reinterpret_cast<Char*>(&data), sizeof(data), info);
        if (status != EStatus_RAMSES_OK)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "SocketManager::interruptWaitCall: failed with " << status);
        }
    }

    void SocketManager::trackSocket(PlatformSocket* socket, const TcpSocketDelegate& delegate)
    {
        trackSocketInternal(m_tcpSocketMap, TcpSocketDelegatePair(socket, delegate), SocketDelegate::Create<SocketManager, &SocketManager::tcpSocketCallback>(*this));
    }

    void SocketManager::trackSocket(PlatformServerSocket* socket, const ServerSocketDelegate& delegate)
    {
        trackSocketInternal(m_serverSocketMap, ServerSocketDelegatePair(socket, delegate), SocketDelegate::Create<SocketManager, &SocketManager::serverSocketCallback>(*this));
    }

    void SocketManager::checkAllSockets(Bool blocking, UInt32 timeout)
    {
        SocketInfoVector copy = m_socketinfos;
        if (blocking)
        {
            NonBlockingSocketChecker::CheckSocketsForIncomingData(copy);
        }
        else
        {
            NonBlockingSocketChecker::CheckSocketsForIncomingData(copy, timeout);
        }
    }

    void SocketManager::removeSocketInfoPair(const SocketDescription& socketDescription)
    {
        SocketInfoVector::Iterator current = m_socketinfos.begin();
        const SocketInfoVector::Iterator end = m_socketinfos.end();

        for (; current != end; ++current)
        {
            if (current->first == socketDescription)
            {
                m_socketinfos.erase(current);
                break;
            }
        }
    }

    void SocketManager::untrackSocket(PlatformSocket* socket)
    {
        untrackSocketInternal(m_tcpSocketMap, socket);
    }

    void SocketManager::untrackSocket(PlatformServerSocket* socket)
    {
        untrackSocketInternal(m_serverSocketMap, socket);
    }

    void SocketManager::serverSocketCallback(const SocketDescription& socketDescription)
    {
        socketCallBackInternal<ServerSocketDelegatePair>(m_serverSocketMap, socketDescription);
    }

    void SocketManager::tcpSocketCallback(const SocketDescription& socketDescription)
    {
        socketCallBackInternal<TcpSocketDelegatePair>(m_tcpSocketMap, socketDescription);
    }
}
