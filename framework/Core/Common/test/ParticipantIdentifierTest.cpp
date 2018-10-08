//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Common/ParticipantIdentifier.h"

using namespace testing;

namespace ramses_internal
{
    TEST(ParticipantIdentifier, Constructor)
    {
        Guid id(true);

        ParticipantIdentifier identifier(id, "MyIdentifier");

        EXPECT_EQ(id, identifier.getParticipantId());
        EXPECT_STREQ("MyIdentifier", identifier.getParticipantName().c_str());
    }

    TEST(ParticipantIdentifier, IsNotEQualWhenIdsDiffer)
    {
        Guid id1(true);
        ParticipantIdentifier identifier1(id1, "MyIdentifier");
        Guid id2(true);
        ParticipantIdentifier identifier2(id2, "MyOtherIdentifier");

        EXPECT_FALSE(identifier1 == identifier2);
    }

    TEST(ParticipantIdentifier, IsEqalWhenIdsAreSame)
    {
        Guid id1(true);
        ParticipantIdentifier identifier1(id1, "MyIdentifier");
        ParticipantIdentifier identifier2(id1, "MyOtherIdentifier");

        EXPECT_TRUE(identifier1 == identifier2);
    }
}
