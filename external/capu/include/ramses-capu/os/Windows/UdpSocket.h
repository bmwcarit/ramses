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

#ifndef RAMSES_CAPU_WINDOWS_UDP_SOCKET_H
#define RAMSES_CAPU_WINDOWS_UDP_SOCKET_H

#include "ramses-capu/os/Socket.h"
#include "ramses-capu/os/StringUtils.h"
#include "ramses-capu/os/Memory.h"

namespace ramses_capu
{
    namespace os
    {
        class UdpSocket
        {
        public:
            UdpSocket();
            ~UdpSocket();
            status_t bind(const uint16_t port, const char* addr = NULL);
            status_t send(const char* buffer, const int32_t length, const SocketAddrInfo& receiverAddr);
            status_t send(const char* buffer, const int32_t length, const char* receiverAddr, const uint16_t receiverPort);
            status_t receive(char* buffer, const int32_t length, int32_t& numBytes, SocketAddrInfo* sender);
            status_t close();
            status_t setBufferSize(const int32_t bufferSize);
            status_t setTimeout(const int32_t timeout);
            status_t allowBroadcast(const bool broadcast);
            status_t getBufferSize(int32_t& bufferSize);
            status_t getTimeout(int32_t& timeout);
            const SocketAddrInfo& getSocketAddrInfo() const;
            const ramses_capu::os::SocketDescription& getSocketDescription() const;

        private:
            SocketDescription mSocket;
            WSADATA mWsaData;
            bool mIsBound;
            SocketAddrInfo mAddrInfo;
            bool mIsInitilized;

            void initialize();
        };

        inline
        UdpSocket::UdpSocket()
            : mSocket(INVALID_SOCKET)
            , mIsBound(false)
            , mIsInitilized(false)
        {
            initialize();
        }

        inline
        UdpSocket::~UdpSocket()
        {
            close();
        }

        inline
        void UdpSocket::initialize()
        {
            if (!mIsInitilized)
            {
                int32_t result = WSAStartup(MAKEWORD(2, 2), &mWsaData);
                if (result == 0)
                {
                    //create the socket which is used to connect the server
                    mSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                    mIsBound = false;
                    if (mSocket == INVALID_SOCKET)
                    {
                        WSACleanup();
                    }
                    else
                    {
                        mIsInitilized = true;
                    }
                }
                else
                {
                    mSocket = INVALID_SOCKET;
                }
            }
        }

        inline
        status_t
        UdpSocket::bind(const uint16_t port, const char* addr)
        {
            //check if the address is valid

            initialize();

            if (mIsBound)
            {
                return CAPU_ERROR;
            }

            if (mSocket == INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            if (port != 0)
            {
                int32_t optVal = 1;
                if (setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&optVal), sizeof(optVal)) != 0)
                {
                    return CAPU_SOCKET_ESOCKET;
                }
            }

            sockaddr_in socketAddr;
            memset( &socketAddr, 0x00, sizeof(socketAddr));
            socketAddr.sin_family = AF_INET;
            if (addr == NULL)
            {
                socketAddr.sin_addr.s_addr = INADDR_ANY;
            }
            else
            {
                const int32_t result = inet_pton(AF_INET, addr, &(socketAddr.sin_addr.s_addr));
                if (result <= 0)
                {
                    return CAPU_SOCKET_EADDR;
                }
            }

            socketAddr.sin_port = htons(port);

            int32_t result = ::bind(mSocket, reinterpret_cast<sockaddr*>(&socketAddr), sizeof(socketAddr));

            if (result == SOCKET_ERROR)
            {
                return CAPU_SOCKET_EBIND;
            }

            struct sockaddr_in sin;
            socklen_t len = sizeof(sin);
            if (getsockname(mSocket, reinterpret_cast<struct sockaddr*>(&sin), &len) != -1)
            {
                char str[INET_ADDRSTRLEN] = { '\0' };
                inet_ntop(AF_INET, &(sin.sin_addr.s_addr), str, INET_ADDRSTRLEN);
                mAddrInfo.port = ntohs(sin.sin_port);
                mAddrInfo.addr = str;
            }

            mIsBound = true;
            return CAPU_OK;
        }

        inline
        const SocketAddrInfo& UdpSocket::getSocketAddrInfo() const
        {
            return mAddrInfo;
        }

        inline
        status_t
        UdpSocket::send(const char* buffer, const int32_t length, const char* receiverAddr, const uint16_t receiverPort)
        {
            if ((buffer == NULL) || (length < 0))
            {
                return CAPU_EINVAL;
            }

            if (mSocket == INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            initialize();

            struct sockaddr_in receiverSockAddr;
            Memory::Set(&receiverSockAddr, 0 , sizeof(sockaddr_in));

            receiverSockAddr.sin_family = AF_INET;
            receiverSockAddr.sin_port = htons(receiverPort);

            int32_t result = inet_pton(AF_INET, receiverAddr, &(receiverSockAddr.sin_addr.s_addr));
            if (result <= 0)
            {
                return CAPU_SOCKET_EADDR;
            }

            result = sendto(mSocket, buffer, length, 0, reinterpret_cast<sockaddr*>( &receiverSockAddr), sizeof(receiverSockAddr));

            if (result == SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }
            return CAPU_OK;
        }

        inline
        status_t
        UdpSocket::send(const char* buffer, const int32_t length, const SocketAddrInfo& receiverAddr)
        {
            return send(buffer, length, receiverAddr.addr.c_str(), receiverAddr.port);
        }

        inline
        status_t
        UdpSocket::receive(char* buffer, const int32_t length, int32_t& numBytes, SocketAddrInfo* sender)
        {
            if ((buffer == NULL) || (length < 0))
            {
                return CAPU_EINVAL;
            }

            if (mSocket == INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            sockaddr_in remoteSocketAddr;
            int32_t remoteSocketAddrSize = sizeof(remoteSocketAddr);

            int32_t result = recvfrom(mSocket, buffer, length, 0, reinterpret_cast<sockaddr*>(&remoteSocketAddr), &remoteSocketAddrSize);
            if (result == SOCKET_ERROR)
            {
                numBytes = 0;
                result = WSAGetLastError();
                if (result == WSAETIMEDOUT)
                {
                    return CAPU_ETIMEOUT;
                }
                else
                {
                    return CAPU_ERROR;
                }
            }
            else
            {
                if (sender != 0)
                {
                    char buf[INET_ADDRSTRLEN] = { '\0' };
                    const char* res = inet_ntop(AF_INET, &(remoteSocketAddr.sin_addr), buf, INET_ADDRSTRLEN);
                    if (0 == res)
                    {
                        return CAPU_ERROR;
                    }

                    sender->port = ntohs(remoteSocketAddr.sin_port);
                    sender->addr = res;
                }
            }

            numBytes = result;
            return CAPU_OK;
        }

        inline
        status_t
        UdpSocket::close()
        {
            int32_t returnValue = CAPU_OK;

            if (mIsBound)
            {
                if (mSocket == INVALID_SOCKET)
                {
                    returnValue = CAPU_SOCKET_ESOCKET;
                }
                else
                {
                    int32_t result = closesocket(mSocket);
                    if (result != 0)
                    {
                        result = WSAGetLastError();
                        if (result != WSANOTINITIALISED)  //socket has already been closed
                        {
                            returnValue = CAPU_SOCKET_ECLOSE;
                        }
                    }
                }
                WSACleanup();
                mIsBound = false;
                mIsInitilized = false;
            }
            mSocket = INVALID_SOCKET;
            return returnValue;
        }

        inline
        status_t
        UdpSocket::setBufferSize(const int32_t bufferSize)
        {
            if (bufferSize < 0)
            {
                return CAPU_EINVAL;
            }
            if (mSocket == INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            if (setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&bufferSize), sizeof(bufferSize)) == SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }

            if (setsockopt(mSocket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&bufferSize), sizeof(bufferSize)) == SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline
        status_t
        UdpSocket::setTimeout(const int32_t timeout)
        {
            if (mSocket == INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            if (setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(int32_t)) == SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }
            if (setsockopt(mSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(int32_t)) == SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline
        status_t
        UdpSocket::allowBroadcast(const bool broadcast)
        {
            if (mSocket == INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            int32_t allow = 0;

            if (broadcast)
                allow = 1;

            if (setsockopt(mSocket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&allow), sizeof(allow)) == SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline
        status_t
        UdpSocket::getBufferSize(int32_t& bufferSize)
        {
            if (mSocket == INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            socklen_t len = sizeof(bufferSize);
            if (getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&bufferSize), &len) == SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline
        status_t
        UdpSocket::getTimeout(int32_t& timeout)
        {
            if (mSocket == INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            struct timeval soTimeout;
            socklen_t len = sizeof(soTimeout);

            if (getsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&soTimeout), &len) == SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }

            timeout = soTimeout.tv_sec;

            return CAPU_OK;
        }

        inline
        const ramses_capu::os::SocketDescription&
        UdpSocket::getSocketDescription() const
        {
            return mSocket;
        }
    }

}

#endif //RAMSES_CAPU_WINDOWS_UDP_SOCKET_H
