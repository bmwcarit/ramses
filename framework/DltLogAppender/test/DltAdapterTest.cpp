//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "DltLogAppender/DltAdapter.h"
#include "Utils/LogContext.h"
#include "Utils/LogMessage.h"
#include "Collections/String.h"

using namespace ramses_internal;

class ADltAdapter : public ::testing::Test
{
public:
    ADltAdapter()
        : adapter(DltAdapter::getDltAdapter())
        , context(new LogContext("RXXX", "Test context"))
        , logLevelChangeCallback([](const String&, int){})
    {
    }

    void SetUp() override
    {
        if (DltAdapter::IsDummyAdapter())
            GTEST_SKIP();

        if (adapter && adapter->isInitialized())
            adapter->uninitialize();
    }

    ~ADltAdapter()
    {
        if (adapter && adapter->isInitialized())
            adapter->uninitialize();
    }

    DltAdapter* adapter;
    std::unique_ptr<LogContext> context;
    std::function<void(const String&, int)> logLevelChangeCallback;
};

// run always
TEST(DltAdapter, singletonNotNull)
{
    EXPECT_TRUE(DltAdapter::getDltAdapter() != nullptr);
}

TEST_F(ADltAdapter, initiallyUninitialized)
{
    EXPECT_FALSE(adapter->isInitialized());
}

TEST_F(ADltAdapter, initializeAndUninitialize)
{
    EXPECT_TRUE(adapter->initialize("TEST", "Test application", true, logLevelChangeCallback, {context.get()}, true));
    EXPECT_TRUE(adapter->isInitialized());

    adapter->uninitialize();
    EXPECT_FALSE(adapter->isInitialized());
}

TEST_F(ADltAdapter, doubleInitFails)
{
    EXPECT_TRUE(adapter->initialize("TEST", "Test application", true, logLevelChangeCallback, {context.get()}, true));
    EXPECT_FALSE(adapter->initialize("TEST", "Test application", true, logLevelChangeCallback, {context.get()}, true));
}

TEST_F(ADltAdapter, appIdMustBeValid)
{
    EXPECT_FALSE(adapter->initialize("", "Test application", true, logLevelChangeCallback, {context.get()}, true));
    EXPECT_FALSE(adapter->initialize("TEST123", "Test application", true, logLevelChangeCallback, {context.get()}, true));
}

TEST_F(ADltAdapter, contextListMayNotBeEmpty)
{
    EXPECT_FALSE(adapter->initialize("TEST", "Test application", true, logLevelChangeCallback, {}, true));
}

TEST_F(ADltAdapter, canLogOnContext)
{
    EXPECT_TRUE(adapter->initialize("TEST", "Test application", true, logLevelChangeCallback, {context.get()}, true));
    StringOutputStream sos(std::string("foo"));
    LogMessage msg(*context, ELogLevel::Info, sos);
    EXPECT_TRUE(adapter->logMessage(msg));
}

TEST_F(ADltAdapter, logFailsWithoutInit)
{
    StringOutputStream sos(std::string("foo"));
    LogMessage msg(*context, ELogLevel::Info, sos);
    EXPECT_FALSE(adapter->logMessage(msg));
}

TEST_F(ADltAdapter, canLogWithLogLevelOff)
{
    EXPECT_TRUE(adapter->initialize("TEST", "Test application", true, logLevelChangeCallback, {context.get()}, true));
    StringOutputStream sos(std::string("foo"));
    LogMessage msg(*context, ELogLevel::Off, sos);
    EXPECT_TRUE(adapter->logMessage(msg));
}

TEST_F(ADltAdapter, logFailsWithNonDltContext)
{
    EXPECT_TRUE(adapter->initialize("TEST", "Test application", true, logLevelChangeCallback, {context.get()}, true));

    std::unique_ptr<LogContext> otherContext(new LogContext("RYYY", "Other test context"));
    StringOutputStream sos(std::string("foo"));
    LogMessage msg(*otherContext, ELogLevel::Info, sos);
    EXPECT_FALSE(adapter->logMessage(msg));
}

#if defined(DLT_ENABLED)

WARNINGS_PUSH
WARNING_DISABLE_LINUX(-Wold-style-cast)
#include <dlt_common_api.h>
WARNINGS_POP

TEST_F(ADltAdapter, canUseAlreadyRegisteredApp)
{
    DLT_REGISTER_APP("RAPP", "Ramses test app");
    EXPECT_TRUE(adapter->initialize("TEST", "Test application", false, logLevelChangeCallback, {context.get()}, true));
    adapter->uninitialize();
    DLT_UNREGISTER_APP();
}
#endif
