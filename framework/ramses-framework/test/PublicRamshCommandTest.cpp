//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PublicRamshCommand.h"
#include "ApiRamshCommandMock.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    using namespace ::testing;

    class APublicRamshCommand : public Test
    {
    public:
        std::shared_ptr<StrictMock<ApiRamshCommandMock>> mock = std::make_shared<StrictMock<ApiRamshCommandMock>>();
        std::string keyword{"kw"};
        std::string help{"some text"};
    };

    TEST_F(APublicRamshCommand, canConstruct)
    {
        EXPECT_CALL(*mock, keyword()).WillOnce(ReturnRef(keyword));
        EXPECT_CALL(*mock, help()).WillOnce(ReturnRef(help));
        PublicRamshCommand cmd(mock);
        EXPECT_EQ(keyword, cmd.keywordString());
        EXPECT_EQ(1u, cmd.keywords().size());
        EXPECT_EQ(help, cmd.descriptionString());
    }

    TEST_F(APublicRamshCommand, canExecute)
    {
        EXPECT_CALL(*mock, keyword()).WillOnce(ReturnRef(keyword));
        EXPECT_CALL(*mock, help()).WillOnce(ReturnRef(help));
        PublicRamshCommand cmd(mock);

        {
            std::vector<std::string> inp;
            inp.push_back("asd");
            inp.push_back("def");
            EXPECT_CALL(*mock, execute(std::vector<std::string>{"asd", "def"})).WillOnce(Return(true));
            EXPECT_TRUE(cmd.executeInput(inp));
        }
        {
            std::vector<std::string> inp;
            EXPECT_CALL(*mock, execute(std::vector<std::string>{})).WillOnce(Return(true));
            EXPECT_TRUE(cmd.executeInput(inp));
        }
    }

    TEST_F(APublicRamshCommand, executePassesThroughReturnValue)
    {
        EXPECT_CALL(*mock, keyword()).WillOnce(ReturnRef(keyword));
        EXPECT_CALL(*mock, help()).WillOnce(ReturnRef(help));
        PublicRamshCommand cmd(mock);

        std::vector<std::string> inp;
        EXPECT_CALL(*mock, execute(std::vector<std::string>{})).WillOnce(Return(false));
        EXPECT_FALSE(cmd.executeInput(inp));
    }

    TEST_F(APublicRamshCommand, failsGracefullyWhenWrappedCommandBecameUnavailable)
    {
        EXPECT_CALL(*mock, keyword()).WillOnce(ReturnRef(keyword));
        EXPECT_CALL(*mock, help()).WillOnce(ReturnRef(help));
        PublicRamshCommand cmd(mock);

        mock.reset();
        std::vector<std::string> inp;
        EXPECT_FALSE(cmd.executeInput(inp));
    }
}
