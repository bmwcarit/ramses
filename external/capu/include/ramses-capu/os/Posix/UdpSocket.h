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

#ifndef RAMSES_CAPU_UNIXBASED_UDP_SOCKET_H
#define RAMSES_CAPU_UNIXBASED_UDP_SOCKET_H

#include <ramses-capu/os/Socket.h>
#include "ramses-capu/os/StringUtils.h"
#include "ramses-capu/os/Memory.h"

namespace ramses_capu
{
    namespace posix
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

        protected:
            int32_t mSocket;
        private:
            int32_t mAddressFamily;
            int32_t mSocketType;
            bool mIsBound;
            bool mIsInitialized;
            SocketAddrInfo mAddrInfo;

            void initialize();
        };

        inline
        UdpSocket::UdpSocket()
            : mIsBound(false)
            , mIsInitialized(false)
        {
            initialize();
        }

        inline
        UdpSocket::~UdpSocket()
        {
            close();
        }

        inline
        void
        UdpSocket::initialize()
        {
            if (!mIsInitialized)
            {
                //create the socket which is used to connect the server
                mAddressFamily = AF_INET;
                mSocketType = SOCK_DGRAM;
                mSocket = socket(mAddressFamily, mSocketType, 0);

                if (mSocket != -1)
                {
                    mIsInitialized = true;
                }
            }
        }

        inline
        status_t
        UdpSocket::bind(const uint16_t port, const char* addr)
        {
            if (mIsBound)
            {
                return CAPU_ERROR;
            }

            initialize();

            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            if (port != 0)
            {
                int32_t optval = 1;
                if (setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) != 0)
                {
                    return CAPU_SOCKET_ESOCKET;
                }
            }

            struct sockaddr_in mServerAddress;
            memset(reinterpret_cast<char*>(&mServerAddress), 0x00, sizeof(mServerAddress));
            mServerAddress.sin_family = AF_INET;
            if (addr == NULL)
            {
                mServerAddress.sin_addr.s_addr = INADDR_ANY;
            }
            else if (inet_aton(addr, &mServerAddress.sin_addr) == 0)
            {
                return CAPU_SOCKET_EADDR;
            }
            mServerAddress.sin_port = htons(port);

            int32_t res = ::bind(mSocket, reinterpret_cast<sockaddr*>(&mServerAddress), sizeof(struct sockaddr_in));
            if (res < 0)
            {
                return CAPU_SOCKET_EBIND;
            }

            struct sockaddr_in sin;
            socklen_t len = sizeof(sin);
            if (getsockname(mSocket, reinterpret_cast<struct sockaddr*>(&sin), &len) != -1)
            {
                char str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(sin.sin_addr.s_addr), str, INET_ADDRSTRLEN);
                mAddrInfo.addr = str;
                mAddrInfo.port = ntohs(sin.sin_port);
            }

            mIsBound = true;
            return CAPU_OK;
        }

        inline
        const
        SocketAddrInfo&
        UdpSocket::getSocketAddrInfo() const
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

            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            initialize();

            struct sockaddr_in receiverSockAddr;
            Memory::Set(&receiverSockAddr, 0, sizeof(sockaddr_in));

            receiverSockAddr.sin_family = AF_INET;
            receiverSockAddr.sin_port   = htons(receiverPort);
            receiverSockAddr.sin_addr.s_addr = inet_addr(receiverAddr);

            const int32_t result = sendto(mSocket, buffer, length, 0, reinterpret_cast<sockaddr*>(&receiverSockAddr), sizeof(receiverSockAddr));

            if (result == -1)
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

            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            sockaddr_in remoteSocketAddr;
            socklen_t remoteSocketAddrSize = sizeof(remoteSocketAddr);

            int32_t result = recvfrom(mSocket, buffer, length, 0, reinterpret_cast<sockaddr*>(&remoteSocketAddr), &remoteSocketAddrSize);
            if (result == -1)
            {
                numBytes = 0;
                if (errno == EAGAIN)
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
                    sender->port = ntohs(remoteSocketAddr.sin_port);
                    char* addr = inet_ntoa(remoteSocketAddr.sin_addr);
                    sender->addr = addr;
                }
            }
            numBytes = result;
            return CAPU_OK;
        }

        inline
        status_t
        UdpSocket::close()
        {
		    if(mIsBound)
			{
		     	mIsInitialized = false;
				mIsBound = false;

				if (mSocket == -1)
				{
					return CAPU_SOCKET_ESOCKET;
				}
				else
				{
					if (::close(mSocket) < 0)
					{
						mSocket = -1;
						return CAPU_SOCKET_ECLOSE;
					}
				}
			}
            mSocket = -1;

            return CAPU_OK;
        }

        inline
        status_t
        UdpSocket::setBufferSize(const int32_t bufferSize)
        {
            if (bufferSize < 0)
            {
                return CAPU_EINVAL;
            }
            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            if (setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize)) == -1)
            {
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline
        status_t
        UdpSocket::allowBroadcast(const bool broadcast)
        {
            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            int32_t allow = 0;

            if (broadcast)
                allow = 1;

            if (setsockopt(mSocket, SOL_SOCKET, SO_BROADCAST, &allow, sizeof(int32_t)) == -1)
            {
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline
        status_t
        UdpSocket::setTimeout(const int32_t timeout)
        {
            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            struct timeval soTimeout;
            soTimeout.tv_sec = timeout / 1000;
            soTimeout.tv_usec = (timeout % 1000) * 1000;

            if (setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, &soTimeout, sizeof(soTimeout)) == -1)
            {
                return CAPU_ERROR;
            }
            if (setsockopt(mSocket, SOL_SOCKET, SO_SNDTIMEO, &soTimeout, sizeof(soTimeout)) == -1)
            {
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline
        status_t
        UdpSocket::getBufferSize(int32_t& bufferSize)
        {
            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            socklen_t len = sizeof(bufferSize);
            if (getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&bufferSize), &len) == -1)
            {
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline
        status_t
        UdpSocket::getTimeout(int32_t& timeout)
        {
            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            struct timeval soTimeout;
            socklen_t len = sizeof(soTimeout);

            if (getsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&soTimeout), &len) == -1)
            {
                return CAPU_ERROR;
            }

            timeout = soTimeout.tv_sec * 1000 + (soTimeout.tv_usec / 1000);

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

#endif // RAMSES_CAPU_UNIXBASED_UDP_SOCKET_H
