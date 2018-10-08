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

#ifndef RAMSES_CAPU_UDP_SOCKET_H
#define RAMSES_CAPU_UDP_SOCKET_H

#include "ramses-capu/Config.h"
#include "ramses-capu/Error.h"
#include <ramses-capu/container/String.h>

#include <ramses-capu/os/PlatformInclude.h>
#include RAMSES_CAPU_PLATFORM_INCLUDE(UdpSocket)


namespace ramses_capu
{
    /**
     * UDP socket for IPv4
     */
    class UdpSocket: private ramses_capu::os::UdpSocket
    {
    public:

        /**
         *
         * @param port indicates port number
         * @param address address to bind if it is not given it accepts all connection from any address
         * @return CAPU_OK  if the server socket is successfully bound
         *         CAPU_SOCKET_EADDR if the addr is faulty
         *         CAPU_ERROR  if the socket is already bound
         *         CAPU_EINVAL if the port is equal to 0
         *         CAPU_SOCKET_ESOCKET if the socket has not been created successfully
         */
        status_t bind(const uint16_t port, const char* address = NULL);

        /**
         * Send the messages
         * @param buffer          the content of message that will be sent to destination
         * @param length          the length of message that will be sent to destination
         * @param receiverAddr    the destination
         * @return CAPU_OK if the sent is successful
         *         CAPU_EINVAL if the buffer is NULL
         *         CAPU_SOCKET_EADDR if the given address is not resolved.
         *         CAPU_SOCKET_ESOCKET if the socket has not been created successfully
         *         CAPU_ERROR otherwise
         */
        status_t send(const char* buffer, const int32_t length, const SocketAddrInfo& receiverAddr);

        /**
         * Send the messages
         * @param buffer          the content of message that will be sent to destination
         * @param length          the length of message that will be sent to destination
         * @param receiverAddr    the destination address
         * @param receiverPort    the destination port
         * @return CAPU_OK if the sent is successful
         *         CAPU_EINVAL if the buffer is NULL
         *         CAPU_SOCKET_EADDR if the given address is not resolved.
         *         CAPU_SOCKET_ESOCKET if the socket has not been created successfully
         *         CAPU_ERROR otherwise
         */
        status_t send(const char* buffer, const int32_t length, const char* receiverAddr, const uint16_t receiverPort);

        /**
         * Receive message
         * @param buffer    buffer that will be used to store incoming message
         * @param length    buffer size
         * @param numBytes  number of bytes on socket
         * @param sender    out parameter for the socket address of the sender
         * @return CAPU_OK if the receive is successfully executed
         *         CAPU_TIMEOUT if there has been a timeout
         *         CAPU_SOCKET_ESOCKET if the socket has not been created successfully
         *         CAPU_ERROR otherwise
         */
        status_t receive(char* buffer, const int32_t length, int32_t& numBytes, SocketAddrInfo* sender);

        /**
         * close the socket
         * @return CAPU_OK if the socket is correctly closed
         *         CAPU_SOCKET_ESOCKET if the socket has not been created successfully
         *         CAPU_ERROR otherwise
         */
        status_t close();

        /**
         * Sets the maximum socket buffer in bytes. The kernel doubles this value (to allow space for bookkeeping overhead)
         * Set the receive buffer size
         * Sets buffer size information.
         * @return CAPU_OK if the buffer is successfully set for both receive and send operations
         *         CAPU_SOCKET_ESOCKET if the socket has not been created successfully
         *         CAPU_ERROR otherwise
         */
        status_t setBufferSize(const int32_t bufferSize);

        /**
         * Set Timeout
         * Sets the timeout value that specifies the maximum amount of time an input function waits until it completes
         * @return CAPU_OK if the Timeout for receive operation is successfully set
         *         CAPU_SOCKET_ESOCKET if the socket has not been created successfully
         *         CAPU_ERROR otherwise
         */
        status_t setTimeout(const int32_t timeout);

        /**
         * Enable or disable the transmission of broadcast packets
         * @return CAPU_OK if the broadcast flag is successfully set
         *         CAPU_SOCKET_ESOCKET if the socket has not been created successfully
         *         CAPU_ERROR otherwise
         */
        status_t allowBroadcast(const bool broadcast);

        /**
         * get the send and receive buffer size
         * gets buffer size information.
         * @return CAPU_OK if the buffer is successfully set for both receive and send operations
         *         CAPU_SOCKET_ESOCKET if the socket has not been created successfully
         *         CAPU_ERROR otherwise
         */
        status_t getBufferSize(int32_t& bufferSize);

        /**
         * Get Timeout
         * @return CAPU_OK if the Timeout for receive operation is successfully set
         *         CAPU_SOCKET_ESOCKET if the socket has not been created successfully
         *         CAPU_ERROR otherwise
         */
        status_t getTimeout(int32_t& timeout);

        /**
         * Gets the address info for the bound socket. If the socket is not bound, the address info will be empty, which means that the port is 0 and the address is an empty string.
         * @return The socket info object.
         */
        const SocketAddrInfo& getSocketAddrInfo() const;

        /**
        * Returns the internal socket descriptor. This can be different on
        * different os
        *
        * @return the internal descriptor of the socket
        */
        inline const ramses_capu::os::SocketDescription& getSocketDescription() const;
    };

    inline
    status_t
    UdpSocket::bind(const uint16_t port, const char* addr)
    {
        return ramses_capu::os::UdpSocket::bind(port, addr);
    }

    inline
    status_t
    UdpSocket::send(const char* buffer, const int32_t length, const SocketAddrInfo& receiverAddr)
    {
        return ramses_capu::os::UdpSocket::send(buffer, length, receiverAddr);
    }

    inline
    status_t
    UdpSocket::send(const char* buffer, const int32_t length, const char* receiverAddr, const uint16_t receiverPort)
    {
        return ramses_capu::os::UdpSocket::send(buffer, length, receiverAddr, receiverPort);
    }

    inline
    status_t
    UdpSocket::receive(char* buffer, const int32_t length, int32_t& numBytes, SocketAddrInfo* sender)
    {
        return ramses_capu::os::UdpSocket::receive(buffer, length, numBytes, sender);
    }

    inline
    status_t
    UdpSocket::close()
    {
        return ramses_capu::os::UdpSocket::close();
    }

    inline
    status_t
    UdpSocket::setBufferSize(const int32_t bufferSize)
    {
        return ramses_capu::os::UdpSocket::setBufferSize(bufferSize);
    }

    inline
    status_t
    UdpSocket::setTimeout(const int32_t timeout)
    {
        return ramses_capu::os::UdpSocket::setTimeout(timeout);
    }

    inline
    status_t
    UdpSocket::allowBroadcast(const bool broadcast)
    {
        return ramses_capu::os::UdpSocket::allowBroadcast(broadcast);
    }

    inline
    status_t
    UdpSocket::getBufferSize(int32_t& bufferSize)
    {
        return ramses_capu::os::UdpSocket::getBufferSize(bufferSize);
    }

    inline
    status_t
    UdpSocket::getTimeout(int32_t& timeout)
    {
        return ramses_capu::os::UdpSocket::getTimeout(timeout);
    }

    inline
    const SocketAddrInfo&
    UdpSocket::getSocketAddrInfo() const
    {
        return ramses_capu::os::UdpSocket::getSocketAddrInfo();
    }

    inline
    const ramses_capu::os::SocketDescription&
    UdpSocket::getSocketDescription() const
    {
        return ramses_capu::os::UdpSocket::getSocketDescription();
    }
}

#endif /* RAMSES_CAPU_UDP_SOCKET_H */
