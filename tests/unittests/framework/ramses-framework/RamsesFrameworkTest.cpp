//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <utility>

#include "gmock/gmock.h"
#include "ramses/framework/RamsesFrameworkConfig.h"
#include "ramses/framework/RamsesFramework.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "impl/RamsesFrameworkImpl.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/ThreadBarrier.h"
#include "ApiRamshCommandMock.h"
#include <thread>

using namespace testing;

namespace ramses::internal
{
TEST(ARamsesFramework, canDefaultConstruct)
{
    RamsesFrameworkConfig config{EFeatureLevel_Latest};
    RamsesFramework fw{config};
    EXPECT_GT(fw.impl().getParticipantAddress().getParticipantId().get(), 0xFF);
}

TEST(ARamsesFramework, canConstructFromConfig)
{
    RamsesFrameworkConfig config{EFeatureLevel_Latest};
    config.setParticipantGuid(0x123);
    RamsesFramework fw{config};
    EXPECT_EQ(fw.impl().getParticipantAddress().getParticipantId().get(), 0x123);
}

TEST(ARamsesFramework, isNotConnectedInitially)
{

    RamsesFrameworkConfig config{EFeatureLevel_Latest};
    RamsesFramework fw{config};
    EXPECT_FALSE(fw.isConnected());
}

TEST(ARamsesFramework, connectLifeCycleOK)
{
    RamsesFrameworkConfig config{EFeatureLevel_Latest};
    RamsesFramework fw{config};
    EXPECT_TRUE(fw.connect());
    EXPECT_TRUE(fw.isConnected());
    EXPECT_TRUE(fw.disconnect());
}

TEST(ARamsesFramework, reportsErrorWhenConnectingSecondTime)
{
    RamsesFrameworkConfig config{EFeatureLevel_Latest};
    RamsesFramework fw{config};
    fw.connect();
    EXPECT_FALSE(fw.connect());
    EXPECT_TRUE(fw.isConnected());
}

TEST(ARamsesFramework, reportsErrorWhenDisconnectingSecondTime)
{
    RamsesFrameworkConfig config{EFeatureLevel_Latest};
    RamsesFramework fw{config};
    fw.connect();
    EXPECT_TRUE(fw.disconnect());
    EXPECT_FALSE(fw.disconnect());
}

    class PartialApiRamshCommandMock : public ApiRamshCommandMock
    {
    public:
        explicit PartialApiRamshCommandMock(std::string  kw_)
            : kw(std::move(kw_))
        {}
        [[nodiscard]] const std::string& keyword() const override { return kw; }
        [[nodiscard]] const std::string& help() const override { return helpText; }
        std::string kw;
        std::string helpText{"text"};
    };


TEST(ARamsesFramework, canAddAndTriggerRamshCommands)
{
    auto cmd_a = std::make_shared<testing::StrictMock<PartialApiRamshCommandMock>>("foo");
    auto cmd_b = std::make_shared<testing::StrictMock<PartialApiRamshCommandMock>>("bar");

    RamsesFrameworkConfig config{EFeatureLevel_Latest};
    RamsesFramework fw{config};
    EXPECT_TRUE(fw.addRamshCommand(cmd_a));
    EXPECT_TRUE(fw.addRamshCommand(cmd_b));

    std::string inp_a = "foo";
    EXPECT_CALL(*cmd_a, execute(std::vector<std::string>{"foo"})).WillOnce(Return(true));
    EXPECT_TRUE(fw.executeRamshCommand(inp_a));

    std::string inp_b = "bar baz";
    EXPECT_CALL(*cmd_b, execute(std::vector<std::string>{"bar", "baz"})).WillOnce(Return(true));
    EXPECT_TRUE(fw.executeRamshCommand(inp_b));

    cmd_a.reset();
    EXPECT_FALSE(fw.executeRamshCommand(inp_a));
}

TEST(ARamsesFramwork, canExecuteExistingAndFailingInvalidRamshCommands)
{
    RamsesFrameworkConfig config{EFeatureLevel_Latest};
    RamsesFramework fw{config};
    EXPECT_TRUE(fw.executeRamshCommand("help"));
    EXPECT_TRUE(fw.executeRamshCommand("setLogLevelConsole trace"));
    EXPECT_FALSE(fw.executeRamshCommand("invalid ramsh command"));
    EXPECT_FALSE(fw.executeRamshCommand(""));
}

TEST(ARamsesFramework, failsToAddInvalidRamshCommands)
{
    RamsesFrameworkConfig config{EFeatureLevel_Latest};
    RamsesFramework fw{config};
    EXPECT_FALSE(fw.addRamshCommand({}));
    EXPECT_FALSE(fw.addRamshCommand(std::make_shared<testing::StrictMock<PartialApiRamshCommandMock>>("")));
    EXPECT_FALSE(fw.addRamshCommand(std::make_shared<testing::StrictMock<PartialApiRamshCommandMock>>("a b")));
    EXPECT_FALSE(fw.addRamshCommand(std::make_shared<testing::StrictMock<PartialApiRamshCommandMock>>("a\"b")));
    EXPECT_FALSE(fw.addRamshCommand(std::make_shared<testing::StrictMock<PartialApiRamshCommandMock>>("help")));
}

TEST(ARamsesFramework, SetLogHandler)
{
    bool loggerCalled {false};

    RamsesFramework::SetLogHandler([&loggerCalled](auto level, auto context, auto message)
        {
            if (level == ELogLevel::Warn && context == "RFRA" && message == "R.main: SetLogHandlerTest")
            {
                loggerCalled = true;
            }
        }
    );

    LOG_WARN(CONTEXT_FRAMEWORK, "SetLogHandlerTest");
    EXPECT_TRUE(loggerCalled);
    RamsesFramework::SetLogHandler(nullptr);

    loggerCalled = false;
    LOG_WARN(CONTEXT_FRAMEWORK, "SetLogHandlerTest");
    EXPECT_FALSE(loggerCalled);
}

TEST(ARamsesFramework, CanSetAndResetLogHandlerMultipleTimes)
{
    RamsesFramework::SetLogHandler([](auto /*level*/, auto /*context*/, auto /*message*/) {});
    RamsesFramework::SetLogHandler(nullptr);

    RamsesFramework::SetLogHandler([](auto /*level*/, auto /*context*/, auto /*message*/) {});
    RamsesFramework::SetLogHandler(nullptr);

    RamsesFramework::SetLogHandler([](auto /*level*/, auto /*context*/, auto /*message*/) {});
    RamsesFramework::SetLogHandler([](auto /*level*/, auto /*context*/, auto /*message*/) {});
    RamsesFramework::SetLogHandler(nullptr);
    RamsesFramework::SetLogHandler(nullptr);
}

TEST(ARamsesFramework, multipleInstancesCanBeCreatedInParallel)
{
    // check stability of sensitive framework components (e.g. logger singleton) when used in multiple instances in parallel
    const size_t numThreads = 32u;
    ramses::internal::ThreadBarrier initBarrier(numThreads);
    ramses::internal::ThreadBarrier deinitBarrier(numThreads);

    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (size_t t = 0u; t < numThreads; ++t)
    {
        threads.emplace_back([&]() {
            RamsesFrameworkConfig config{ EFeatureLevel_Latest };
            initBarrier.wait();
            RamsesFramework fw{ config };
            std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
            deinitBarrier.wait();
            });
    }
    for (auto& t : threads)
        t.join();
}

class ARamsesFrameworkLogging : public ::testing::Test
{
protected:
    void SetUp() override
    {
        RamsesFramework::SetLogHandler([this](auto /*unused*/, auto /*unused*/, auto message) {
            m_logMessage = message;
            });
    }

    void TearDown() override
    {
        RamsesFramework::SetLogHandler(nullptr);
    }

    std::string m_logMessage;
};

TEST_F(ARamsesFrameworkLogging, SetLoggingInstanceName)
{
    // logging prefix name tests work with static thread_local variables, each test case needs to run in own thread to not affect other test cases
    std::thread t([&] {

        // initial default
        LOG_INFO(CONTEXT_FRAMEWORK, "test");
        EXPECT_EQ("R.main: test", m_logMessage);

        RamsesFrameworkConfig cfg{ EFeatureLevel_Latest };
        cfg.setLoggingInstanceName("instName");
        RamsesFramework fw{ cfg };

        LOG_INFO(CONTEXT_FRAMEWORK, "test");
        EXPECT_EQ("instName.main: test", m_logMessage);
        LOG_WARN(CONTEXT_FRAMEWORK, "test");
        EXPECT_EQ("instName.main: test", m_logMessage);
        LOG_ERROR(CONTEXT_FRAMEWORK, "test");
        EXPECT_EQ("instName.main: test", m_logMessage);

        });
    t.join();
}

TEST_F(ARamsesFrameworkLogging, SetLoggingPrefix)
{
    // logging prefix name tests work with static thread_local variables, each test case needs to run in own thread to not affect other test cases
    std::thread t([&] {

        // init default
        LOG_INFO(CONTEXT_FRAMEWORK, "test");
        EXPECT_EQ("R.main: test", m_logMessage);
        LOG_WARN(CONTEXT_FRAMEWORK, "test");
        EXPECT_EQ("R.main: test", m_logMessage);
        LOG_ERROR(CONTEXT_FRAMEWORK, "test");
        EXPECT_EQ("R.main: test", m_logMessage);

        RamsesLogger::SetPrefixes("I", "T", "A");
        LOG_INFO(CONTEXT_FRAMEWORK, "test");
        EXPECT_EQ("I.T.A: test", m_logMessage);
        LOG_WARN(CONTEXT_FRAMEWORK, "test");
        EXPECT_EQ("I.T.A: test", m_logMessage);
        LOG_ERROR(CONTEXT_FRAMEWORK, "test");
        EXPECT_EQ("I.T.A: test", m_logMessage);

        });
    t.join();
}

TEST_F(ARamsesFrameworkLogging, SetLoggingPrefixPerThread)
{
    // logging prefix name tests work with static thread_local variables, each test case needs to run in own thread to not affect other test cases
    std::thread t([&] {

        RamsesFrameworkConfig cfg{ EFeatureLevel_Latest };
        cfg.setLoggingInstanceName("instName1");
        RamsesFramework fw1{ cfg };
        LOG_INFO(CONTEXT_FRAMEWORK, "test");
        EXPECT_EQ("instName1.main: test", m_logMessage);

        std::thread t0([&] {
            cfg.setLoggingInstanceName("instName2");
            RamsesFramework fw2{ cfg };
            LOG_INFO(CONTEXT_FRAMEWORK, "test");
            EXPECT_EQ("instName2.main: test", m_logMessage);

            std::thread t1([&] {
                cfg.setLoggingInstanceName("instName3");
                RamsesFramework fw3{ cfg };
                LOG_INFO(CONTEXT_FRAMEWORK, "test");
                EXPECT_EQ("instName3.main: test", m_logMessage);

                std::thread t2([&] {
                    cfg.setLoggingInstanceName("instName4");
                    RamsesFramework fw4{ cfg };
                    LOG_INFO(CONTEXT_FRAMEWORK, "test");
                    EXPECT_EQ("instName4.main: test", m_logMessage);
                    });
                t2.join();

                LOG_INFO(CONTEXT_FRAMEWORK, "test");
                EXPECT_EQ("instName3.main: test", m_logMessage);
                });
            t1.join();

            LOG_INFO(CONTEXT_FRAMEWORK, "test");
            EXPECT_EQ("instName2.main: test", m_logMessage);
            });
        t0.join();

        // main thread not affected
        LOG_INFO(CONTEXT_FRAMEWORK, "test");
        EXPECT_EQ("instName1.main: test", m_logMessage);

        });
    t.join();
}
}
