//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "RamsesFrameworkImpl.h"

using namespace ramses;
using namespace ramses_internal;

TEST(ARamsesFrameworkImpl, canCreateFrameworkImplFromDefaultConfig)
{
    RamsesFrameworkConfig config;
    RamsesFrameworkImpl& impl = RamsesFrameworkImpl::createImpl(config);

    delete &impl;
}

TEST(ARamsesFrameworkImpl, defaultConfigGeneratesGuidNotStartingWithZeros)
{
    // starting with zeros is reserved for hard coded values for easier debugging
    RamsesFrameworkConfig config;
    RamsesFrameworkImpl& impl = RamsesFrameworkImpl::createImpl(config);

    EXPECT_FALSE(impl.getParticipantAddress().getParticipantId().isInvalid());
    EXPECT_GT(impl.getParticipantAddress().getParticipantId().getLow64(), 0xFF);

    delete &impl;
}
