//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/Core/Common/ParticipantIdentifier.h"

using namespace testing;

namespace ramses::internal
{
    TEST(ParticipantIdentifier, Constructor)
    {
        Guid id(777);

        ParticipantIdentifier identifier(id, "MyIdentifier");

        EXPECT_EQ(id, identifier.getParticipantId());
        EXPECT_STREQ("MyIdentifier", identifier.getParticipantName().c_str());
    }

    TEST(ParticipantIdentifier, IsNotEQualWhenIdsDiffer)
    {
        Guid id1(444);
        ParticipantIdentifier identifier1(id1, "MyIdentifier");
        Guid id2(5465);
        ParticipantIdentifier identifier2(id2, "MyOtherIdentifier");

        EXPECT_FALSE(identifier1 == identifier2);
    }

    TEST(ParticipantIdentifier, IsEqalWhenIdsAreSame)
    {
        Guid id1(998879);
        ParticipantIdentifier identifier1(id1, "MyIdentifier");
        ParticipantIdentifier identifier2(id1, "MyOtherIdentifier");

        EXPECT_TRUE(identifier1 == identifier2);
    }
}
