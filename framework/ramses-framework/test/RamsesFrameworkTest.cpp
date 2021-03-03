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
#include "Ramsh/Ramsh.h"

using namespace ramses;
using namespace testing;

TEST(ARamsesFramework, canDefaultConstruct)
{
    RamsesFramework fw;
    EXPECT_GT(fw.impl.getParticipantAddress().getParticipantId().get(), 0xFF);
}

TEST(ARamsesFramework, canConstructFromConfig)
{
    const char* argv[] = {"", "-guid", "0000-000000000123"};
    RamsesFrameworkConfig config(3, argv);
    RamsesFramework fw(config);
    EXPECT_EQ(fw.impl.getParticipantAddress().getParticipantId().get(), 0x123);
}


TEST(ARamsesFramework, canConstructWithArgcArgv)
{
    const char* argv[] = {"", "-guid", "0000-000000000124"};
    RamsesFramework fw(3, argv);
    EXPECT_EQ(fw.impl.getParticipantAddress().getParticipantId().get(), 0x124);
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

    std::vector<std::string> inp_a;
    inp_a.push_back("foo");
    EXPECT_CALL(*cmd_a, execute(std::vector<std::string>{"foo"})).WillOnce(Return(true));
    EXPECT_TRUE(fw.impl.getRamsh().execute(inp_a));

    std::vector<std::string> inp_b;
    inp_b.push_back("bar");
    inp_b.push_back("baz");
    EXPECT_CALL(*cmd_b, execute(std::vector<std::string>{"bar", "baz"})).WillOnce(Return(true));
    EXPECT_TRUE(fw.impl.getRamsh().execute(inp_b));

    cmd_a.reset();
    EXPECT_FALSE(fw.impl.getRamsh().execute(inp_a));
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
