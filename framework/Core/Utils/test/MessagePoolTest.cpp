//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Utils/MessagePool.h"
#include "Collections/String.h"

using namespace testing;

namespace ramses_internal
{
    class MessagePoolTest : public testing::Test
    {
    protected:
        MessagePoolTest()
        {
        }

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
        const UInt32 id = m_messagePool.addMessage("msg 1");
        EXPECT_STREQ("msg 1", m_messagePool.getMessage(id));
    }

    TEST_F(MessagePoolTest, fillCache)
    {
        const ramses_internal::String msgBase("msg ");
        for (UInt32 i = 0u; i < m_messagePool.MaxMessageEntries; ++i)
        {
            ramses_internal::String msg = msgBase;
            msg += ('0' + static_cast<Char>(i));
            m_messagePool.addMessage(msg.c_str());
        }

        EXPECT_STREQ(m_messagePool.getSuccessText(), m_messagePool.getMessage(m_messagePool.SuccessMessageID));

        for (UInt32 i = 0u; i < m_messagePool.MaxMessageEntries; ++i)
        {
            ramses_internal::String msg = msgBase;
            msg += ('0' + static_cast<Char>(i));
            EXPECT_STREQ(msg.c_str(), m_messagePool.getMessage(m_messagePool.SuccessMessageID + 1u + i));
        }
    }

    TEST_F(MessagePoolTest, floodCache)
    {
        const UInt32 maxMessages = m_messagePool.MaxMessageEntries;
        for (UInt32 i = 0u; i < maxMessages * 10; ++i)
        {
            m_messagePool.addMessage("message");
        }

        EXPECT_STREQ(m_messagePool.getSuccessText(), m_messagePool.getMessage(m_messagePool.SuccessMessageID));

        const UInt32 id = m_messagePool.addMessage("msg X");
        EXPECT_STREQ("msg X", m_messagePool.getMessage(id));
    }

    TEST_F(MessagePoolTest, tooOldEntryGivesUnknownMessage)
    {
        const UInt32 maxMessages = m_messagePool.MaxMessageEntries;
        for (UInt32 i = 0u; i < maxMessages * 3; ++i)
        {
            m_messagePool.addMessage("message");
        }

        EXPECT_STREQ(m_messagePool.getSuccessText(), m_messagePool.getMessage(m_messagePool.SuccessMessageID));
        EXPECT_STREQ(m_messagePool.getUnknownText(), m_messagePool.getMessage(maxMessages));
    }
}
