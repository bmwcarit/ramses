//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/ConnectionSystemBase.h"
#include "TransportCommon/IConnectionStatusListener.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "PlatformAbstraction/VariantWrapper.h"
#include "ScopedConsoleLogDisable.h"
#include "gmock/gmock.h"
#include <deque>

namespace ramses_internal
{
    namespace ThreadedStressTestInternal
    {
        // Test instance to avoid dependeing on ramses or dcsm connection systems
        using TestInstanceId = StronglyTypedValue<uint16_t, 0xFFFF, struct TestInstanceIdTag>;

        // concrete callback classs with extra test callback
        class TestCallbacks : public ISomeIPStackCallbacksCommon<TestInstanceId>
        {
        public:
            virtual void handleTestMessage(const SomeIPMsgHeader& header, uint32_t arg) = 0;
        };

        // messages between connection systems (aka messages over someip)
        namespace Message
        {
            struct Available
            {
                TestInstanceId iid;
            };

            struct Unavailable
            {
                TestInstanceId iid;
            };

            struct ParticipantInfo
            {
                TestInstanceId to;
                SomeIPMsgHeader hdr;
                uint16_t protocolVersion;
                uint32_t minorProtocolVersion;
                TestInstanceId senderInstanceId;
                uint64_t expectedReceiverPid;
                uint8_t clockType;
                uint64_t timestampNow;
            };

            struct KeepAlive
            {
                TestInstanceId to;
                SomeIPMsgHeader hdr;
                uint64_t timestampNow;
                bool usingPreviousMessageId;
            };

            struct Test
            {
                TestInstanceId to;
                SomeIPMsgHeader hdr;
                uint32_t        arg;
            };

            using Type = absl::variant<absl::monostate, Available, Unavailable, ParticipantInfo, KeepAlive, Test>;
        }

        // event from connnection system to user
        namespace Event
        {
            struct NewParticipant
            {
                Guid pid;
                bool operator==(NewParticipant other) const
                {
                    return pid == other.pid;
                }
            };

            struct ParticipantGone
            {
                Guid pid;
                bool operator==(ParticipantGone other) const
                {
                    return pid == other.pid;
                }
            };

            struct TestMsg
            {
                Guid pid;
                uint32_t arg;
                bool operator==(TestMsg other) const
                {
                    return pid == other.pid && arg == other.arg;
                }
            };

            using Type = absl::variant<absl::monostate, NewParticipant, ParticipantGone, TestMsg>;
        }

        // thread safe blocking queue
        template <typename T>
        class BlockingQueue
        {
        public:
            void push(T msg)
            {
                std::lock_guard<std::mutex> l(m_mutex);
                m_data.push_back(std::move(msg));
                m_condVar.notify_one();
            }

            T pop(std::chrono::milliseconds timeout = std::chrono::milliseconds{20000})
            {
                std::unique_lock<std::mutex> l(m_mutex);
                if (!m_condVar.wait_for(l, timeout, [&]() { return !m_data.empty(); }))
                    return {};
                auto val = m_data.front();
                m_data.pop_front();
                return val;
            }

            void waitForData(std::chrono::milliseconds timeout = std::chrono::milliseconds{20000})
            {
                std::unique_lock<std::mutex> l(m_mutex);
                m_condVar.wait_for(l, timeout, [&]() { return !m_data.empty(); });
            }

            void pushAll(const std::vector<T>& vec)
            {
                std::lock_guard<std::mutex> l(m_mutex);
                m_data.insert(m_data.end(), vec.begin(), vec.end());
                m_condVar.notify_all();
            }

            std::vector<T> popAll()
            {
                std::unique_lock<std::mutex> l(m_mutex);
                std::vector<T> res(m_data.begin(), m_data.end());
                m_data.clear();
                return res;
            }

        private:
            std::deque<T> m_data;
            std::mutex m_mutex;
            std::condition_variable m_condVar;
        };

        using StackQueue = BlockingQueue<Message::Type>;
        using EventQueue = BlockingQueue<Event::Type>;

        // stack implemetation that puts all messages in a StackQueue
        class QueueForwardingStack : public ISomeIPStackCommon<TestInstanceId>
        {
        public:
            QueueForwardingStack(TestInstanceId iid, StackQueue& outQueue)
                : m_iid(iid)
                , m_outQueue(outQueue)
            {
            }

            virtual bool connect() override
            {
                m_connected = true;
                m_outQueue.push(Message::Available{m_iid});
                return true;
            }

            virtual bool disconnect() override
            {
                m_connected = false;
                m_outQueue.push(Message::Unavailable{m_iid});
                return true;
            }

            virtual void logConnectionState(StringOutputStream& /*sos*/) override {}

            virtual InstanceIdType getServiceInstanceId() const override
            {
                return m_iid;
            }

            virtual bool sendParticipantInfo(InstanceIdType to, const SomeIPMsgHeader& header, uint16_t protocolVersion, uint32_t minorProtocolVersion, InstanceIdType senderInstanceId,
                                             uint64_t expectedReceiverPid, uint8_t clockType, uint64_t timestampNow) override
            {
                m_outQueue.push(Message::ParticipantInfo{to, header, protocolVersion, minorProtocolVersion, senderInstanceId, expectedReceiverPid, clockType, timestampNow});
                return true;
            }

            virtual bool sendKeepAlive(InstanceIdType to, const SomeIPMsgHeader& header, uint64_t timestampNow, bool usingPreviousMessageId) override
            {
                m_outQueue.push(Message::KeepAlive{to, header, timestampNow, usingPreviousMessageId});
                return true;
            }

            bool sendTest(InstanceIdType to, const SomeIPMsgHeader& header, uint32_t arg)
            {
                m_outQueue.push(Message::Test{to, header, arg});
                return true;
            }

        private:
            const TestInstanceId m_iid;
            StackQueue& m_outQueue;
            bool m_connected = false;
            std::thread m_runner;
        };

        // class to execute StackQueue actions in a thread on connectionsystem callbacks
        class QueueCallbackRunner
        {
        public:
            QueueCallbackRunner(TestInstanceId iid, TestCallbacks& callbacks, StackQueue& inQueue, PlatformLock& lock)
                : m_iid(iid)
                , m_callbacks(callbacks)
                , m_inQueue(inQueue)
                , m_lock(lock)
            {
            }

            void startRunner()
            {
                assert(!m_thread.joinable());
                m_stop = false;
                m_thread = std::thread([&]() { run(); });
            }

            void stopRunner()
            {
                assert(m_thread.joinable());
                m_stop = true;
                m_inQueue.push(Message::Type{});
                m_thread.join();
            }

        private:
            void run()
            {
                while (!m_stop)
                {
                    Message::Type msg = m_inQueue.pop();
                    std::unique_lock<std::recursive_mutex> l(m_lock);
                    if (auto avail = absl::get_if<Message::Available>(&msg))
                        m_callbacks.handleServiceAvailable(avail->iid);
                    else if (auto unavail = absl::get_if<Message::Unavailable>(&msg))
                        m_callbacks.handleServiceUnavailable(unavail->iid);
                    else if (auto pinfo = absl::get_if<Message::ParticipantInfo>(&msg))
                    {
                        if (m_iid == pinfo->to)
                            m_callbacks.handleParticipantInfo(pinfo->hdr,
                                                              pinfo->protocolVersion,
                                                              pinfo->minorProtocolVersion,
                                                              pinfo->senderInstanceId,
                                                              pinfo->expectedReceiverPid,
                                                              pinfo->clockType,
                                                              pinfo->timestampNow);
                    }
                    else if (auto alive = absl::get_if<Message::KeepAlive>(&msg))
                    {
                        if (m_iid == alive->to)
                            m_callbacks.handleKeepAlive(alive->hdr, alive->timestampNow, alive->usingPreviousMessageId);
                    }
                    else if (auto test = absl::get_if<Message::Test>(&msg))
                    {
                        if (m_iid == test->to)
                            m_callbacks.handleTestMessage(test->hdr, test->arg);
                    }
                    else if (absl::get_if<absl::monostate>(&msg))
                        ; // used to wake up thread
                    else
                        assert(false);
                }
            }

            const TestInstanceId m_iid;
            TestCallbacks& m_callbacks;
            StackQueue& m_inQueue;
            PlatformLock& m_lock;
            std::thread m_thread;
            std::atomic<bool> m_stop{false};
        };

        // status listener to put all events into EventQueue
        class QueueConnectionStatusListener : public IConnectionStatusListener
        {
        public:
            explicit QueueConnectionStatusListener(EventQueue& queue)
                : m_queue(queue)
            {
            }

        private:
            virtual void newParticipantHasConnected(const Guid& guid) override
            {
                m_queue.push(Event::NewParticipant{guid});
            }

            virtual void participantHasDisconnected(const Guid& guid) override
            {
                m_queue.push(Event::ParticipantGone{guid});
            }

            EventQueue& m_queue;
        };

        // concrete connection system with test message
        class TestConnectionSystem : public ConnectionSystemBase<TestCallbacks>
        {
        public:
            TestConnectionSystem(const std::shared_ptr<QueueForwardingStack>& _stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                                 UInt32 protocolVersion, PlatformLock& lock,
                                 std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                                 EventQueue& eventQueue, bool enableInitiatorResponder)
                : ConnectionSystemBase(_stack, communicationUserID, namedPid, protocolVersion, lock,
                                       keepAliveInterval, keepAliveTimeout,
                                       []() { return std::chrono::steady_clock::now(); },
                                       CONTEXT_COMMUNICATION, "TEST",
                                       enableInitiatorResponder)
                , m_stack(_stack)
                , m_eventQueue(eventQueue)
            {
            }

            bool sendTest(const Guid& to, uint32_t arg)
            {
                return sendUnicast("sendTestMessage", to, [&](TestInstanceId iid, SomeIPMsgHeader hdr) {
                    return m_stack->sendTest(iid, hdr, arg);
                });
            }

            void handleTestMessage(const SomeIPMsgHeader& header, uint32_t arg) override
            {
                if (const Guid* pid = processReceivedMessageHeader(header, "handleTestMessage"))
                    m_eventQueue.push(Event::TestMsg{*pid, arg});
            }

        private:
            std::shared_ptr<QueueForwardingStack> m_stack;
            EventQueue& m_eventQueue;
        };

        // wrapper around connection system, queues and callback runner. this is a fully functioning connection system
        // that acts in incoming messages and reacts with outgoing messages and events.
        class ThreadedQueuedConnectionSystem
        {
        public:
            ThreadedQueuedConnectionSystem(StackQueue& inQueue, StackQueue& outQueue,
                                           uint16_t id, bool enableInitiatorResponder,
                                           std::chrono::milliseconds keepaliveTimeout = std::chrono::milliseconds{2000})
                : m_inQueue(inQueue)
                , m_outQueue(outQueue)
                , selfId(id)
                , m_stack(std::make_shared<QueueForwardingStack>(TestInstanceId(id), m_outQueue))
                , m_connSys(std::make_unique<TestConnectionSystem>(m_stack,
                                                                   id,
                                                                   ParticipantIdentifier(Guid(id), "foo"),
                                                                   199,
                                                                   m_lock,
                                                                   std::chrono::milliseconds{10}, keepaliveTimeout,
                                                                   eventQueue,
                                                                   enableInitiatorResponder))
                , m_callbackRunner(std::make_unique<QueueCallbackRunner>(TestInstanceId(id), *m_connSys, m_inQueue, m_lock))
                , listener(eventQueue)
            {
                m_callbackRunner->startRunner();
                m_connSys->getConnectionStatusUpdateNotifier().registerForConnectionUpdates(&listener);
            }

            ~ThreadedQueuedConnectionSystem()
            {
                disconnect();
                m_connSys->getConnectionStatusUpdateNotifier().unregisterForConnectionUpdates(&listener);
                m_callbackRunner->stopRunner();
            }

            bool connect()
            {
                std::unique_lock<std::recursive_mutex> l(m_lock);
                return m_connSys->connect();
            }

            bool disconnect()
            {
                std::unique_lock<std::recursive_mutex> l(m_lock);
                return m_connSys->disconnect();
            }

            template <typename T>::testing::AssertionResult nextEvent(T e)
            {
                const auto var = eventQueue.pop();
                if (absl::holds_alternative<absl::monostate>(var))
                    return ::testing::AssertionFailure() << "timeout";
                else if (!absl::holds_alternative<T>(var))
                    return ::testing::AssertionFailure() << "wrong type index " << var.index();
                if (absl::get<T>(var) == e)
                    return ::testing::AssertionSuccess();
                return ::testing::AssertionFailure() << "unequal";
            }

            StackQueue& m_inQueue;
            StackQueue& m_outQueue;
            uint16_t selfId;
            PlatformLock m_lock;
            std::shared_ptr<QueueForwardingStack> m_stack;
            std::unique_ptr<TestConnectionSystem> m_connSys;
            std::unique_ptr<QueueCallbackRunner> m_callbackRunner;
            EventQueue eventQueue;
            QueueConnectionStatusListener listener;
        };

        bool LastEventIsConnectedToOther(const std::vector<Event::Type>& vec, uint16_t otherId)
        {
            for (auto it = std::make_reverse_iterator(vec.end()); it != std::make_reverse_iterator(vec.begin()); ++it)
            {
                // first thing from back must be NewParticipant
                if (auto ev = absl::get_if<Event::NewParticipant>(&*it))
                    if (ev->pid == Guid(otherId))
                        return true;

                // fails if it is ParticipantGone
                if (auto ev = absl::get_if<Event::ParticipantGone>(&*it))
                    if (ev->pid == Guid(otherId))
                        break;
            }
            return false;
        }

        struct TestConfig {
            bool enableInitiatorResponder_a;
            uint16_t id_a;

            bool enableInitiatorResponder_b;
            uint16_t id_b;

            uint64_t keepaliveTimeoutMs;

            const char* desc;

            friend std::ostream& operator<<(std::ostream& os, const TestConfig& tc) { return os << tc.desc; } // avoid valgrind googletest byte print issue due to padding
        };
    }

    using namespace ThreadedStressTestInternal;

    class AConnectionSystemGenericThreadedStressTest : public ::testing::TestWithParam<TestConfig>
    {
        ScopedConsoleLogDisable consoleDisabler;
    };

    INSTANTIATE_TEST_SUITE_P(AConnectionSystemGenericThreadedStressTestP,
                             AConnectionSystemGenericThreadedStressTest,
                             ::testing::ValuesIn(std::vector<TestConfig>{
                                 {true, 1, false, 2, 2000, "a_old"},
                                 {false, 1, true, 2, 2000, "b_old"},
                                 {true, 1, true, 2, 600, "both_new_a_client"},
                                 {true, 2, true, 1, 600, "both_new_b_client"},
                             }),
                             [](const auto& arg) { return arg.param.desc; });

    TEST_P(AConnectionSystemGenericThreadedStressTest, canCreateWithConnectedQueues)
    {
        StackQueue b2a;
        StackQueue a2b;
        ThreadedQueuedConnectionSystem a(b2a, a2b, GetParam().id_a, GetParam().enableInitiatorResponder_a);
        ThreadedQueuedConnectionSystem b(a2b, b2a, GetParam().id_b, GetParam().enableInitiatorResponder_b);
    }

    TEST_P(AConnectionSystemGenericThreadedStressTest, canConnectWithConnectedQueues)
    {
        StackQueue b2a;
        StackQueue a2b;
        ThreadedQueuedConnectionSystem a(b2a, a2b, GetParam().id_a, GetParam().enableInitiatorResponder_a);
        ThreadedQueuedConnectionSystem b(a2b, b2a, GetParam().id_b, GetParam().enableInitiatorResponder_b);

        EXPECT_TRUE(a.connect());
        EXPECT_TRUE(b.connect());

        EXPECT_TRUE(a.nextEvent(Event::NewParticipant{Guid(b.selfId)}));
        EXPECT_TRUE(b.nextEvent(Event::NewParticipant{Guid(a.selfId)}));

        EXPECT_TRUE(a.disconnect());

        EXPECT_TRUE(a.nextEvent(Event::ParticipantGone{Guid(b.selfId)}));
        EXPECT_TRUE(b.nextEvent(Event::ParticipantGone{Guid(a.selfId)}));
    }

    TEST_P(AConnectionSystemGenericThreadedStressTest, multiConnectionSystemStressTest)
    {
        StackQueue aIn;
        StackQueue aOut;
        StackQueue bIn;
        StackQueue bOut;
        ThreadedQueuedConnectionSystem a(aIn, aOut, GetParam().id_a, GetParam().enableInitiatorResponder_a, std::chrono::milliseconds{GetParam().keepaliveTimeoutMs});
        ThreadedQueuedConnectionSystem b(bIn, bOut, GetParam().id_b, GetParam().enableInitiatorResponder_b, std::chrono::milliseconds{GetParam().keepaliveTimeoutMs});

        // set up randomness
        std::random_device randomSource;
        unsigned int seed = randomSource();
        SCOPED_TRACE(seed);
        std::mt19937 gen(seed);

        auto rnd = [&]() {
            std::uniform_int_distribution<uint32_t> dis(0, 100);
            return dis(gen);
        };
        auto num = [&]() {
            std::uniform_int_distribution<uint16_t> dis(0, 6);
            return dis(gen);
        };
        const uint16_t proto_a = GetParam().enableInitiatorResponder_a ? 1 : 0;
        const uint16_t proto_b = GetParam().enableInitiatorResponder_b ? 1 : 0;

        EXPECT_TRUE(a.connect());
        EXPECT_TRUE(b.connect());

        // randomly inject or drop messages
        for (int i = 0; i < 4000; ++i)
        {
            const bool direction = rnd() < 50;
            StackQueue& dst = direction ? bIn  : aIn;
            const uint16_t srcId = direction ? a.selfId : b.selfId;
            const uint16_t dstId = direction ? b.selfId : a.selfId;
            const uint16_t proto = direction ? proto_a : proto_b;

            // inject messages
            if (rnd() < 20)
                dst.push(Message::Available{TestInstanceId(num())});
            if (rnd() < 20)
                dst.push(Message::Available{TestInstanceId(srcId)});
            if (rnd() < 2)
                dst.push(Message::Available{TestInstanceId()});

            if (rnd() < 10)
                dst.push(Message::Unavailable{TestInstanceId(num())});
            if (rnd() < 10)
                dst.push(Message::Unavailable{TestInstanceId(srcId)});
            if (rnd() < 2)
                dst.push(Message::Unavailable{TestInstanceId()});

            if (rnd() < 5)
                dst.push(Message::KeepAlive{TestInstanceId(dstId), SomeIPMsgHeader{srcId, num(), num()}, 0, false});
            if (rnd() < 5)
                dst.push(Message::KeepAlive{TestInstanceId(dstId), SomeIPMsgHeader{num(), num(), num()}, 0, false});

            if (rnd() < 10)
                dst.push(Message::Test{TestInstanceId(dstId), SomeIPMsgHeader{srcId, num(), num()}, num()});
            if (rnd() < 5)
                dst.push(Message::Test{TestInstanceId(dstId), SomeIPMsgHeader{num(), num(), num()}, num()});

            if (rnd() < 10)
                dst.push(Message::ParticipantInfo{TestInstanceId(dstId), SomeIPMsgHeader{srcId, num(), 1},
                                                  199, proto, TestInstanceId(srcId), dstId, 0, 0});
            if (rnd() < 5)
            {
                // pid and senderInstanceId must be same, diff not supported yet
                const auto senderId = num();
                dst.push(Message::ParticipantInfo{TestInstanceId(dstId), SomeIPMsgHeader{senderId, num(), num()},
                                                  199, proto, TestInstanceId(senderId), num(), 0, 0});
            }
            if (rnd() < 2)
            {
                // pid and senderInstanceId must be same, diff not supported yet
                const auto senderId = num();
                dst.push(Message::ParticipantInfo{TestInstanceId(dstId), SomeIPMsgHeader{senderId, num(), num()},
                                                  num(), num(), TestInstanceId(senderId), num(), 0, 0});
            }

            // let connection systems catch up from time to time
            if (i % 50 == 0 && i != 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

            // forward messages
            auto randomizedForward = [&](auto& dstQueue, auto& srcQueue) {
                auto v = srcQueue.popAll();
                // drop random messages
                if (rnd() < 20)
                    v.erase(std::remove_if(v.begin(), v.end(), [&](auto) { return rnd() < 10; }), v.end());
                // reorder
                else if (rnd() < 20)
                    std::shuffle(v.begin(), v.end(), gen);
                // else: just forward

                dstQueue.pushAll(v);
            };
            randomizedForward(aIn, bOut);
            randomizedForward(bIn, aOut);
        }

        // ensure available
        aIn.push(Message::Available{TestInstanceId(b.selfId)});
        bIn.push(Message::Available{TestInstanceId(a.selfId)});

        // process messages for limited time
        // either limited by time (20s) or when only keepalive messages exchanged for "nothingNewLimit" cycles
        // (system reached a stable state)
        const uint64_t nothingNewLimit = (GetParam().enableInitiatorResponder_a && GetParam().enableInitiatorResponder_b) ?
            20 : 100;           // when a and b new it converges reliably a lot faster than before
        const auto startT = std::chrono::steady_clock::now();
        uint64_t nothingNewCounter = 0;
        while (startT + std::chrono::seconds(20) > std::chrono::steady_clock::now() &&
               nothingNewCounter < nothingNewLimit)
        {
            // forward all
            aOut.waitForData(std::chrono::milliseconds(10));
            const auto toB = aOut.popAll();
            const auto toA = bOut.popAll();
            bIn.pushAll(toB);
            aIn.pushAll(toA);

            // check if still real communication happending (non-keepalive), early out if not
            auto isKeepAlive = [](const auto msg) {
                return absl::holds_alternative<Message::KeepAlive>(msg);
            };
            if (std::all_of(toB.begin(), toB.end(), isKeepAlive) &&
                std::all_of(toA.begin(), toA.end(), isKeepAlive))
                ++nothingNewCounter;
            else
                nothingNewCounter = 0;
        }

        // ensure gets to stable connected state: last event for otherId must be NewParticipant
        EXPECT_TRUE(LastEventIsConnectedToOther(b.eventQueue.popAll(), a.selfId));
        EXPECT_TRUE(LastEventIsConnectedToOther(a.eventQueue.popAll(), b.selfId));

        // send and receive test message
        {
            std::lock_guard<std::recursive_mutex> la(a.m_lock);
            a.m_connSys->sendTest(Guid(b.selfId), 456);
        }
        {
            std::lock_guard<std::recursive_mutex> lb(b.m_lock);
            b.m_connSys->sendTest(Guid(a.selfId), 765);
        }

        bIn.pushAll(aOut.popAll());
        aIn.pushAll(bOut.popAll());

        EXPECT_TRUE(a.nextEvent(Event::TestMsg{Guid(b.selfId), 765}));
        EXPECT_TRUE(b.nextEvent(Event::TestMsg{Guid(a.selfId), 456}));
    }


    // ConnectionSystemBase specific test
    class AConnectionSystemBaseThreadedStressTest : public ::testing::Test
    {
        ScopedConsoleLogDisable consoleDisabler;
    };

    // TODO: try to adapt to connsysIR
    TEST_F(AConnectionSystemBaseThreadedStressTest, disconnectsWithoutKeepalive)
    {
        StackQueue to;
        StackQueue from;
        ThreadedQueuedConnectionSystem a(to, from, 1, false, std::chrono::milliseconds{100});

        EXPECT_TRUE(a.connect());

        to.push(Message::Available{TestInstanceId(2)});
        to.push(Message::ParticipantInfo{TestInstanceId(1), SomeIPMsgHeader{2, 123, 1}, 199, SomeIPConstants::FallbackMinorProtocolVersion, TestInstanceId(2), 1, 0, 0});

        EXPECT_TRUE(a.nextEvent(Event::NewParticipant{Guid(2)}));
        EXPECT_TRUE(a.nextEvent(Event::ParticipantGone{Guid(2)}));
    }

    TEST_F(AConnectionSystemBaseThreadedStressTest, simultaneousNewSessionDoesNotLeadToReconnectLoop)
    {
        StackQueue b2a;
        StackQueue a2b;
        ThreadedQueuedConnectionSystem a(b2a, a2b, 1, false);
        ThreadedQueuedConnectionSystem b(a2b, b2a, 2, false);

        EXPECT_TRUE(a.connect());
        EXPECT_TRUE(b.connect());

        EXPECT_TRUE(a.nextEvent(Event::NewParticipant{Guid(2)}));
        EXPECT_TRUE(b.nextEvent(Event::NewParticipant{Guid(1)}));

        {
            // inject new session on both sides simultaneously (guaranteed by locks)
            std::lock_guard<std::recursive_mutex> l1(a.m_lock);
            std::lock_guard<std::recursive_mutex> l2(b.m_lock);

            b2a.push(Message::ParticipantInfo{TestInstanceId(1), SomeIPMsgHeader{2, 123, 1}, 199, SomeIPConstants::FallbackMinorProtocolVersion, TestInstanceId(2), 1, 0, 0});
            a2b.push(Message::ParticipantInfo{TestInstanceId(2), SomeIPMsgHeader{1, 234, 1}, 199, SomeIPConstants::FallbackMinorProtocolVersion, TestInstanceId(1), 2, 0, 0});
        }

        // wait until no new events appear for some time.
        // limit to 20s to prevent test hang in case of failure.
        const auto startT = std::chrono::steady_clock::now();
        uint64_t nothingNewCounter = 0;
        std::vector<Event::Type> fullEventQueueA;
        std::vector<Event::Type> fullEventQueueB;
        while (startT + std::chrono::seconds(20) > std::chrono::steady_clock::now() &&
               nothingNewCounter < 10)
        {
            a.eventQueue.waitForData(std::chrono::milliseconds(10));
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
            const auto eventsA = a.eventQueue.popAll();
            const auto eventsB = b.eventQueue.popAll();
            if (eventsA.empty() && eventsB.empty())
                ++nothingNewCounter;
            else
                nothingNewCounter = 0;
            fullEventQueueA.insert(fullEventQueueA.end(), eventsA.begin(), eventsA.end());
            fullEventQueueB.insert(fullEventQueueB.end(), eventsB.begin(), eventsB.end());
        }

        EXPECT_TRUE(LastEventIsConnectedToOther(fullEventQueueB, a.selfId));
        EXPECT_TRUE(LastEventIsConnectedToOther(fullEventQueueA, b.selfId));

        EXPECT_TRUE(a.disconnect());

        EXPECT_TRUE(a.nextEvent(Event::ParticipantGone{Guid(2)}));
        EXPECT_TRUE(b.nextEvent(Event::ParticipantGone{Guid(1)}));

        // queue must be empty
        EXPECT_EQ(0u, a.eventQueue.popAll().size());
        EXPECT_EQ(0u, b.eventQueue.popAll().size());
    }
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::ThreadedStressTestInternal::TestInstanceId)
