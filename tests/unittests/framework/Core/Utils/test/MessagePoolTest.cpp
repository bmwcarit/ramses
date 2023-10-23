//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/Core/Utils/MessagePool.h"

using namespace testing;

namespace ramses::internal
{
    class MessagePoolTest : public testing::Test
    {
    protected:
        MessagePool<32u, 0u> m_messagePool;
    };

    TEST_F(MessagePoolTest, getSuccessMsg)
    {
        EXPECT_STREQ(m_messagePool.getSuccessText(), m_messagePool.getMessage(m_messagePool.SuccessMessageID));
    }

    TEST_F(MessagePoolTest, getInvalidMsg)
    {
        EXPECT_STREQ(m_messagePool.getUnknownText(), m_messagePool.getMessage(m_messagePool.SuccessMessageID + 1u));
    }

    TEST_F(MessagePoolTest, addMessage)
    {
        const uint32_t id = m_messagePool.addMessage("msg 1");
        EXPECT_STREQ("msg 1", m_messagePool.getMessage(id));
    }

    TEST_F(MessagePoolTest, fillCache)
    {
        const std::string msgBase("msg ");
        for (uint32_t i = 0u; i < m_messagePool.MaxMessageEntries; ++i)
        {
            std::string msg = msgBase;
            msg += ('0' + static_cast<char>(i));
            m_messagePool.addMessage(msg.c_str());
        }

        EXPECT_STREQ(m_messagePool.getSuccessText(), m_messagePool.getMessage(m_messagePool.SuccessMessageID));

        for (uint32_t i = 0u; i < m_messagePool.MaxMessageEntries; ++i)
        {
            std::string msg = msgBase;
            msg += ('0' + static_cast<char>(i));
            EXPECT_STREQ(msg.c_str(), m_messagePool.getMessage(m_messagePool.SuccessMessageID + 1u + i));
        }
    }

    TEST_F(MessagePoolTest, floodCache)
    {
        const uint32_t maxMessages = m_messagePool.MaxMessageEntries;
        for (uint32_t i = 0u; i < maxMessages * 10; ++i)
        {
            m_messagePool.addMessage("message");
        }

        EXPECT_STREQ(m_messagePool.getSuccessText(), m_messagePool.getMessage(m_messagePool.SuccessMessageID));

        const uint32_t id = m_messagePool.addMessage("msg X");
        EXPECT_STREQ("msg X", m_messagePool.getMessage(id));
    }

    TEST_F(MessagePoolTest, tooOldEntryGivesUnknownMessage)
    {
        const uint32_t maxMessages = m_messagePool.MaxMessageEntries;
        for (uint32_t i = 0u; i < maxMessages * 3; ++i)
        {
            m_messagePool.addMessage("message");
        }

        EXPECT_STREQ(m_messagePool.getSuccessText(), m_messagePool.getMessage(m_messagePool.SuccessMessageID));
        EXPECT_STREQ(m_messagePool.getUnknownText(), m_messagePool.getMessage(maxMessages));
    }

    TEST_F(MessagePoolTest, willNotReturnSuccessMessageIdIfFilled)
    {
        const std::string msg("msg");
        for (uint32_t i = 0u; i < 2 * m_messagePool.MaxMessageEntries; ++i)
        {
            const auto id = m_messagePool.addMessage(msg.c_str());
            EXPECT_NE(uint32_t(m_messagePool.SuccessMessageID), id);
            EXPECT_STREQ(msg.c_str(), m_messagePool.getMessage(id));
        }
    }
}
