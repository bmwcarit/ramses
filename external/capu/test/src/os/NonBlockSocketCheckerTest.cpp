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

#include <gtest/gtest.h>
#include "ramses-capu/os/TcpServerSocket.h"
#include "ramses-capu/os/Thread.h"
#include "ramses-capu/os/TcpSocket.h"
#include "gmock/gmock-matchers.h"
#include "gmock/gmock-generated-matchers.h"
#include "ramses-capu/container/HashTable.h"
#include "ramses-capu/os/NonBlockSocketChecker.h"
#include "ramses-capu/os/Time.h"


class RandomPort
{
public:
    /**
    * Gets a Random Port between 1024 and 10024
    */
    static uint16_t get()
    {
        return (rand() % 10000) + 40000; // 0-1023 = Well Known, 1024-49151 = User, 49152 - 65535 = Dynamic
    }
};

class AsyncSocketHandler
{
public:

    AsyncSocketHandler(uint16_t port)
        : m_socketInfos()
        , m_receiveCount(0)
    {
        m_serverSocket.bind(port, "0.0.0.0");
        m_serverSocket.listen(10);

        m_socketInfos.push_back(ramses_capu::os::SocketInfoPair(m_serverSocket.getSocketDescription(), ramses_capu::os::SocketDelegate::Create<AsyncSocketHandler, &AsyncSocketHandler::acceptConnectionCallback>(*this)));
    }

    ~AsyncSocketHandler()
    {
        ramses_capu::HashTable<ramses_capu::os::SocketDescription, ramses_capu::TcpSocket*>::Iterator current = m_clientSockets.begin();
        const ramses_capu::HashTable<ramses_capu::os::SocketDescription, ramses_capu::TcpSocket*>::Iterator end = m_clientSockets.end();

        for (; current != end; ++current)
        {
            current->value->close();
            delete current->value;
        }
    }

    void acceptConnectionCallback(const ramses_capu::os::SocketDescription& socketDescription)
    {
        m_socketLock.lock();
        UNUSED(socketDescription);
        ramses_capu::TcpSocket* clientSocket = m_serverSocket.accept();
        EXPECT_TRUE(0 != clientSocket);
        m_clientSockets.put(clientSocket->getSocketDescription(), clientSocket);
        m_socketInfos.push_back(ramses_capu::os::SocketInfoPair(clientSocket->getSocketDescription(), ramses_capu::os::SocketDelegate::Create<AsyncSocketHandler, &AsyncSocketHandler::receiveDataCallback>(*this)));
        m_socketLock.unlock();
    }

    void sendSomeData()
    {
        m_socketLock.lock();
        ramses_capu::HashTable<ramses_capu::os::SocketDescription, ramses_capu::TcpSocket*>::Iterator current = m_clientSockets.begin();
        ramses_capu::HashTable<ramses_capu::os::SocketDescription, ramses_capu::TcpSocket*>::Iterator end = m_clientSockets.end();

        for (; current != end; ++current)
        {
            uint32_t sendValue = 42;
            int32_t sendBytes;
            current->value->send(reinterpret_cast<char*>(&sendValue), sizeof(sendValue), sendBytes);
        }
        m_socketLock.unlock();
    }

    void receiveDataCallback(const ramses_capu::os::SocketDescription& socketDescription)
    {
        m_socketLock.lock();
        int32_t data;
        int32_t numbytes = 0;

        ramses_capu::HashTable<ramses_capu::os::SocketDescription, ramses_capu::TcpSocket*>::Iterator entry = m_clientSockets.find(socketDescription);
        if (entry != m_clientSockets.end())
        {
            EXPECT_EQ(ramses_capu::CAPU_OK, entry->value->receive(reinterpret_cast<char*>(&data), sizeof(data), numbytes));
            if (numbytes != 0)
            {
                EXPECT_EQ(42, data);
                ++m_receiveCount;
            }
        }
        m_socketLock.unlock();
    }

    ramses_capu::uint_t getNumberOfClientSockets()
    {
        m_socketLock.lock();
        const ramses_capu::uint_t num = m_clientSockets.count();
        m_socketLock.unlock();
        return num;
    }

    ramses_capu::vector<ramses_capu::os::SocketInfoPair> getSocketInfoCopy()
    {
        m_socketLock.lock();
        const ramses_capu::vector<ramses_capu::os::SocketInfoPair> copy = m_socketInfos;
        m_socketLock.unlock();
        return copy;
    }

    ramses_capu::vector<ramses_capu::os::SocketInfoPair> m_socketInfos;
    ramses_capu::HashTable<ramses_capu::os::SocketDescription, ramses_capu::TcpSocket*> m_clientSockets;
    ramses_capu::Mutex m_socketLock;
    ramses_capu::TcpServerSocket m_serverSocket;
    uint32_t m_receiveCount;
};


class AsyncClient : public ramses_capu::Runnable
{
public:
    AsyncClient()
        : m_port(0)
        , m_send(false)
        , m_receiveCount(0)
    {
    }

    void receiveSomeData(const ramses_capu::os::SocketDescription&)
    {
        int32_t data;
        int32_t numbytes = 0;

        EXPECT_EQ(ramses_capu::CAPU_OK, m_clientSocket.receive(reinterpret_cast<char*>(&data), sizeof(data), numbytes));
        if (numbytes!=0)
        {
            EXPECT_EQ(42, data);
            ++m_receiveCount;
        }
    }

    void run()
    {
        m_clientSocket.setTimeout(1000);

        while (m_clientSocket.connect("localhost", m_port) != ramses_capu::CAPU_OK){
            if (isCancelRequested())
                return;
        }

        m_socketInfos.push_back(ramses_capu::os::SocketInfoPair(m_clientSocket.getSocketDescription(), ramses_capu::os::SocketDelegate::Create<AsyncClient, &AsyncClient::receiveSomeData>(*this)));

        if (m_send)
        {
            uint32_t sendValue = 42;
            int32_t sendBytes = -1;
            m_clientSocket.send(reinterpret_cast<char*>(&sendValue), sizeof(sendValue), sendBytes);
            EXPECT_EQ(static_cast<int32_t>(sizeof(sendValue)), sendBytes);
        }
    }

    void start(uint16_t port, bool send)
    {
        m_port = port;
        m_send = send;
        m_thread.start(*this);
    }

    void stop()
    {
        m_thread.cancel();
        m_thread.join();
        m_clientSocket.close();
    }

    ramses_capu::TcpSocket* getSocket()
    {
        return &m_clientSocket;
    }

    uint16_t m_port;
    bool m_send;
    ramses_capu::TcpSocket m_clientSocket;
    ramses_capu::Thread m_thread;
    ramses_capu::vector<ramses_capu::os::SocketInfoPair> m_socketInfos;
    uint32_t m_receiveCount;
};

TEST(NonBlockSocketCheckerTest, DISABLED_AcceptALotOfClients)
{
    static const uint32_t clientcount = 10;
    static const uint64_t testtimeout = 5000;

    uint16_t port = RandomPort::get();

    AsyncSocketHandler asyncSocketHandler(port);

    AsyncClient asyncClient[clientcount];

    for (uint32_t i = 0; i < clientcount; ++i)
    {
        asyncClient[i].start(port, false);
    }

    uint64_t startTime = ramses_capu::Time::GetMillisecondsMonotonic();
    bool timeout = false;
    while (asyncSocketHandler.getNumberOfClientSockets() < clientcount && !timeout)
    {
        ramses_capu::vector<ramses_capu::os::SocketInfoPair> sockets = asyncSocketHandler.getSocketInfoCopy();
        ramses_capu::NonBlockSocketChecker::CheckSocketsForIncomingData(sockets, 10);
        timeout = ((ramses_capu::Time::GetMillisecondsMonotonic() - startTime) > testtimeout);
    }

    for (uint32_t i = 0; i < clientcount; ++i)
    {
        asyncClient[i].stop();
    }

    EXPECT_FALSE(timeout);
}

TEST(NonBlockSocketCheckerTest, DISABLED_ReceiveDataFromALotOfClients)
{
    static const uint32_t clientcount = 50;

    uint16_t port = RandomPort::get();

    AsyncSocketHandler asyncSocketHandler(port);


    AsyncClient asyncClient[clientcount];

    for (uint32_t i = 0; i < clientcount; ++i)
    {
        asyncClient[i].start(port, true);
    }

    while (asyncSocketHandler.m_receiveCount < clientcount)
    {
        ramses_capu::NonBlockSocketChecker::CheckSocketsForIncomingData(asyncSocketHandler.m_socketInfos, 10);
    }

    for (uint32_t i = 0; i < clientcount; ++i)
    {
        asyncClient[i].stop();
    }
}

TEST(NonBlockSocketCheckerTest, DISABLED_ReceiveDataOnClientSide)
{
    uint16_t port = RandomPort::get();

    AsyncSocketHandler asyncSocketHandler(port);

    AsyncClient asyncClient;
    asyncClient.start(port, false);

    while (asyncSocketHandler.m_clientSockets.count() < 1)
    {
        ramses_capu::NonBlockSocketChecker::CheckSocketsForIncomingData(asyncSocketHandler.m_socketInfos, 0);
    }

    asyncSocketHandler.sendSomeData();

    while (asyncClient.m_receiveCount < 1)
    {
        ramses_capu::NonBlockSocketChecker::CheckSocketsForIncomingData(asyncClient.m_socketInfos, 1000);
    }

    asyncClient.stop();
}
