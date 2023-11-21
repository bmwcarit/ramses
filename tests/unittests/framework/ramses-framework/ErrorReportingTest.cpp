//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "impl/ErrorReporting.h"
#include "impl/RamsesObjectImpl.h"
#include "ramses/framework/RamsesObject.h"
#include "internal/Core/Utils/RamsesLogger.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/ThreadBarrier.h"
#include "LogTestUtils.h"
#include <thread>

namespace ramses::internal
{
    class DummyObjectImpl : public RamsesObjectImpl
    {
    public:
        DummyObjectImpl(ERamsesObjectType type, std::string_view name)
            : RamsesObjectImpl(type, name)
        {}

        void deinitializeFrameworkData() override {};
    };
    class DummyObject : public RamsesObject
    {
    public:
        explicit DummyObject(std::string_view name)
            : RamsesObject{ std::make_unique<DummyObjectImpl>(ERamsesObjectType::Node, name) }
        {
        }
        ~DummyObject() override = default;
    };

    class AErrorReporting : public ::testing::Test
    {
    protected:
        ErrorReporting           m_errorReporting;
        std::vector<std::string> m_loggedErrors;
        ScopedLogContextLevel    m_logCollector{CONTEXT_CLIENT, ELogLevel::Info, [this](ELogLevel type, std::string_view message) {
                                                    EXPECT_EQ(ELogLevel::Error, type);
                                                    m_loggedErrors.emplace_back(std::string(message));
                                                }};

        DummyObject object1{ "obj1" };
        DummyObject object2{ "obj2" };
    };

    TEST_F(AErrorReporting, HasNoErrorInitially)
    {
        EXPECT_FALSE(m_errorReporting.getError().has_value());
    }

    TEST_F(AErrorReporting, ProducesNoLogsDuringConstruction)
    {
        EXPECT_EQ(0u, m_loggedErrors.size());
    }

    TEST_F(AErrorReporting, StoresAffectedObjectWhenProvided)
    {
        m_errorReporting.set("error 1", &object1);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "error 1");
        EXPECT_EQ(m_errorReporting.getError()->object, &object1);
        EXPECT_EQ(m_errorReporting.getError()->type, EIssueType::Error);

        m_errorReporting.set("error 2", &object2);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "error 2");
        EXPECT_EQ(m_errorReporting.getError()->object, &object2);
        EXPECT_EQ(m_errorReporting.getError()->type, EIssueType::Error);
    }

    TEST_F(AErrorReporting, StoresAffectedObjectWhenProvidedAsImpl)
    {
        m_errorReporting.set("error 1", object1.impl());
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "error 1");
        EXPECT_EQ(m_errorReporting.getError()->object, &object1);
        EXPECT_EQ(m_errorReporting.getError()->type, EIssueType::Error);

        m_errorReporting.set("error 2", object2.impl());
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "error 2");
        EXPECT_EQ(m_errorReporting.getError()->object, &object2);
        EXPECT_EQ(m_errorReporting.getError()->type, EIssueType::Error);
    }

    TEST_F(AErrorReporting, LogsErrorsInTheOrderAdded)
    {
        m_errorReporting.set("error 1", &object1);
        m_errorReporting.set("error 2", &object2);

        ASSERT_EQ(m_loggedErrors.size(), 2u);
        EXPECT_EQ(m_loggedErrors[0], "R.main: [obj1] error 1");
        EXPECT_EQ(m_loggedErrors[1], "R.main: [obj2] error 2");
    }

    TEST_F(AErrorReporting, ClearsError)
    {
        m_errorReporting.set("error 1", nullptr);
        EXPECT_TRUE(m_errorReporting.getError().has_value());

        m_errorReporting.reset();
        EXPECT_FALSE(m_errorReporting.getError().has_value());
    }

    TEST_F(AErrorReporting, IsThreadsafe)
    {
        ThreadBarrier initDoneBarrier(3);
        ThreadBarrier setDoneBarrier(3);
        ThreadBarrier getDone(3);

        std::thread t1([&]() {
            initDoneBarrier.wait();
            m_errorReporting.set("error 1", nullptr);
            setDoneBarrier.wait();
            EXPECT_TRUE(m_errorReporting.getError().has_value());
            getDone.wait();
            });
        std::thread t2([&]() {
            initDoneBarrier.wait();
            m_errorReporting.set("error 2", nullptr);
            setDoneBarrier.wait();
            EXPECT_TRUE(m_errorReporting.getError().has_value());
            getDone.wait();
            });
        std::thread t3([&]() {
            initDoneBarrier.wait();
            m_errorReporting.set("error 3", nullptr);
            setDoneBarrier.wait();
            EXPECT_TRUE(m_errorReporting.getError().has_value());
            getDone.wait();
            });
        t1.join();
        t2.join();
        t3.join();
    }
}
