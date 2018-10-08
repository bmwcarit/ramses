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

#ifndef RAMSES_CAPU_GENERIC_TCPSOCKET_H
#define RAMSES_CAPU_GENERIC_TCPSOCKET_H

#include "ramses-capu/Config.h"
#include "ramses-capu/Error.h"
#include "ramses-capu/os/Socket.h"

namespace ramses_capu
{

    namespace generic
    {
        class TcpSocket
        {
        public:
            status_t setBufferSize(int32_t bufferSize);
            status_t setLingerOption(bool isLinger, uint16_t linger);
            status_t setNoDelay(bool noDelay);
            status_t setKeepAlive(bool keepAlive);
            status_t getBufferSize(int32_t& bufferSize);
            status_t getLingerOption(bool& isLinger, uint16_t& linger);
            status_t getNoDelay(bool& noDelay);
            status_t getKeepAlive(bool& keepAlive);
            status_t getRemoteAddress(char** remoteAddress);
            const ramses_capu::os::SocketDescription& getSocketDescription() const;

        protected:
            TcpSocket();

            status_t setSocketParameters();
            status_t getSocketAddr(const char* dest_addr, uint16_t port, struct sockaddr_in& socketAddressInfo);

        private:
            int32_t mBufferSize;
            bool  mIsLinger;
            uint16_t mLinger;
            bool  mNoDelay;
            bool  mKeepAlive;

        protected:
            ramses_capu::os::SocketDescription mSocket;

        private:
            status_t setBufferSizeInternal();
            status_t setLingerOptionInternal();
            status_t setNoDelayInternal();
            status_t setKeepAliveInternal();
        };

        inline
        TcpSocket::TcpSocket()
            : mBufferSize(-1)
            , mIsLinger(false)
            , mLinger(0)
            , mNoDelay(false)
            , mKeepAlive(false)
            , mSocket(CAPU_INVALID_SOCKET)
        {

        }

        inline
        status_t TcpSocket::setSocketParameters()
        {
            status_t returnStatus = CAPU_OK;
            status_t status = setBufferSizeInternal();
            if (status != CAPU_OK) {
                returnStatus = status;
            }
            status = setNoDelayInternal();
            if (status != CAPU_OK) {
                returnStatus = status;
            }
            status = setKeepAliveInternal();
            if (status != CAPU_OK) {
                returnStatus = status;
            }

            return returnStatus;
        }

        inline
            status_t TcpSocket::getSocketAddr(const char* dest_addr, uint16_t port, struct sockaddr_in& socketAddressInfo)
        {
            addrinfo *res = NULL;
            addrinfo hints;
            ramses_capu::Memory::Set(reinterpret_cast<char*>(&hints), 0x00, sizeof(addrinfo));
            hints.ai_family = AF_INET;

            if (0 != getaddrinfo(reinterpret_cast<const char*>(dest_addr), NULL, &hints, &res))
            {
                return CAPU_SOCKET_EADDR;
            }

            for (addrinfo *ptr = res; ptr != NULL; ptr = ptr->ai_next)
            {
                if (ptr->ai_family == AF_INET)
                {
                    ramses_capu::Memory::Set(reinterpret_cast<char*>(&socketAddressInfo), 0x00, sizeof(sockaddr_in));

                    socketAddressInfo.sin_family = AF_INET;
                    ramses_capu::Memory::Copy(reinterpret_cast<char*>(&socketAddressInfo.sin_addr.s_addr), reinterpret_cast<char*>(ptr->ai_addr->sa_data) + 2, sizeof(in_addr));
                    socketAddressInfo.sin_port = htons(port);

                    freeaddrinfo(res);
                    return CAPU_OK;
                }
            }

            freeaddrinfo(res);
            return CAPU_SOCKET_ERROR;
        }

        inline status_t TcpSocket::getRemoteAddress(char** remoteAddress)
        {
            if (mSocket == CAPU_INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            sockaddr_in client_address;
            socklen_t size = sizeof(client_address);
            ramses_capu::Memory::Set(&client_address, 0, size);

            int32_t ret = getpeername(mSocket, reinterpret_cast<sockaddr*>(&client_address), &size);

            if (ret != 0)
            {
                return CAPU_ERROR;
            }

            if ( 0 != remoteAddress)
            {
                char buffer[INET_ADDRSTRLEN] = { '\0' };
                const char *remoteIP = inet_ntop(AF_INET, &(client_address.sin_addr), buffer, INET_ADDRSTRLEN);
                if (NULL == remoteIP)
                {
                    return CAPU_ERROR;
                }

                uint_t stringLength = StringUtils::Strlen(remoteIP) + 1;
                *remoteAddress = new char[stringLength];
                StringUtils::Strncpy(*remoteAddress, stringLength, remoteIP);
            }
            return CAPU_OK;
        }

        inline
        status_t
            TcpSocket::setBufferSize(int32_t bufferSize)
        {
            mBufferSize = bufferSize;
            if (CAPU_INVALID_SOCKET != mSocket)
            {
                return setBufferSizeInternal();
            }
            return CAPU_OK;
        }

        inline
            status_t
            TcpSocket::setLingerOption(bool isLinger, uint16_t linger)
        {
            mIsLinger = isLinger;
            mLinger   = linger;
            if (CAPU_INVALID_SOCKET != mSocket)
            {
                return setLingerOptionInternal();
            }
            return CAPU_OK;
        }

        inline
            status_t
            TcpSocket::setNoDelay(bool noDelay)
        {
            mNoDelay = noDelay;
            if (CAPU_INVALID_SOCKET != mSocket)
            {
                return setNoDelayInternal();
            }
            return CAPU_OK;
        }

        inline
            status_t
            TcpSocket::setKeepAlive(bool keepAlive)
        {
            mKeepAlive = keepAlive;
            if (CAPU_INVALID_SOCKET != mSocket)
            {
                return setKeepAliveInternal();
            }
            return CAPU_OK;
        }

        inline
            status_t
            TcpSocket::setBufferSizeInternal()
        {
            if (mBufferSize < 0)
            {
                return CAPU_EINVAL;
            }
            if (mSocket == CAPU_INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            if (setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&mBufferSize), sizeof(mBufferSize)) <= CAPU_SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline
            status_t
            TcpSocket::setLingerOptionInternal()
        {
            if (mSocket == CAPU_INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            struct linger soLinger;

            soLinger.l_onoff  = mIsLinger ? 1 : 0;
            soLinger.l_linger = mIsLinger ? mLinger : 0;

            if (setsockopt(mSocket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&soLinger), sizeof(soLinger)) <= CAPU_SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }
            return CAPU_OK;
        }

        inline
            status_t
            TcpSocket::setNoDelayInternal()
        {
            if (mSocket == CAPU_INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }
            int32_t opt;
            if (mNoDelay)
            {
                opt = 1;
            }
            else
            {
                opt = 0;
            }
            if (setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&opt), sizeof(opt)) <= CAPU_SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }
            return CAPU_OK;
        }

        inline
            status_t
            TcpSocket::setKeepAliveInternal()
        {
            if (mSocket == CAPU_INVALID_SOCKET)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            int32_t opt = mKeepAlive ? 1 : 0;

            if (setsockopt(mSocket, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&opt), sizeof(opt)) <= CAPU_SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }
            return CAPU_OK;
        }

        inline status_t TcpSocket::getBufferSize(int32_t& bufferSize)
        {
            if (mSocket == CAPU_INVALID_SOCKET)
            {
                bufferSize = mBufferSize;
                return CAPU_OK;
            }

            socklen_t len = sizeof(bufferSize);
            if (getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&bufferSize), &len) <= CAPU_SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline status_t TcpSocket::getLingerOption(bool& isLinger, uint16_t& _linger)
        {
            if (mSocket == CAPU_INVALID_SOCKET)
            {
                isLinger = mIsLinger;
                _linger = mLinger;
                return CAPU_OK;
            }

            struct linger soLinger;
            socklen_t len = sizeof(soLinger);

            if (getsockopt(mSocket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&soLinger), &len) <= CAPU_SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }

            _linger = soLinger.l_linger;

            if (soLinger.l_onoff == 1)
            {
                isLinger = true;
            }
            else
            {
                isLinger = false;
            }

            return CAPU_OK;
        }

        inline status_t TcpSocket::getNoDelay(bool& noDelay)
        {
            if (mSocket == CAPU_INVALID_SOCKET)
            {
                noDelay = mNoDelay;
                return CAPU_OK;
            }

            int32_t opt = 0;
            socklen_t len = sizeof(opt);

            if (getsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&opt), &len) <= CAPU_SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }

            if (opt > 0)
            {
                noDelay = true;
            }
            else
            {
                noDelay = false;
            }

            return CAPU_OK;
        }

        inline status_t TcpSocket::getKeepAlive(bool& keepAlive)
        {
            if (mSocket == CAPU_INVALID_SOCKET)
            {
                keepAlive  = mKeepAlive;
                return CAPU_OK;
            }

            int32_t opt = 0;
            socklen_t len = sizeof(opt);

            if (getsockopt(mSocket, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&opt), &len) <= CAPU_SOCKET_ERROR)
            {
                return CAPU_ERROR;
            }

            if (opt == 1)
            {
                keepAlive = true;
            }
            else
            {
                keepAlive = false;
            }

            return CAPU_OK;
        }

        inline const ramses_capu::os::SocketDescription& TcpSocket::getSocketDescription() const
        {
            return mSocket;
        }
    }


}

#endif //RAMSES_CAPU_GENERIC_TCPSOCKET_H
