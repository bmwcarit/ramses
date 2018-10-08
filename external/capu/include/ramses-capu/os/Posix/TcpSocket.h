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


#ifndef RAMSES_CAPU_UNIXBASED_TCP_SOCKET_H
#define RAMSES_CAPU_UNIXBASED_TCP_SOCKET_H

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <cstring>
#include <ramses-capu/os/Socket.h>
#include <ramses-capu/os/Generic/TcpSocket.h>
#include <unistd.h>
#include <fcntl.h>

namespace ramses_capu
{
    namespace posix
    {

      class TcpSocket: private ramses_capu::generic::TcpSocket
        {
        public:
            TcpSocket();
            TcpSocket(const ramses_capu::os::SocketDescription& socketDescription);
            ~TcpSocket();

            status_t send(const char* buffer, int32_t length, int32_t& sentBytes);
            status_t receive(char* buffer, int32_t length, int32_t& numBytes);
            status_t close();
            status_t connect(const char* dest_addr, uint16_t port, int32_t timeoutMs);

            status_t setTimeout(int32_t timeout);
            status_t getTimeout(int32_t& timeout);

            using ramses_capu::generic::TcpSocket::setBufferSize;
            using ramses_capu::generic::TcpSocket::setLingerOption;
            using ramses_capu::generic::TcpSocket::setNoDelay;
            using ramses_capu::generic::TcpSocket::setKeepAlive;
            using ramses_capu::generic::TcpSocket::getBufferSize;
            using ramses_capu::generic::TcpSocket::getLingerOption;
            using ramses_capu::generic::TcpSocket::getNoDelay;
            using ramses_capu::generic::TcpSocket::getKeepAlive;
            using ramses_capu::generic::TcpSocket::getRemoteAddress;
            using ramses_capu::generic::TcpSocket::getSocketDescription;

        protected:
            status_t setPosixSocketParams();
            int32_t mTimeout;
            using ramses_capu::generic::TcpSocket::mSocket;
            using ramses_capu::generic::TcpSocket::getSocketAddr;

        private:
            status_t setTimeoutInternal();

        };

        inline
        TcpSocket::TcpSocket()
        : mTimeout(-1)
        {
        }

        inline
        TcpSocket::TcpSocket(const ramses_capu::os::SocketDescription& socketDescription)
        : ramses_capu::generic::TcpSocket()
        , mTimeout(-1)
        {
            mSocket = socketDescription;
        }

        inline
        TcpSocket::~TcpSocket()
        {
            close();
        }

        inline
        status_t
        TcpSocket::send(const char* buffer, int32_t length, int32_t& sentBytes)
        {

            if ((buffer == NULL) || (length < 0))
            {
                return CAPU_EINVAL;
            }
            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            const int32_t res = ::send(mSocket, buffer, length, 0);

            if (res < 0)
            {
                if (errno == EAGAIN) {
                    return CAPU_ETIMEOUT;
                } else {
                    return CAPU_ERROR;
                }
            }

            sentBytes = res;
            return CAPU_OK;
        }

        inline
        status_t
        TcpSocket::receive(char* buffer, int32_t length, int32_t& numBytes)
        {
            if ((buffer == NULL) || (length < 0))
            {
                return CAPU_EINVAL;
            }
            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            const int32_t res = ::recv(mSocket, buffer, length, 0);

            if (res == -1)
            {
                numBytes = 0;
                if (errno == EAGAIN || errno == EINTR)
                {
                    return CAPU_ETIMEOUT;
                }
                else
                {
                    return CAPU_ERROR;
                }
            }
            numBytes = res;

            return CAPU_OK;
        }

        inline
        status_t
        TcpSocket::close()
        {
            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }
            else
            {
                shutdown(mSocket, SHUT_RDWR);
                if (::close(mSocket) < 0)
                {
                    return CAPU_SOCKET_ECLOSE;
                }
                mSocket = -1;
                return CAPU_OK;
            }
        }

        inline
        status_t
        TcpSocket::connect(const char* dest_addr, uint16_t port, int32_t timeoutMs)
        {
            if ((dest_addr == NULL) || (port == 0))
            {
                return CAPU_EINVAL;
            }
            mSocket = socket(AF_INET, SOCK_STREAM, 0);

            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            status_t status = setPosixSocketParams();
            if (status != CAPU_OK && status != CAPU_EINVAL)
            {
                return status;
            }

            struct sockaddr_in serverAddress;
            status = getSocketAddr(dest_addr, port, serverAddress);
            if (status != CAPU_OK)
            {
                return status;
            }

            const int socketFlags = fcntl(mSocket, F_GETFL, 0);
            if (timeoutMs > 0)
            {
                fcntl(mSocket, F_SETFL, socketFlags | O_NONBLOCK);
            }
            int res = ::connect(mSocket, reinterpret_cast<const sockaddr*>(&serverAddress), sizeof(serverAddress));
            if (res != 0)
            {
                if (errno != EINPROGRESS)
                {
                    close();
                    return CAPU_SOCKET_ECONNECT;
                }

                fd_set wfd, efd;
                FD_ZERO(&wfd);
                FD_SET(mSocket, &wfd);
                FD_ZERO(&efd);
                FD_SET(mSocket, &efd);

                if (timeoutMs > 0)
                {
                    struct timeval timeout;
                    timeout.tv_sec = timeoutMs / 1000;
                    timeout.tv_usec = (timeoutMs % 1000) * 1000;

                    res = select(mSocket+1, nullptr, &wfd, &efd, &timeout);
                }
                else
                {
                    res = select(mSocket+1, nullptr, &wfd, &efd, nullptr);
                }
                if (res == -1)
                {
                    close();
                    return CAPU_SOCKET_ECONNECT;
                }
                if (res == 0)
                {
                    // timeout
                    close();
                    return CAPU_SOCKET_ECONNECT;
                }

                assert(res == 1);
                if (FD_ISSET(mSocket, &efd))
                {
                    // connect error
                    close();
                    return CAPU_SOCKET_ECONNECT;
                }

                assert(FD_ISSET(mSocket, &wfd));

                // check socket error code
                int sock_error = 0;
                socklen_t sock_error_len = sizeof sock_error;
                res = getsockopt(mSocket, SOL_SOCKET, SO_ERROR, &sock_error, &sock_error_len);
                if (res != 0 || sock_error != 0)
                {
                    close();
                    return CAPU_SOCKET_ECONNECT;
                }
            }

            if (timeoutMs > 0)
            {
                res = fcntl(mSocket, F_SETFL, socketFlags);
                if (res != 0)
                {
                    close();
                    return CAPU_SOCKET_ECONNECT;
                }
            }

            return CAPU_OK;
        }

        inline
        status_t TcpSocket::setPosixSocketParams()
        {
            status_t returnStatus = CAPU_OK;
            status_t status = setSocketParameters();
            if (status != CAPU_OK) {
                returnStatus = status;
            }

            status = setTimeoutInternal();
            if (status != CAPU_OK) {
                returnStatus = status;
            }

            return returnStatus;
        }

        inline
        status_t
        TcpSocket::setTimeout(int32_t timeout)
        {
            mTimeout = timeout;
            if (-1 != mSocket)
            {
                return setTimeoutInternal();
            }
            return CAPU_OK;
        }

        inline status_t TcpSocket::setTimeoutInternal()
        {

            if (mTimeout < 0)
            {
                return CAPU_EINVAL;
            }

            if (mSocket == -1)
            {
                return CAPU_SOCKET_ESOCKET;
            }

            struct timeval _timeout;
            _timeout.tv_sec = mTimeout / 1000;
            _timeout.tv_usec = (mTimeout % 1000) * 1000;

            if (setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, &_timeout, sizeof(_timeout)) < 0)
            {
                return CAPU_ERROR;
            }
            if (setsockopt(mSocket, SOL_SOCKET, SO_SNDTIMEO, &_timeout, sizeof(_timeout)) < 0)
            {
                return CAPU_ERROR;
            }

            return CAPU_OK;
        }

        inline status_t TcpSocket::getTimeout(int32_t& timeout)
        {
            if (mSocket == -1)
            {
                timeout = mTimeout;
                return CAPU_OK;
            }

            struct timeval _timeout;

            socklen_t len = sizeof(_timeout);

            if (getsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, &_timeout, &len) < 0)
            {
                return CAPU_ERROR;
            }

            timeout = _timeout.tv_sec * 1000;
            timeout += _timeout.tv_usec / 1000;

            return CAPU_OK;
        }
    }


}

#endif // RAMSES_CAPU_UNIXBASED_TCP_SOCKET_H
