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

#ifndef RAMSES_CAPU_TCPSERVERSOCKET_H
#define RAMSES_CAPU_TCPSERVERSOCKET_H

#include <ramses-capu/os/PlatformInclude.h>
#include "ramses-capu/os/TcpSocket.h"
#include "ramses-capu/Error.h"

#include RAMSES_CAPU_PLATFORM_INCLUDE(TcpServerSocket)

namespace ramses_capu
{
    /**
     * Server side socket. Creates socket connections by waiting for connections.
     */
    class TcpServerSocket: private ramses_capu::os::TcpServerSocket
    {
    public:

        /**
         * The program flow will be blocked until a connection arrives
         * Programmer is responsible for deallocating memory of returning socket.
         *
         * @param timeoutMillis accept timeout. Default value is 0, which means to block forever.
         * @return TcpSocket if a connection is accepted
         *         NULL otherwise
         */
        TcpSocket* accept(uint32_t timeoutMillis = 0);

        /**
         * Close the socket which is used for accepting connection
         *
         * @return CAPU_OK if the socket is successfully closed
         *         CAPU_SOCKET_ESOCKET if the socket is not created
         */
        status_t close();

        /**
         * Binds the server to the given address and port.
         *
         * @param port indicates port number. 0 means the OS chooses a random free port.
         *             The chosen port can be determined by calling port().
         * @param address address to bind if it is not given it accepts all connection from any address
         * @param reuseAddr if set to true the socket can be bound to a socket that is already in use
         * @return CAPU_OK  if the server socket is successfully bound
         *         CAPU_SOCKET_EADDR if the addr is faulty
         *         CAPU_ERROR  if the socket is already bound
         *         CAPU_EINVAL if the addr is NULL or port is equal to 0
         *         CAPU_SOCKET_ESOCKET if the socket is not created
         */
        status_t bind(uint16_t port, const char* address = NULL, bool reuseAddr = true);

        /**
         *
         * @param backlog (maximum length of the queue of pending connections)
         * @return CAPU_OK if the listen is successful
         *         CAPU_SOCKET_ESOCKET if the socket is not created
         *         CAPU_ERROR otherwise
         */
        status_t listen(uint8_t backlog);

        /**
         * Returns the port to which the server socket is bound.
         *
         * @return The port to which the server socket is bound or 0 if the server socket
         *         is not bound yet.
         */
        uint16_t port();

        /**
         * Returns the internal descriptor of the socket. This can be different on
         * different operating systems.
         *
         * @return The internal descriptor of the socket
         */
        const ramses_capu::os::SocketDescription& getSocketDescription() const;
    };

    inline
    TcpSocket*
    TcpServerSocket::accept(uint32_t timeoutMillis)
    {
        return ramses_capu::os::TcpServerSocket::accept(timeoutMillis);
    }

    inline
    status_t
    TcpServerSocket::close()
    {
        return ramses_capu::os::TcpServerSocket::close();
    }

    inline
    status_t
    TcpServerSocket::bind(uint16_t port, const char* addr, bool reuseAddr)
    {
        return ramses_capu::os::TcpServerSocket::bind(port, addr, reuseAddr);
    }

    inline
    status_t
    TcpServerSocket::listen(uint8_t backlog)
    {
        return ramses_capu::os::TcpServerSocket::listen(backlog);
    }

    inline
    uint16_t
    TcpServerSocket::port()
    {
        return ramses_capu::os::TcpServerSocket::port();
    }

    inline
    const ramses_capu::os::SocketDescription&
    TcpServerSocket::getSocketDescription() const
    {
        return ramses_capu::os::TcpServerSocket::getSocketDescription();
    }
}
#endif //RAMSES_CAPU_TCPSERVERSOCKET_H
