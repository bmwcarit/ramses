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
#include <mutex>
#include <chrono>

class AsyncSocketHandler
{
public:

    AsyncSocketHandler()
        : m_socketInfos()
        , m_receiveCount(0)
        , m_port(0)
    {
        m_serverSocket.bind(0, "0.0.0.0");
        m_serverSocket.listen(10);
        m_port = m_serverSocket.port();

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
        std::lock_guard<std::mutex> l(m_socketLock);
        UNUSED(socketDescription);
        ramses_capu::TcpSocket* clientSocket = m_serverSocket.accept();
        EXPECT_TRUE(0 != clientSocket);
        m_clientSockets.put(clientSocket->getSocketDescription(), clientSocket);
        m_socketInfos.push_back(ramses_capu::os::SocketInfoPair(clientSocket->getSocketDescription(), ramses_capu::os::SocketDelegate::Create<AsyncSocketHandler, &AsyncSocketHandler::receiveDataCallback>(*this)));
    }

    void sendSomeData()
    {
        std::lock_guard<std::mutex> l(m_socketLock);
        ramses_capu::HashTable<ramses_capu::os::SocketDescription, ramses_capu::TcpSocket*>::Iterator current = m_clientSockets.begin();
        ramses_capu::HashTable<ramses_capu::os::SocketDescription, ramses_capu::TcpSocket*>::Iterator end = m_clientSockets.end();

        for (; current != end; ++current)
        {
            uint32_t sendValue = 42;
            int32_t sendBytes;
            current->value->send(reinterpret_cast<char*>(&sendValue), sizeof(sendValue), sendBytes);
        }
    }

    void receiveDataCallback(const ramses_capu::os::SocketDescription& socketDescription)
    {
        std::lock_guard<std::mutex> g(m_socketLock);
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
    }

    ramses_capu::uint_t getNumberOfClientSockets()
    {
        std::lock_guard<std::mutex> l(m_socketLock);
        return m_clientSockets.count();
    }

    ramses_capu::vector<ramses_capu::os::SocketInfoPair> getSocketInfoCopy()
    {
        std::lock_guard<std::mutex> l(m_socketLock);
        return m_socketInfos;
    }

    ramses_capu::vector<ramses_capu::os::SocketInfoPair> m_socketInfos;
    ramses_capu::HashTable<ramses_capu::os::SocketDescription, ramses_capu::TcpSocket*> m_clientSockets;
    std::mutex m_socketLock;
    ramses_capu::TcpServerSocket m_serverSocket;
    uint32_t m_receiveCount;
    uint16_t m_port;
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
    const uint32_t clientcount = 10;
    const std::chrono::milliseconds testtimeout{5000};

    AsyncSocketHandler asyncSocketHandler;

    AsyncClient asyncClient[clientcount];

    for (uint32_t i = 0; i < clientcount; ++i)
    {
        asyncClient[i].start(asyncSocketHandler.m_port, false);
    }

    const auto startTime = std::chrono::steady_clock::now();
    bool timeout = false;
    while (asyncSocketHandler.getNumberOfClientSockets() < clientcount && !timeout)
    {
        ramses_capu::vector<ramses_capu::os::SocketInfoPair> sockets = asyncSocketHandler.getSocketInfoCopy();
        ramses_capu::NonBlockSocketChecker::CheckSocketsForIncomingData(sockets, 10);
        timeout = ((std::chrono::steady_clock::now() - startTime) > testtimeout);
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

    AsyncSocketHandler asyncSocketHandler;

    AsyncClient asyncClient[clientcount];

    for (uint32_t i = 0; i < clientcount; ++i)
    {
        asyncClient[i].start(asyncSocketHandler.m_port, true);
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
    AsyncSocketHandler asyncSocketHandler;

    AsyncClient asyncClient;
    asyncClient.start(asyncSocketHandler.m_port, false);

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
