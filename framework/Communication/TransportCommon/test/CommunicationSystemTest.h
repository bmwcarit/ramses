//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMUNICATION_COMMUNICATIONSYSTEMTEST_H
#define RAMSES_COMMUNICATION_COMMUNICATIONSYSTEMTEST_H

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "gmock/gmock-generated-nice-strict.h"
#include "MockConnectionStatusListener.h"
#include "CommunicationSystemTestWrapper.h"
#include "CommunicationSystemTestFactory.h"

namespace ramses_internal
{
    using namespace testing;

    class ACommunicationSystem : public ::testing::TestWithParam<ECommunicationSystemType>
    {
    public:
        ACommunicationSystem()
            : state(CommunicationSystemTestFactory::ConstructTestState(GetParam(), EServiceType::Ramses))
        {
        }

        ~ACommunicationSystem()
        {
        }

        std::unique_ptr<CommunicationSystemTestState> state;
        StrictMock<MockConnectionStatusListener> listener;
    };

    class ACommunicationSystemWithDaemon : public ::testing::TestWithParam<std::tuple<ECommunicationSystemType, EServiceType>>
    {
    public:
        ACommunicationSystemWithDaemon()
            : state(CommunicationSystemTestFactory::ConstructTestState(std::get<0>(GetParam()), std::get<1>(GetParam())))
            , daemon(CommunicationSystemTestFactory::ConstructDiscoveryDaemonTestWrapper(*state))
        {
        }

        ~ACommunicationSystemWithDaemon()
        {
        }

        void SetUp() override
        {
            EXPECT_TRUE(daemon->start());
        }

        void TearDown() override
        {
            EXPECT_TRUE(daemon->stop());
        }

        std::unique_ptr<CommunicationSystemTestState> state;
        std::unique_ptr<CommunicationSystemDiscoveryDaemonTestWrapper> daemon;
    };

#define TESTING_SERVICETYPE_RAMSES(commsysProvider) \
    ::testing::Combine(::testing::ValuesIn(commsysProvider), ::testing::Values(EServiceType::Ramses))
#define TESTING_SERVICETYPE_DCSM(commsysProvider) \
    ::testing::Combine(::testing::ValuesIn(commsysProvider), ::testing::Values(EServiceType::Dcsm))
}

#endif
