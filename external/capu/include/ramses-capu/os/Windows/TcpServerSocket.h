/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_WINDOWS_TCPSERVERSOCKET_H
#define RAMSES_CAPU_WINDOWS_TCPSERVERSOCKET_H

#include "ramses-capu/os/Socket.h"
#include "ramses-capu/os/Memory.h"
#include "ramses-capu/os/TcpSocket.h"

namespace ramses_capu
{
    namespace os
    {
        class TcpServerSocket
        {
        public:
            TcpServerSocket();
            ~TcpServerSocket();

            ramses_capu::TcpSocket* accept(uint32_t timeoutMillis = 0);
            status_t close();
            status_t bind(uint16_t port, const char* addr = NULL, bool reuseAddr = true);
            status_t listen(uint8_t backlog);
            uint16_t port();
            const ramses_capu::os::SocketDescription& getSocketDescription() const;
        private:
            SocketDescription mTcpServerSocket;
            WSADATA mWsaData;
            bool mIsBound;
            uint16_t mPort;
        };

        inline
        TcpServerSocket::TcpServerSocket() : mPort(0)
        {
            mTcpServerSocket = INVALID_SOCKET;
            mIsBound = false;
            //Initialize Winsock
            int32_t result = WSAStartup(MAKEWORD(2, 2), &mWsaData);
            if (result == 0)
            {
                //create the socket which is used to connect the server
                mTcpServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            }
            else
            {
                WSACleanup();
                mTcpServerSocket = INVALID_SOCKET;
            }
        }

        inline
        TcpServerSocket::~TcpServerSocket()
        {
            close();
        }

        inline
        ramses_capu::TcpSocket*
        TcpServerSocket::accept(uint32_t timeoutMillis)
        {
            if (timeoutMillis > 0)
            {
                fd_set readfds;
                FD_ZERO(&readfds);
                FD_SET(mTcpServerSocket, &readfds);

                timeval timeout;
                timeout.tv_sec = timeoutMillis / 1000;
                timeout.tv_usec = (timeoutMillis % 1000) * 1000;

                int ret = select(0, &readfds, 0, 0, &timeout);  // block until request comes in
                if (ret <= 0)
                {
                    // no request came within timeout
                    return 0;
                }
            }

            SocketDescription socket = ::accept(mTcpServerSocket, NULL, NULL);

            if (socket == INVALID_SOCKET)
            {
                return 0;
            }

            ramses_capu::TcpSocket* clientSocket = new ramses_capu::TcpSocket(socket);
            return clientSocket;
        }

        inline
        status_t
        TcpServerSocket::close()
        {
            int32_t returnValue = CAPU_OK;
            if (mTcpServerSocket == INVALID_SOCKET)
            {
                returnValue = CAPU_SOCKET_ESOCKET;
            }
            else
            {
                if (closesocket(mTcpServerSocket) != 0)
                {
                    int32_t result = WSAGetLastError();
                    if (result != WSANOTINITIALISED)  //socket has already been closed
                    {
                        returnValue = CAPU_SOCKET_ECLOSE;
                    }
                }
                WSACleanup();
            }
            mTcpServerSocket = INVALID_SOCKET;
            mIsBound = false;
            return returnValue;
        }

        inline
        status_t
        TcpServerSocket::bind(uint16_t port, const char* addr, bool reuseAddr)
        {
            //check if the address is valid

            if (mIsBound)
            {
                return CAPU_ERROR;
            }

            if (mTcpServerSocket == INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            if (port != 0 && reuseAddr)
            {
                int32_t optVal = 1;
                if (setsockopt(mTcpServerSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&optVal), sizeof(optVal)) != 0)
                {
                    return CAPU_SOCKET_ESOCKET;
                }
            }

            mPort = port;
            sockaddr_in socketAddr;
            Memory::Set(&socketAddr, 0x00, sizeof(socketAddr));
            socketAddr.sin_family = AF_INET;
            if (addr == NULL)
            {
                socketAddr.sin_addr.s_addr = INADDR_ANY;
            }
            else
            {
                const int32_t result = inet_pton(AF_INET, addr, &(socketAddr.sin_addr.s_addr));
                if ((result <= 0) || (INADDR_NONE == socketAddr.sin_addr.s_addr))
                {
                    return CAPU_SOCKET_EADDR;
                }
            }

            socketAddr.sin_port = htons(port);

            int32_t result = ::bind(mTcpServerSocket, reinterpret_cast<sockaddr*>( &socketAddr), sizeof(socketAddr));
            if (result == SOCKET_ERROR)
            {
                return CAPU_SOCKET_EBIND;
            }

            // if the port was zero check out the port chosen by the os
            if (0 == mPort)
            {
                int size = sizeof(struct sockaddr_in);
                Memory::Set( &socketAddr, 0x00, sizeof(socketAddr));
                result = ::getsockname(mTcpServerSocket, reinterpret_cast<sockaddr*>( &socketAddr), &size);
                if (result == SOCKET_ERROR)
                {
                    return CAPU_SOCKET_EBIND;
                }
                mPort = ntohs(socketAddr.sin_port);
            }


            mIsBound = true;
            return CAPU_OK;
        }

        inline
        status_t
        TcpServerSocket::listen(uint8_t backlog)
        {
            if (!mIsBound)
            {
                return CAPU_EINVAL;
            }

            if (mTcpServerSocket == INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            int32_t result = ::listen(mTcpServerSocket, backlog);

            if (result == SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline
        uint16_t TcpServerSocket::port()
        {
            return mPort;
        }

        inline
        const ramses_capu::os::SocketDescription& TcpServerSocket::getSocketDescription() const
        {
            return mTcpServerSocket;
        }

    }
}
#endif //RAMSES_CAPU_WINDOWS_TCPSERVERSOCKET_H
