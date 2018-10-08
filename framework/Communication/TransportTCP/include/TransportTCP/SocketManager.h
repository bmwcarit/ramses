//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SOCKETMANAGER_H
#define RAMSES_SOCKETMANAGER_H

#include "Collections/HashMap.h"
#include "Collections/Pair.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "ramses-capu/util/Delegate.h"
#include "Utils/NonblockingSocketChecker.h"
#include "PlatformAbstraction/PlatformUdpSocket.h"
#include "PlatformAbstraction/PlatformSocket.h"
#include "PlatformAbstraction/PlatformServerSocket.h"


namespace ramses_internal
{
    typedef ramses_capu::Delegate<void, PlatformServerSocket&> ServerSocketDelegate;
    typedef ramses_capu::Delegate<void, PlatformSocket&> TcpSocketDelegate;

    typedef Pair<PlatformServerSocket*, ServerSocketDelegate> ServerSocketDelegatePair;
    typedef Pair<PlatformSocket*, TcpSocketDelegate> TcpSocketDelegatePair;

    class SocketManager final
    {
    public:
        SocketManager();
        ~SocketManager();

        void trackSocket(PlatformSocket* socket, const TcpSocketDelegate& delegate);
        void trackSocket(PlatformServerSocket* socket, const ServerSocketDelegate& delegate);

        void untrackSocket(PlatformSocket* socket);
        void untrackSocket(PlatformServerSocket* socket);

        void interruptWaitCall();
        void checkAllSockets(Bool blocking, UInt32 timeout = 0);

        void exit();

    private:
        typedef HashMap<SocketDescription, ServerSocketDelegatePair> SocketDescriptionToServerSocketDelegateMap;
        typedef HashMap<SocketDescription, TcpSocketDelegatePair> SocketDescriptorToTcpSocketMap;

        typedef HashMap<void*, SocketDescription> SocketPtrToSocketDescriptionMap;

        SocketDescriptorToTcpSocketMap m_tcpSocketMap;
        SocketDescriptionToServerSocketDelegateMap m_serverSocketMap;

        SocketPtrToSocketDescriptionMap m_socketPtrToSocketDescription;

        SocketInfoVector    m_socketinfos;

        PlatformUdpSocket   m_interruptSocket;

        void socketCallBackInternal(const SocketDescription& socketdescription);
        void serverSocketCallback(const SocketDescription& socketDescription);
        void tcpSocketCallback(const SocketDescription& socketDescription);
        void waitCallInterrupted(const SocketDescription& socketDescription);

        void removeSocketInfoPair(const SocketDescription& socketDescription);

        template<typename MAPTYPE, typename SOCKET>
        void untrackSocketInternal(MAPTYPE& map, SOCKET* socket);

        template<typename MAPTYPE, typename SOCKETDELEGATEPAIR>
        void trackSocketInternal(MAPTYPE& map, const SOCKETDELEGATEPAIR& pair, const SocketDelegate& socketDelegate);

        template<typename SOCKETDELEGATEINFOS, typename MAPTYPE>
        void socketCallBackInternal(MAPTYPE& map, const SocketDescription& socketdescription);
    };

    template<typename MAPTYPE, typename SOCKET>
    inline
    void SocketManager::untrackSocketInternal(MAPTYPE& map, SOCKET* socket)
    {
        SocketDescription socketDescription;
        if (m_socketPtrToSocketDescription.get(socket, socketDescription) == EStatus_RAMSES_OK)
        {
            removeSocketInfoPair(socketDescription);
            map.remove(socketDescription);
            m_socketPtrToSocketDescription.remove(socket);
        }
    }

    template<typename MAPTYPE, typename SOCKETDELEGATEPAIR>
    inline
    void SocketManager::trackSocketInternal(MAPTYPE& map, const SOCKETDELEGATEPAIR& pair, const SocketDelegate& socketDelegate)
    {
        m_socketinfos.push_back(SocketInfoPair(pair.first->getSocketDescription(), socketDelegate));
        map.put(pair.first->getSocketDescription(), pair);
        m_socketPtrToSocketDescription.put(pair.first, pair.first->getSocketDescription());
    }

    template<typename SOCKETDELEGATEINFOS, typename MAPTYPE>
    inline
    void SocketManager::socketCallBackInternal(MAPTYPE& map, const SocketDescription& socketdescription)
    {
        for (SocketInfoVector::Iterator iter = m_socketinfos.begin(); iter != m_socketinfos.end(); ++iter)
        {
            if (iter->first == socketdescription)
            {
                m_socketinfos.erase(iter);
                break;
            }
        }

        SOCKETDELEGATEINFOS socketInfos;
        if (map.get(socketdescription, socketInfos) == EStatus_RAMSES_OK)
        {
            socketInfos.second(*socketInfos.first);
        }
    }
}

#endif
