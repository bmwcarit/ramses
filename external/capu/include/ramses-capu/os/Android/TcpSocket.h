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

#ifndef RAMSES_CAPU_ANDROID_TCP_SOCKET_H
#define RAMSES_CAPU_ANDROID_TCP_SOCKET_H

#include <ramses-capu/os/Posix/TcpSocket.h>

namespace ramses_capu
{
    namespace os
    {

        class TcpSocket: private ramses_capu::posix::TcpSocket
        {
        public:
            TcpSocket();
            TcpSocket(const SocketDescription& socketDescription);
            using ramses_capu::posix::TcpSocket::send;
            using ramses_capu::posix::TcpSocket::receive;
            using ramses_capu::posix::TcpSocket::close;
            using ramses_capu::posix::TcpSocket::connect;
            using ramses_capu::posix::TcpSocket::setBufferSize;
            using ramses_capu::posix::TcpSocket::setLingerOption;
            using ramses_capu::posix::TcpSocket::setNoDelay;
            using ramses_capu::posix::TcpSocket::setKeepAlive;
            using ramses_capu::posix::TcpSocket::setTimeout;
            using ramses_capu::posix::TcpSocket::getBufferSize;
            using ramses_capu::posix::TcpSocket::getLingerOption;
            using ramses_capu::posix::TcpSocket::getNoDelay;
            using ramses_capu::posix::TcpSocket::getKeepAlive;
            using ramses_capu::posix::TcpSocket::getTimeout;
            using ramses_capu::posix::TcpSocket::getRemoteAddress;
            using ramses_capu::posix::TcpSocket::getSocketDescription;

        };

        inline
        TcpSocket::TcpSocket()
        {
        }

        inline
        TcpSocket::TcpSocket(const SocketDescription& socketDescription)
            : ramses_capu::posix::TcpSocket(socketDescription)
        {

        }
    }
}
#endif // RAMSES_CAPU_ANDROID_TCP_SOCKET_H
