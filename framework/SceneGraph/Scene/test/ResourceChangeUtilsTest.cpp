//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneUtils/ResourceChangeUtils.h"
#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"

using namespace testing;

namespace ramses_internal
{
    class ResourceChangeUtilsTest : public testing::Test
    {
    public:
        ResourceContentHashVector addedResources;
        ResourceContentHashVector removedResources;

        const ResourceContentHash hash1 = { 1, 2 };
    };

    TEST_F(ResourceChangeUtilsTest, willPutNewlyAddedResourceToAddedList)
    {
        ResourceChangeUtils::ConsolidateResource(addedResources, removedResources, hash1);
        ASSERT_FALSE(addedResources.empty());
        EXPECT_TRUE(removedResources.empty());
        EXPECT_EQ(hash1, addedResources.front());
    }

    TEST_F(ResourceChangeUtilsTest, willPutNewlyRemovedResourceToRemovedList)
    {
        ResourceChangeUtils::ConsolidateResource(removedResources, addedResources, hash1);
        EXPECT_TRUE(addedResources.empty());
        ASSERT_FALSE(removedResources.empty());
        EXPECT_EQ(hash1, removedResources.front());
    }

    TEST_F(ResourceChangeUtilsTest, willConsolidateResourceThatWasRemovedAndAdded)
    {
        ResourceChangeUtils::ConsolidateResource(removedResources, addedResources, hash1);
        ResourceChangeUtils::ConsolidateResource(addedResources, removedResources, hash1);
        EXPECT_TRUE(addedResources.empty());
        EXPECT_TRUE(removedResources.empty());
    }

    TEST_F(ResourceChangeUtilsTest, willConsolidateResourceThatWasAddedAndRemoved)
    {
        ResourceChangeUtils::ConsolidateResource(addedResources, removedResources, hash1);
        ResourceChangeUtils::ConsolidateResource(removedResources, addedResources, hash1);
        EXPECT_TRUE(addedResources.empty());
        EXPECT_TRUE(removedResources.empty());
    }
}
