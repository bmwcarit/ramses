//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "internals/ErrorReporting.h"
#include "ramses-logic/Logger.h"
#include "LogicNodeDummy.h"

namespace rlogic::internal
{
    class AErrorReporting : public ::testing::Test
    {
    protected:
        AErrorReporting()
        {
            // Explicitly check that default logging does not affect custom error logs
            Logger::SetDefaultLogging(false);

            Logger::SetLogHandler([this](ELogMessageType type, std::string_view message) {
                EXPECT_EQ(ELogMessageType::Error, type);
                m_loggedErrors.emplace_back(std::string(message));
                });
        }

        void TearDown() override
        {
            // Unset custom logger to avoid interference with other tests which use logs
            Logger::SetLogHandler({});
        }

        ErrorReporting       m_errorReporting;

        std::vector<std::string> m_loggedErrors;
    };

    TEST_F(AErrorReporting, ProducesNoErrorsDuringConstruction)
    {
        EXPECT_EQ(0u, m_errorReporting.getErrors().size());
    }

    TEST_F(AErrorReporting, ProducesNoLogsDuringConstruction)
    {
        EXPECT_EQ(0u, m_loggedErrors.size());
    }

    TEST_F(AErrorReporting, StoresSourceLogicObjectWhenProvided)
    {
        class TestObject : public LogicObject
        {
        public:
            TestObject() : LogicObject(std::make_unique<LogicObjectImpl>("", 0))
            {
            }
        };
        TestObject object1;
        TestObject object2;

        m_errorReporting.add("error 1", &object1, EErrorType::ContentStateError);
        m_errorReporting.add("error 2", &object2, EErrorType::BinaryVersionMismatch);

        ASSERT_EQ(m_errorReporting.getErrors().size(), 2u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "error 1");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, &object1);
        EXPECT_EQ(m_errorReporting.getErrors()[0].type, EErrorType::ContentStateError);
        EXPECT_EQ(m_errorReporting.getErrors()[1].message, "error 2");
        EXPECT_EQ(m_errorReporting.getErrors()[1].object, &object2);
        EXPECT_EQ(m_errorReporting.getErrors()[1].type, EErrorType::BinaryVersionMismatch);
    }

    TEST_F(AErrorReporting, StoresErrorsInTheOrderAdded)
    {
        m_errorReporting.add("error 1", nullptr, EErrorType::IllegalArgument);
        m_errorReporting.add("error 2", nullptr, EErrorType::IllegalArgument);

        ASSERT_EQ(m_errorReporting.getErrors().size(), 2u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "error 1");
        EXPECT_EQ(m_errorReporting.getErrors()[1].message, "error 2");
    }

    TEST_F(AErrorReporting, LogsErrorsInTheOrderAdded)
    {
        m_errorReporting.add("error 1", nullptr, EErrorType::IllegalArgument);
        m_errorReporting.add("error 2", nullptr, EErrorType::IllegalArgument);

        ASSERT_EQ(m_loggedErrors.size(), 2u);
        EXPECT_EQ(m_loggedErrors[0], "error 1");
        EXPECT_EQ(m_loggedErrors[1], "error 2");

    }

    TEST_F(AErrorReporting, ClearsErrors)
    {
        m_errorReporting.add("error 1", nullptr, EErrorType::IllegalArgument);

        ASSERT_EQ(1u, m_errorReporting.getErrors().size());

        m_errorReporting.clear();

        ASSERT_TRUE(m_errorReporting.getErrors().empty());
    }
}
