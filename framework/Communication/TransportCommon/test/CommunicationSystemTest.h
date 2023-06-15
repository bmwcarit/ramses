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
#include "MockConnectionStatusListener.h"
#include "CommunicationSystemTestWrapper.h"
#include "TransportCommon/IConnectionStatusUpdateNotifier.h"

namespace ramses_internal
{
    using namespace testing;

    class ACommunicationSystem : public ::testing::TestWithParam<ECommunicationSystemType>
    {
    public:
        std::unique_ptr<CommunicationSystemTestState> state{std::make_unique<CommunicationSystemTestState>(GetParam(), EServiceType::Ramses)};
        StrictMock<MockConnectionStatusListener> listener;
    };

    class ACommunicationSystemWithDaemon : public ::testing::TestWithParam<std::tuple<ECommunicationSystemType, EServiceType>>
    {
    public:
        void SetUp() override
        {
            EXPECT_TRUE(daemon->start());
        }

        void TearDown() override
        {
            EXPECT_TRUE(daemon->stop());
        }

        std::unique_ptr<CommunicationSystemTestState> state{std::make_unique<CommunicationSystemTestState>(std::get<0>(GetParam()), std::get<1>(GetParam()))};
        std::unique_ptr<ConnectionSystemTestDaemon> daemon{std::make_unique<ConnectionSystemTestDaemon>()};
    };

#define TESTING_SERVICETYPE_RAMSES(commsysProvider) \
    ::testing::Combine(::testing::ValuesIn(commsysProvider), ::testing::Values(EServiceType::Ramses))
}

#endif
