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

#ifndef RAMSES_CAPU_ANDROID_UDP_SOCKET_H
#define RAMSES_CAPU_ANDROID_UDP_SOCKET_H

#include <ramses-capu/os/Posix/UdpSocket.h>

namespace ramses_capu
{
    namespace os
    {
        class UdpSocket: private ramses_capu::posix::UdpSocket
        {
        public:
            using ramses_capu::posix::UdpSocket::bind;
            using ramses_capu::posix::UdpSocket::send;
            using ramses_capu::posix::UdpSocket::receive;
            using ramses_capu::posix::UdpSocket::close;
            using ramses_capu::posix::UdpSocket::setBufferSize;
            using ramses_capu::posix::UdpSocket::setTimeout;
            using ramses_capu::posix::UdpSocket::allowBroadcast;
            using ramses_capu::posix::UdpSocket::getBufferSize;
            using ramses_capu::posix::UdpSocket::getTimeout;
            using ramses_capu::posix::UdpSocket::getSocketAddrInfo;
            using ramses_capu::posix::UdpSocket::getSocketDescription;
        };
    }
}

#endif // RAMSES_CAPU_ANDROID_UDP_SOCKET_H
