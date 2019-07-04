//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/DcsmConsumer.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "DcsmEventHandlerMocks.h"
#include <gmock/gmock.h>

namespace ramses
{
    using namespace testing;

    class ADcsmConsumer : public ::testing::Test
    {
    public:
        ADcsmConsumer()
        {}

        RamsesFramework fw;
        StrictMock<ramses_internal::DcsmConsumerEventHandlerMock> handler;
    };

    TEST_F(ADcsmConsumer, canCreateAndDestroy)
    {
        DcsmConsumer* c = fw.createDcsmConsumer();
        ASSERT_TRUE(c != nullptr);
        EXPECT_EQ(StatusOK, fw.destroyDcsmConsumer(*c));
    }

    TEST_F(ADcsmConsumer, canCreateAndLetFrameworkDestroy)
    {
        DcsmConsumer* c = fw.createDcsmConsumer();
        ASSERT_TRUE(c != nullptr);
    }

    TEST_F(ADcsmConsumer, cannotCreateTwiceOnSameFramework)
    {
        DcsmConsumer* c1 = fw.createDcsmConsumer();
        ASSERT_TRUE(c1 != nullptr);
        DcsmConsumer* c2 = fw.createDcsmConsumer();
        ASSERT_TRUE(c2 == nullptr);
    }

    TEST_F(ADcsmConsumer, cannotDestroyConsumerFromOtherFramework)
    {
        DcsmConsumer* c = fw.createDcsmConsumer();
        ASSERT_TRUE(c != nullptr);

        RamsesFramework otherFw;
        EXPECT_NE(StatusOK, otherFw.destroyDcsmConsumer(*c));
    }

    TEST_F(ADcsmConsumer, canCreateAndDestroyConsumerOnDifferentFrameworksSimultaneously)
    {
        DcsmConsumer* c1 = fw.createDcsmConsumer();
        ASSERT_TRUE(c1 != nullptr);

        RamsesFramework otherFw;
        DcsmConsumer* c2 = otherFw.createDcsmConsumer();
        ASSERT_TRUE(c2 != nullptr);

        EXPECT_EQ(StatusOK, fw.destroyDcsmConsumer(*c1));
        EXPECT_EQ(StatusOK, otherFw.destroyDcsmConsumer(*c2));
    }

    TEST_F(ADcsmConsumer, emptyDispatchSucceeds)
    {
        DcsmConsumer* c = fw.createDcsmConsumer();
        ASSERT_TRUE(c != nullptr);
        EXPECT_EQ(StatusOK, c->dispatchEvents(handler));
    }

    TEST_F(ADcsmConsumer, sendCallWithUnknownContentFails)
    {
        DcsmConsumer* c = fw.createDcsmConsumer();
        EXPECT_NE(StatusOK, c->assignContentToConsumer(ContentID{123}, SizeInfo{1, 2}));
        EXPECT_NE(StatusOK, c->contentSizeChange(ContentID{123}, SizeInfo{1, 2}, AnimationInformation{0, 0}));
        EXPECT_NE(StatusOK, c->contentStateChange(ContentID{123}, EDcsmState::Offered, AnimationInformation{0, 0}));
        EXPECT_NE(StatusOK, c->acceptStopOffer(ContentID{123}, AnimationInformation{0, 0}));
    }
}
