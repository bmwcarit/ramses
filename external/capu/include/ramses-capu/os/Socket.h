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

#ifndef RAMSES_CAPU_SOCKET_H
#define RAMSES_CAPU_SOCKET_H

#include "ramses-capu/Config.h"
#include "ramses-capu/container/String.h"

#include "ramses-capu/os/PlatformInclude.h"
#include RAMSES_CAPU_PLATFORM_INCLUDE(Socket)

namespace ramses_capu
{

    /**
     * Struct describing a socket address.
     */
    struct SocketAddrInfo
    {
        /**
         * Constructor.
         */
        SocketAddrInfo()
            : port(0)
        {
        }

        /**
         * The port number for the socket
         */
        uint16_t port;

        /**
         * The IP Address to use for the socket
         */
        String addr;
    };
}
#endif // RAMSES_CAPU_SOCKET_H
