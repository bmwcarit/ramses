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
#include <mutex>
#include <condition_variable>

namespace ramses_capu_test
{
    namespace
    {
        std::mutex mutex;
        std::condition_variable cv;
        bool cond = false;
        uint16_t chosenPort = 0;
    }

    class ThreadClientTest : public ramses_capu::Runnable
    {
        uint16_t m_port;
    public:
        //client thread to test data exchange between client and server
        ThreadClientTest(uint16_t port) : m_port(port) {}

        void run()
        {
            int32_t communication_variable;
            int32_t numBytes = 0;
            //ALLOCATION AND SYNCH OF cient and  server
            ramses_capu::TcpSocket* cli_socket = new ramses_capu::TcpSocket();

            //wait for server to start up
            {
                std::unique_lock<std::mutex> l(mutex);
                cv.wait(l, [&](){ return cond; });
                cond = false;
                if (0 == m_port)
                {
                    m_port = chosenPort;
                }
            }
            EXPECT_LT(0, m_port);

            //TRY TO CONNECT TO IPV6
            EXPECT_EQ(ramses_capu::CAPU_SOCKET_EADDR, cli_socket->connect("::1", m_port));

            //connects to he given id
            ramses_capu::status_t result = ramses_capu::CAPU_ERROR;
            int32_t attemps = 0;
            while (result != ramses_capu::CAPU_OK && attemps < 100)
            {
                result = cli_socket->connect("localhost", m_port);
                attemps++;
                ramses_capu::Thread::Sleep(50);
            }
            EXPECT_EQ(ramses_capu::CAPU_OK, result);

            int32_t i = 5;
            int32_t sendData;
            //send data
            EXPECT_EQ(ramses_capu::CAPU_OK, cli_socket->send(reinterpret_cast<char*>(&i), sizeof(int32_t), sendData));

            //receive
            ramses_capu::status_t res = cli_socket->receive(reinterpret_cast<char*>(&communication_variable), sizeof(int32_t), numBytes);
            EXPECT_EQ(ramses_capu::CAPU_OK, res);

            //CHECK VALUE
            EXPECT_EQ(6, communication_variable);

            {
                std::lock_guard<std::mutex> l(mutex);
                cond = true;
                cv.notify_one();
            }

            //socket close
            EXPECT_EQ(ramses_capu::CAPU_OK, cli_socket->close());
            //deallocating
            delete cli_socket;
        }
    };

    class ThreadTimeoutOnReceiveClientTest : public ramses_capu::Runnable
    {
        uint16_t m_port;
    public:
        //timeout test
        ThreadTimeoutOnReceiveClientTest(uint16_t port) : m_port(port) {}

        void run()
        {
            int32_t communication_variable;
            int32_t numBytes = 0;
            ramses_capu::TcpSocket cli_socket;

            cli_socket.setTimeout(2);

            //connects to he given id
            ramses_capu::status_t result = ramses_capu::CAPU_ERROR;
            //wait for server to start up
            {
                std::unique_lock<std::mutex> l(mutex);
                cv.wait(l, [&](){ return cond; });
                cond = false;
                if (0 == m_port)
                {
                    m_port = chosenPort;
                }
            }
            result = cli_socket.connect("localhost", m_port);
            ASSERT_TRUE(result == ramses_capu::CAPU_OK);

            int32_t i = 5;

            int32_t sentBytes;
            //send data
            EXPECT_EQ(ramses_capu::CAPU_OK, cli_socket.send(reinterpret_cast<char*>(&i), sizeof(int32_t), sentBytes));

            //receive
            EXPECT_EQ(ramses_capu::CAPU_ETIMEOUT, cli_socket.receive(reinterpret_cast<char*>(&communication_variable), sizeof(int32_t), numBytes));

            //client has received timeout, server can close socket
            {
                std::lock_guard<std::mutex> l(mutex);
                cond = true;
                cv.notify_one();
            }
            //socket close
            EXPECT_EQ(ramses_capu::CAPU_OK, cli_socket.close());
        }
    };

    class ThreadTimeoutOnSendClientTest : public ramses_capu::Runnable
    {
        uint16_t m_port;
    public:
        //timeout test
        ThreadTimeoutOnSendClientTest(uint16_t port) : m_port(port) {}

    private:
        ramses_capu::TcpSocket cli_socket;

        /*
        TCP stacks on different platforms behave differently in case a timeout
        on send occurred. This function ensures that the combination of states
        are valid.
        */
        void testSendOnASocketWherePreviousSendTimedOut(ramses_capu::status_t previousErrorOnSend)
        {

            //try to send again on broken socket
            char data = 5;
            int32_t sentBytes = 0;
            ramses_capu::status_t currentErrorOnSend = cli_socket.send(&data, sizeof(int32_t), sentBytes);
            if (previousErrorOnSend == ramses_capu::CAPU_ERROR)
            {
                EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, currentErrorOnSend);
            }

            if (previousErrorOnSend == ramses_capu::CAPU_ETIMEOUT)
            {
                EXPECT_EQ(ramses_capu::CAPU_ETIMEOUT, currentErrorOnSend);
            }

        }

        void testCloseSocketOnASocketWherePreviousSendTimedOut(ramses_capu::status_t previousErrorOnSend)
        {
            if (previousErrorOnSend == ramses_capu::CAPU_ERROR)
            {
                EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, cli_socket.close());
            }

            if (previousErrorOnSend == ramses_capu::CAPU_ETIMEOUT)
            {
                EXPECT_EQ(ramses_capu::CAPU_OK, cli_socket.close());
            }
        }

    public:

        void run()
        {
            cli_socket.setTimeout(200);

            //wait for server to start up
            {
                std::unique_lock<std::mutex> l(mutex);
                cv.wait(l, [&](){ return cond; });
                cond = false;
                if (0 == m_port)
                {
                    m_port = chosenPort;
                }
            }

            ramses_capu::status_t connectStatus = ramses_capu::CAPU_ERROR;
            while (connectStatus != ramses_capu::CAPU_OK) {
                connectStatus = cli_socket.connect("localhost", m_port);
                if (connectStatus != ramses_capu::CAPU_OK) {
                    ramses_capu::Thread::Sleep(100);
                }
            }

            const uint32_t dataSize = 1024 * 1024 / 8;
            uint32_t messageCount = 100;

            char* data = new char[dataSize];

            ramses_capu::Memory::Set(data, 0x00000000, dataSize);

            int32_t sentBytes;
            ramses_capu::status_t status;

            while (messageCount--)
            {
                status = cli_socket.send(data, dataSize, sentBytes);
                if (status == ramses_capu::CAPU_ERROR || status == ramses_capu::CAPU_ETIMEOUT) {
                    break;
                }
            }

            //Some TCP stacks a timeout on send returns a CAPU_ERROR as the connection has been closed.
            //Other stacks return a CAPU_ETIMEOUT and the socket can be still used.
            EXPECT_THAT(status, testing::AnyOf(ramses_capu::CAPU_ERROR, ramses_capu::CAPU_ETIMEOUT));

            testSendOnASocketWherePreviousSendTimedOut(status);

            {
                std::lock_guard<std::mutex> l(mutex);
                cond = true;
                cv.notify_one();
            }

            testCloseSocketOnASocketWherePreviousSendTimedOut(status);

            delete[] data;
        }
    };

    class ThreadServerTest : public ramses_capu::Runnable
    {
        uint16_t m_port;
    public:
        //SERVER thread to test data exchange between client and server
        ThreadServerTest(uint16_t port) : m_port(port) {}

        void run()
        {
            int32_t communication_variable;
            int32_t numBytes = 0;
            //server socket allocation
            ramses_capu::TcpServerSocket* socket = new ramses_capu::TcpServerSocket();

            //bind to given address
            EXPECT_EQ(ramses_capu::CAPU_OK, socket->bind(m_port, "0.0.0.0"));

            if (0 == m_port)
            {
                m_port = socket->port();
                chosenPort = m_port;
            }

            //start listening
            EXPECT_EQ(ramses_capu::CAPU_OK, socket->listen(5));
            //accept connection

            //server is ready to accept clients
            {
                std::lock_guard<std::mutex> l(mutex);
                cond = true;
                cv.notify_one();
            }
            ramses_capu::TcpSocket* new_socket = socket->accept();

            //receive data
            ramses_capu::status_t result = ramses_capu::CAPU_ERROR;

            result = new_socket->receive(reinterpret_cast<char*>(&communication_variable), sizeof(int32_t), numBytes);
            EXPECT_EQ(ramses_capu::CAPU_OK, result);
            //CHECK VALUE
            EXPECT_EQ(5, communication_variable);
            //update data
            communication_variable++;


            int32_t sentBytes;
            //send it back
            EXPECT_EQ(ramses_capu::CAPU_OK, new_socket->send(reinterpret_cast<char*>(&communication_variable), sizeof(int32_t), sentBytes));

            //wait with close until client has received data
            {
                std::unique_lock<std::mutex> l(mutex);
                cv.wait(l, [&](){ return cond; });
                cond = false;
            }

            //close session
            EXPECT_EQ(ramses_capu::CAPU_OK, new_socket->close());
            //deallocate session identifier
            delete new_socket;
            EXPECT_EQ(ramses_capu::CAPU_OK, socket->close());
            delete socket;
        }
    };

    class ThreadTimeoutOnReceiveServerTest : public ramses_capu::Runnable
    {
        uint16_t mPort;

    public:
        //timeout test
        ThreadTimeoutOnReceiveServerTest(uint16_t port) : mPort(port) {}
        inline void run()
        {
            int32_t communication_variable;
            int32_t numBytes = 0;
            //server socket allocation
            ramses_capu::TcpServerSocket* socket = new ramses_capu::TcpServerSocket();

            //bind to given address
            EXPECT_EQ(ramses_capu::CAPU_OK, socket->bind(mPort, "0.0.0.0"));

            if (0 == mPort)
            {
                mPort = socket->port();
                chosenPort = mPort;
            }

            //start listening
            EXPECT_EQ(ramses_capu::CAPU_OK, socket->listen(5));

            //server is ready to accept clients
            {
                std::lock_guard<std::mutex> l(mutex);
                cond = true;
                cv.notify_one();
            }
            //accept connection
            ramses_capu::TcpSocket* new_socket = socket->accept();
            ramses_capu::status_t result = ramses_capu::CAPU_ERROR;

            result = new_socket->receive(reinterpret_cast<char*>(&communication_variable), sizeof(int32_t), numBytes);
            EXPECT_EQ(ramses_capu::CAPU_OK, result);
            //CHECK VALUE
            EXPECT_EQ(5, communication_variable);

            //wait for timeout on client side
            {
                std::unique_lock<std::mutex> l(mutex);
                cv.wait(l, [&](){ return cond; });
                cond = false;
            }

            //close session
            EXPECT_EQ(ramses_capu::CAPU_OK, new_socket->close());
            //deallocate session identifier
            delete new_socket;
            EXPECT_EQ(ramses_capu::CAPU_OK, socket->close());
            delete socket;
        }
    };

    class ThreadTimeoutOnSendServerTest : public ramses_capu::Runnable
    {
        uint16_t mPort;

    public:
        //timeout test
        ThreadTimeoutOnSendServerTest(uint16_t port) : mPort(port) {}
        inline void run()
        {
            //server socket allocation
            ramses_capu::TcpServerSocket* socket = new ramses_capu::TcpServerSocket();

            //bind to given address
            EXPECT_EQ(ramses_capu::CAPU_OK, socket->bind(mPort, "0.0.0.0"));

            if (0 == mPort)
            {
                mPort = socket->port();
                chosenPort = mPort;
            }

            //start listening
            EXPECT_EQ(ramses_capu::CAPU_OK, socket->listen(5));

            //server is ready to accept clients
            {
                std::lock_guard<std::mutex> l(mutex);
                cond = true;
                cv.notify_one();
            }
            //accept connection
            ramses_capu::TcpSocket* new_socket = socket->accept();

            //do not call receive to cause a timeout on sender side

            //wait for timeout on client side
            {
                std::unique_lock<std::mutex> l(mutex);
                cv.wait(l, [&](){ return cond; });
                cond = false;
            }

            //close session
            EXPECT_EQ(ramses_capu::CAPU_OK, new_socket->close());
            //deallocate session identifier
            delete new_socket;
            EXPECT_EQ(ramses_capu::CAPU_OK, socket->close());
            delete socket;
        }
    };

    class ThreadReconnectServerTest : public ramses_capu::Runnable
    {
        uint16_t mPort;
        ramses_capu::TcpServerSocket mSocket;
    public:
        //timeout test
        ThreadReconnectServerTest(uint16_t port) : mPort(port)
        {

            //bind to given address
            EXPECT_EQ(ramses_capu::CAPU_OK, mSocket.bind(mPort, "0.0.0.0"));
            if (0 == mPort)
            {
                mPort = mSocket.port();
            }

            //start listening
            EXPECT_EQ(ramses_capu::CAPU_OK, mSocket.listen(5));
        }

        ~ThreadReconnectServerTest()
        {
            mSocket.close();
        }

        inline void run()
        {
            int32_t communication_variable;
            int32_t numBytes = 0;

            ramses_capu::TcpSocket* new_socket = mSocket.accept();
            ramses_capu::status_t result = ramses_capu::CAPU_ERROR;

            result = new_socket->receive(reinterpret_cast<char*>(&communication_variable), sizeof(int32_t), numBytes);
            EXPECT_EQ(ramses_capu::CAPU_OK, result);
            //CHECK VALUE
            EXPECT_EQ(5, communication_variable);

            //close session
            EXPECT_EQ(ramses_capu::CAPU_OK, new_socket->close());
            //deallocate session identifier
            delete new_socket;

        }

        inline uint16_t port()
        {
            return mPort;
        }
    };

    TEST(TcpSocket, ConnectTest)
    {
        ramses_capu::TcpSocket* socket = new ramses_capu::TcpSocket();
        //pass null
        EXPECT_EQ(ramses_capu::CAPU_EINVAL, socket->connect(NULL, 22));
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_EADDR, socket->connect("www.test", 22));

        delete socket;
    }

    TEST(TcpSocket, ConnectTwiceTest)
    {
        ramses_capu::TcpSocket socket;
        socket.setTimeout(50);
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ECONNECT, socket.connect("127.0.0.1", 6556));
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ECONNECT, socket.connect("127.0.0.1", 6556));
    }

    TEST(TcpSocket, UnconnectedSocketCloseReceiveAndSendTest)
    {
        ramses_capu::TcpSocket* socket = new ramses_capu::TcpSocket();
        int32_t i = 0;
        int32_t numBytes = 0;
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->close());
        //try to send data via closed socket
        int32_t sentBytes;
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->send(static_cast<const char*>("asda"), 4, sentBytes));
        //try to receive data from closed socket
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->receive(reinterpret_cast<char*>(&i), 4, numBytes));
        //Deallocation of socket
        delete socket;
    }

    TEST(TcpSocket, SetAndGetPropertiesTest)
    {
        ramses_capu::TcpSocket* socket = new ramses_capu::TcpSocket();
        ramses_capu::TcpServerSocket* serverSocket = new ramses_capu::TcpServerSocket();
        //TRY TO CHANGE THE PROPERTIES OF NOT CONNECTED SOCKET
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->setBufferSize(1024));
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->setKeepAlive(true));
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->setLingerOption(true, 90));
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->setNoDelay(false));
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->setTimeout(90));

        int32_t int_tmp = 0;
        uint16_t short_tmp = 0;
        bool boolmp = false;
        char* remoteIP = 0;

        //CHECK THE PROPERTIES ARE CORRECTLY SET
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->getBufferSize(int_tmp));

        //On Linux the kernel adjusts the buffer size and set it to doubles of given size (at least)
        //therefore we have to check here for >=
        EXPECT_TRUE(int_tmp >= 1024);
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->getKeepAlive(boolmp));
        EXPECT_TRUE(boolmp);
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->getLingerOption(boolmp, short_tmp));
        EXPECT_EQ(90, short_tmp);
        EXPECT_TRUE(boolmp);
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->getNoDelay(boolmp));
        EXPECT_FALSE(boolmp);
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->getTimeout(int_tmp));
        EXPECT_EQ(90, int_tmp);

        serverSocket->bind(0, "0.0.0.0");
        uint16_t port = serverSocket->port();
        serverSocket->listen(3);

        socket->connect("127.0.0.1", port);

        //TRY TO CHANGE THE PROPERTIES OF CONNECTED SOCKET
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->setBufferSize(2024));
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->setKeepAlive(false));

        EXPECT_EQ(ramses_capu::CAPU_OK, socket->setLingerOption(false, 0));
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->setNoDelay(true));
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->setTimeout(100));

        //CHECK THE PROPERTIES ARE CORRECTLY SET
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->getBufferSize(int_tmp));
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->getRemoteAddress(&remoteIP));
        EXPECT_STREQ("127.0.0.1", remoteIP);

        //kernel adjusts the buffer size and set it to doubles of given size (at least)
        EXPECT_TRUE(int_tmp >= 2024);
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->getKeepAlive(boolmp));
        EXPECT_FALSE(boolmp);
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->getLingerOption(boolmp, short_tmp));
        EXPECT_FALSE(boolmp);
        EXPECT_EQ(short_tmp, 0);
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->getNoDelay(boolmp));
        EXPECT_TRUE(boolmp);
        EXPECT_EQ(ramses_capu::CAPU_OK, socket->getTimeout(int_tmp));
        EXPECT_GE(int_tmp, 100);
        EXPECT_LT(int_tmp, 110);

        socket->close();
        //TRY TO CHANGE THE PROPERTIES OF CLOSED SOCKET
        /*
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->setBufferSize(1024) );
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->setKeepAlive(true) );
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->setLingerOption(true, 90) );
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->setNoDelay(false) );
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->setTimeout(90) );
        //TRY TO GET PROPERTIES OF CLOSED SOCKET
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->getBufferSize(int_tmp) );
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->getKeepAlive(boolmp) );
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->getLingerOption(boolmp, int_tmp) );
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->getNoDelay(boolmp) );
        EXPECT_EQ(ramses_capu::CAPU_SOCKET_ESOCKET, socket->getTimeout(int_tmp) );
         */

        delete socket;
        delete serverSocket;
        delete[] remoteIP;
    }

    TEST(SocketAndTcpServerSocket, CommunicationTest)
    {
        cond = false;
        ThreadServerTest server(0);
        ramses_capu::Thread* server_thread = new ramses_capu::Thread();
        server_thread->start(server);

        ThreadClientTest client(0);
        ramses_capu::Thread* client_thread = new ramses_capu::Thread();
        client_thread->start(client);

        //Create two threads which will behave like client and server to test functionality
        server_thread->join();
        client_thread->join();

        delete client_thread;
        delete server_thread;
    }

    TEST(SocketAndTcpServerSocket, ReconnectTest)
    {
        cond = false;
        ThreadReconnectServerTest server(0);
        uint16_t port = server.port();
        ASSERT_LT(0u, port);
        ramses_capu::Thread server_thread;

        ramses_capu::TcpSocket socket;
        int32_t buffer = 5;
        int32_t sentBytes;

        for (int32_t i = 0; i < 5; ++i)
        {
            server_thread.start(server);
            socket.connect("127.0.0.1", port);
            socket.send(reinterpret_cast<char*>(&buffer), sizeof(int32_t), sentBytes);
            server_thread.join();
        }

    }

    TEST(SocketAndTcpServerSocket, TimeoutOnReceiveTest)
    {
        cond = false;

        ThreadTimeoutOnReceiveServerTest server(0);
        ramses_capu::Thread* server_thread = new ramses_capu::Thread();
        server_thread->start(server);

        ThreadTimeoutOnReceiveClientTest client(0);
        ramses_capu::Thread* client_thread = new ramses_capu::Thread();
        client_thread->start(client);

        //client_thread two threads which will behave like client and server to test functionality
        server_thread->join();
        client_thread->join();

        delete client_thread;
        delete server_thread;
    }

    TEST(SocketAndTcpServerSocket, TimeoutOnSendTest)
    {
        cond = false;

        ThreadTimeoutOnSendServerTest server(0);
        ramses_capu::Thread* server_thread = new ramses_capu::Thread();
        server_thread->start(server);

        ThreadTimeoutOnSendClientTest client(0);
        ramses_capu::Thread* client_thread = new ramses_capu::Thread();
        client_thread->start(client);

        //client_thread two threads which will behave like client and server to test functionality
        server_thread->join();
        client_thread->join();

        delete client_thread;
        delete server_thread;
    }

    class TestServer : public ramses_capu::Runnable
    {
    public:
        ramses_capu::TcpServerSocket server;
        uint16_t port;
        int32_t receivedLength;
        ramses_capu::status_t receivedRetVal;
        ramses_capu::Thread t;

        TestServer() : server(), port(0), receivedLength(0), receivedRetVal(0)
        {
            server.bind(port);
            port = server.port();
            server.listen(6);

            t.start(*this);
        }

        void waitForReceived()
        {
            t.join();
        }

        void run()
        {
            ramses_capu::TcpSocket* client = server.accept();
            char buffer[1000];
            receivedRetVal = client->receive(buffer, sizeof(buffer), receivedLength);
            delete client;
        }
    };

    TEST(SocketAndTcpServerSocket, ReceiveReturnsOkOnClose)
    {
        TestServer server;
        ramses_capu::TcpSocket client;
        client.connect("127.0.0.1", server.port);
        client.close();
        server.waitForReceived();
        EXPECT_EQ(ramses_capu::CAPU_OK, server.receivedRetVal);
        EXPECT_EQ(0, server.receivedLength);
    }
}
