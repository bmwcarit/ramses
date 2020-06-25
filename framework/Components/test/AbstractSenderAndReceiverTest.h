//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ABSTRACTSENDERANDRECEIVERTEST_H
#define RAMSES_ABSTRACTSENDERANDRECEIVERTEST_H

#include <gmock/gmock.h>
#include "framework_common_gmock_header.h"
#include "CommunicationSystemTestWrapper.h"
#include "Common/TypedMemoryHandle.h"
#include "CommunicationSystemTestFactory.h"

namespace ramses_internal
{
    using namespace testing;

    class AbstractSenderAndReceiverTest : public ::testing::TestWithParam<ECommunicationSystemType>
    {
    public:
        explicit AbstractSenderAndReceiverTest(EServiceType serviceType)
            : m_state(CommunicationSystemTestFactory::ConstructTestState(GetParam(), serviceType))
            , m_daemon(CommunicationSystemTestFactory::ConstructDiscoveryDaemonTestWrapper(*m_state))
            , m_senderTestWrapper(CommunicationSystemTestFactory::ConstructTestWrapper(*m_state, "sender"))
            , m_receiverTestWrapper(CommunicationSystemTestFactory::ConstructTestWrapper(*m_state, "receiver"))
            , sender(*m_senderTestWrapper->commSystem)
            , receiver(*m_receiverTestWrapper->commSystem)
            , senderId(m_senderTestWrapper->id)
            , receiverId(m_receiverTestWrapper->id)
            , receiverExpectCallLock(m_receiverTestWrapper->frameworkLock)
        {
        }

        virtual ~AbstractSenderAndReceiverTest()
        {
        }

        ::testing::AssertionResult waitForEvent(int numberEvents = 1, uint32_t waitTimeMsOverride = 0)
        {
            return m_state->event.waitForEvents(numberEvents, waitTimeMsOverride);
        }

        void sendEvent()
        {
            m_state->event.signal();
        }

        void skipStatisticsTest()
        {
            statisticsTestEnabled = false;
        }

    private:
        void SetUp() override
        {
            ASSERT_TRUE(m_daemon->start());
            m_state->connectAll();
            ASSERT_TRUE(m_state->blockOnAllConnected());

            // make sure receiver can get broadcasts
            m_receiverTestWrapper->registerAsEventReceiver();

            // record message stats
            numberMessagesSentBefore = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
            numberMessagesReceivedBefore = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();
        }

        void TearDown() override
        {
            if (statisticsTestEnabled)
            {
                // check at least one message sent and received
                EXPECT_LE(numberMessagesSentBefore + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
                EXPECT_LE(numberMessagesReceivedBefore + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
            }

            m_state->disconnectAll();
            EXPECT_TRUE(m_daemon->stop());
        }

        std::unique_ptr<CommunicationSystemTestState> m_state;
        std::unique_ptr<CommunicationSystemDiscoveryDaemonTestWrapper> m_daemon;

    protected:
        std::unique_ptr<CommunicationSystemTestWrapper> m_senderTestWrapper;
        std::unique_ptr<CommunicationSystemTestWrapper> m_receiverTestWrapper;

    public:
        ICommunicationSystem& sender;
        ICommunicationSystem& receiver;
        const Guid& senderId;
        const Guid& receiverId;
        PlatformLock& receiverExpectCallLock;
        uint32_t numberMessagesSentBefore = 0;
        uint32_t numberMessagesReceivedBefore = 0;
        bool statisticsTestEnabled = true;
    };
}

#endif
