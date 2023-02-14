//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "RamsesFrameworkImpl.h"
#include "ApiRamshCommandMock.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Utils/LogMacros.h"

using namespace ramses;
using namespace testing;

TEST(ARamsesFramework, canDefaultConstruct)
{
    RamsesFramework fw;
    EXPECT_GT(fw.impl.getParticipantAddress().getParticipantId().get(), 0xFF);
}

TEST(ARamsesFramework, canConstructFromConfig)
{
    const char* argv[] = {"", "--guid", "0000-000000000123"};
    RamsesFrameworkConfig config(3, argv);
    RamsesFramework fw(config);
    EXPECT_EQ(fw.impl.getParticipantAddress().getParticipantId().get(), 0x123);
}

TEST(ARamsesFramework, isNotConnectedInitially)
{
    RamsesFramework fw;
    EXPECT_FALSE(fw.isConnected());
}

TEST(ARamsesFramework, connectLifeCycleOK)
{
    RamsesFramework fw;
    EXPECT_EQ(ramses::StatusOK, fw.connect());
    EXPECT_TRUE(fw.isConnected());
    EXPECT_EQ(ramses::StatusOK, fw.disconnect());
}

TEST(ARamsesFramework, reportsErrorWhenConnectingSecondTime)
{
    RamsesFramework fw;
    fw.connect();
    EXPECT_NE(ramses::StatusOK, fw.connect());
    EXPECT_TRUE(fw.isConnected());
}

TEST(ARamsesFramework, reportsErrorWhenDisconnectingSecondTime)
{
    RamsesFramework fw;
    fw.connect();
    EXPECT_EQ(ramses::StatusOK, fw.disconnect());
    EXPECT_NE(ramses::StatusOK, fw.disconnect());
}

namespace
{
    class PartialApiRamshCommandMock : public ramses_internal::ApiRamshCommandMock
    {
    public:
        explicit PartialApiRamshCommandMock(const std::string& kw_)
            : kw(kw_)
        {}
        const std::string& keyword() const override { return kw; }
        const std::string& help() const override { return helpText; }
        std::string kw;
        std::string helpText{"text"};
    };
}

TEST(ARamsesFramework, canAddAndTriggerRamshCommands)
{
    auto cmd_a = std::make_shared<testing::StrictMock<PartialApiRamshCommandMock>>("foo");
    auto cmd_b = std::make_shared<testing::StrictMock<PartialApiRamshCommandMock>>("bar");

    RamsesFramework fw;
    EXPECT_EQ(StatusOK, fw.addRamshCommand(cmd_a));
    EXPECT_EQ(StatusOK, fw.addRamshCommand(cmd_b));

    std::string inp_a = "foo";
    EXPECT_CALL(*cmd_a, execute(std::vector<std::string>{"foo"})).WillOnce(Return(true));
    EXPECT_EQ(StatusOK, fw.executeRamshCommand(inp_a));

    std::string inp_b = "bar baz";
    EXPECT_CALL(*cmd_b, execute(std::vector<std::string>{"bar", "baz"})).WillOnce(Return(true));
    EXPECT_EQ(StatusOK, fw.executeRamshCommand(inp_b));

    cmd_a.reset();
    EXPECT_NE(StatusOK, fw.executeRamshCommand(inp_a));
}

TEST(ARamsesFramwork, canExecuteExistingAndFailingInvalidRamshCommands)
{
    RamsesFramework fw;
    EXPECT_EQ(StatusOK, fw.executeRamshCommand("help"));
    EXPECT_EQ(StatusOK, fw.executeRamshCommand("setLogLevelConsole trace"));
    EXPECT_NE(StatusOK, fw.executeRamshCommand("invalid ramsh command"));
    EXPECT_NE(StatusOK, fw.executeRamshCommand(""));
}

TEST(ARamsesFramework, failsToAddInvalidRamshCommands)
{
    RamsesFramework fw;
    EXPECT_NE(StatusOK, fw.addRamshCommand({}));
    EXPECT_NE(StatusOK, fw.addRamshCommand(std::make_shared<testing::StrictMock<PartialApiRamshCommandMock>>("")));
    EXPECT_NE(StatusOK, fw.addRamshCommand(std::make_shared<testing::StrictMock<PartialApiRamshCommandMock>>("a b")));
    EXPECT_NE(StatusOK, fw.addRamshCommand(std::make_shared<testing::StrictMock<PartialApiRamshCommandMock>>("a\"b")));
    EXPECT_NE(StatusOK, fw.addRamshCommand(std::make_shared<testing::StrictMock<PartialApiRamshCommandMock>>("help")));
}

TEST(ARamsesFramework, SetLogHandler)
{
    bool loggerCalled {false};

    RamsesFramework::SetLogHandler([&loggerCalled](auto level, auto& context, auto& message)
        {
            if (level == ELogLevel::Warn && context == "RFRA" && message == "SetLogHandlerTest")
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
    RamsesFramework::SetLogHandler([](auto /*level*/, auto& /*context*/, auto& /*message*/) {});
    RamsesFramework::SetLogHandler(nullptr);

    RamsesFramework::SetLogHandler([](auto /*level*/, auto& /*context*/, auto& /*message*/) {});
    RamsesFramework::SetLogHandler(nullptr);

    RamsesFramework::SetLogHandler([](auto /*level*/, auto& /*context*/, auto& /*message*/) {});
    RamsesFramework::SetLogHandler([](auto /*level*/, auto& /*context*/, auto& /*message*/) {});
    RamsesFramework::SetLogHandler(nullptr);
    RamsesFramework::SetLogHandler(nullptr);
}
