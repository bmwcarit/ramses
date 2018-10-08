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

#ifndef RAMSES_CAPU_LINUX_TCPSERVERSOCKET_H
#define RAMSES_CAPU_LINUX_TCPSERVERSOCKET_H


#include <ramses-capu/os/Posix/TcpServerSocket.h>

namespace ramses_capu
{
    namespace os
    {
        class TcpServerSocket: private ramses_capu::posix::TcpServerSocket
        {
        public:
            using ramses_capu::posix::TcpServerSocket::accept;
            using ramses_capu::posix::TcpServerSocket::close;
            using ramses_capu::posix::TcpServerSocket::bind;
            using ramses_capu::posix::TcpServerSocket::listen;
            using ramses_capu::posix::TcpServerSocket::port;
            using ramses_capu::posix::TcpServerSocket::getSocketDescription;
        };
    }
}
#endif // RAMSES_CAPU_LINUX_TCPSERVERSOCKET_H
