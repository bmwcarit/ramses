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

#ifndef RAMSES_CAPU_UNIXBASED_TCPSERVERSOCKET_H
#define RAMSES_CAPU_UNIXBASED_TCPSERVERSOCKET_H

#include <ramses-capu/os/Socket.h>
#include <ramses-capu/os/Memory.h>

namespace ramses_capu
{
    namespace posix
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
            ramses_capu::os::SocketDescription mServerSock;
            bool mIsBound;
            uint16_t mPort;
        };


        inline
        TcpServerSocket::TcpServerSocket() : mServerSock(-1), mIsBound(false), mPort(0)
        {
            mServerSock = socket(AF_INET, SOCK_STREAM, 0);
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
                FD_SET(mServerSock, &readfds);

                struct timeval timeout;
                timeout.tv_sec = timeoutMillis / 1000;
                timeout.tv_usec = (timeoutMillis % 1000) * 1000;

                int ret = select(mServerSock + 1, &readfds, 0, 0, &timeout); // block until request comes in
                if (ret <= 0)
                {
                    return 0;
                }
            }

            int32_t clientAddrSize = sizeof(sockaddr_in);
            struct sockaddr_in serverAddr;
            int32_t socket = ::accept(mServerSock, reinterpret_cast<sockaddr*>(&serverAddr), reinterpret_cast<socklen_t*>(&clientAddrSize));

            if (socket < 0)
            {
                return 0;
            }

            ramses_capu::TcpSocket* s = new ramses_capu::TcpSocket(socket);
            return s;
        }

        inline
        status_t
        TcpServerSocket::close()
        {
            int32_t returnValue = CAPU_OK;
            if (mServerSock != -1)
            {
                shutdown(mServerSock, SHUT_RDWR);
                if (::close(mServerSock) < 0)
                {
                    returnValue = CAPU_ERROR;
                }
            }
            else
            {
                returnValue = CAPU_SOCKET_ESOCKET;
            }
            mServerSock = -1;
            mIsBound = false;
            return returnValue;
        }

        inline
        status_t
        TcpServerSocket::bind(uint16_t port, const char* addr, bool reuseAddr)
        {

            sockaddr_in serverAddress;
            if (mIsBound)
            {
                return CAPU_ERROR;
            }

            if (mServerSock == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            if (port != 0 && reuseAddr)
            {
                int32_t optval = 1;
                if (setsockopt(mServerSock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) != 0)
                {
                    return CAPU_SOCKET_ESOCKET;
                }
            }

            mPort = port;

            Memory::Set(reinterpret_cast<char*>(&serverAddress), 0x00, sizeof(serverAddress));
            serverAddress.sin_family = AF_INET;
            if (addr == NULL)
            {
                serverAddress.sin_addr.s_addr = INADDR_ANY;
            }
            else if (inet_aton(addr, &serverAddress.sin_addr) == 0)
            {
                return CAPU_SOCKET_EADDR;
            }
            serverAddress.sin_port = htons(port);

            int32_t res = ::bind(mServerSock, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(struct sockaddr_in));
            if (res < 0)
            {
                return CAPU_SOCKET_EBIND;
            }

            // if the port was zero check out the port chosen by the os
            if (0 == mPort)
            {
                socklen_t size = sizeof(struct sockaddr_in);
                Memory::Set(reinterpret_cast<char*>(&serverAddress), 0x00, sizeof(serverAddress));
                res = ::getsockname(mServerSock, reinterpret_cast<sockaddr*>(&serverAddress), &size);
                if (res < 0)
                {
                    return CAPU_SOCKET_EBIND;
                }
                mPort = ntohs(serverAddress.sin_port);
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

            if (mServerSock == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            if (::listen(mServerSock, backlog) < 0)
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
            return mServerSock;
        }

    }
}

#endif // RAMSES_CAPU_UNIXBASED_TCPSERVERSOCKET_H
