//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "DltLogAppender/DltAdapter.h"
#include "ramses-capu/os/Memory.h"
#include "Utils/LogContext.h"
#include "Utils/LogMessage.h"
#include "Collections/String.h"

using namespace ramses_internal;

class DltAdapterWithRegisteredApplication : public ::testing::Test
{
public:
    DltAdapterWithRegisteredApplication()
    {
        m_dltAdapter = DltAdapter::getDltAdapter();
        EXPECT_TRUE(m_dltAdapter->registerApplication(String("APP0"), String("appname")));
    };
    virtual ~DltAdapterWithRegisteredApplication()
    {
        m_dltAdapter->unregisterApplication();
    };
protected:
    DltAdapter* m_dltAdapter;
};

TEST(DltAdapter, RegisterAndUnregisterApplication)
{
    DltAdapter* m_dltAdapter = DltAdapter::getDltAdapter();
    m_dltAdapter->unregisterApplication();

    EXPECT_TRUE(m_dltAdapter->registerApplication("TEST", "Test application"));
    bool dltInitialized = m_dltAdapter->isDltInitialized();
    EXPECT_TRUE(dltInitialized);

    EXPECT_STREQ(m_dltAdapter->getApplicationName().c_str(), "TEST");
    EXPECT_STREQ(m_dltAdapter->getApplicationDescription().c_str(), "Test application");
    EXPECT_EQ(m_dltAdapter->getDltStatus(), EDltError_NO_ERROR);

    m_dltAdapter->unregisterApplication();

    EXPECT_EQ(m_dltAdapter->getDltStatus(), EDltError_NO_ERROR);
    EXPECT_STREQ(m_dltAdapter->getApplicationName().c_str(), "");
    EXPECT_STREQ(m_dltAdapter->getApplicationDescription().c_str(), "");
}

TEST(DltAdapter, RegisterApplicationWithAppNameTooLarge)
{
    DltAdapter* m_dltAdapter = DltAdapter::getDltAdapter();
    EXPECT_FALSE(m_dltAdapter->registerApplication("TESTABC", "Test application"));
}

TEST(DltAdapter, SingletonNotNull)
{
    EXPECT_TRUE(DltAdapter::getDltAdapter() != nullptr);
}

TEST_F(DltAdapterWithRegisteredApplication, DltInitialized)
{
    bool dltInitialized = m_dltAdapter->isDltInitialized();
    EXPECT_TRUE(dltInitialized);
}

TEST_F(DltAdapterWithRegisteredApplication, RegisterandUnregisterContext)
{
    LogContext con("Test context", "TCTX");

    m_dltAdapter->registerContext(&con, false, ELogLevel::Off);
    EXPECT_EQ(m_dltAdapter->getDltStatus(),EDltError_NO_ERROR);

    m_dltAdapter->unregisterApplication();

    EXPECT_EQ(m_dltAdapter->getDltStatus(),EDltError_NO_ERROR);
}

TEST_F(DltAdapterWithRegisteredApplication, LogMessageInDltAdapter)
{
    DltAdapter* dltAdapter = DltAdapter::getDltAdapter();

    bool dltInitialized = dltAdapter->isDltInitialized();
    EXPECT_TRUE(dltInitialized);

    dltAdapter->registerApplication("TEST","Test application");

    LogContext con("Test context", "TCTX");

    dltAdapter->registerContext(&con, false, ELogLevel::Off);

    StringOutputStream stream;
    stream << "sample message";
    LogMessage msg(con, ELogLevel::Info, stream);

    dltAdapter->logMessage(msg);
    EXPECT_EQ(dltAdapter->getDltStatus(),EDltError_NO_ERROR);

    dltAdapter->unregisterApplication();
    EXPECT_EQ(dltAdapter->getDltStatus(),EDltError_NO_ERROR);
}
