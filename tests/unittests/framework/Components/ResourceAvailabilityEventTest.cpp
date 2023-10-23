//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Components/ResourceAvailabilityEvent.h"

#include <gtest/gtest.h>

namespace ramses::internal
{
    bool checkEqual(ResourceAvailabilityEvent const& a, ResourceAvailabilityEvent const& b)
    {
        return
            a.sceneid == b.sceneid &&
            a.availableResources == b.availableResources;
    }

    TEST(AResourceAvailabilityEvent, hasSameValuesAfterWritingAndReading)
    {
        ResourceAvailabilityEvent a;
        a.sceneid = SceneId(123u);
        ResourceAvailabilityEvent b;
        b.sceneid = SceneId(123u);
        std::vector<std::byte> testVec;

        a.writeToBlob(testVec);
        b.readFromBlob(testVec);
        EXPECT_TRUE(checkEqual(a, b));

        a.sceneid = SceneId{ 1234 };
        a.availableResources.emplace_back(13u,14u);

        EXPECT_FALSE(checkEqual(a, b));

        testVec.clear();
        a.writeToBlob(testVec);
        b.readFromBlob(testVec);
        EXPECT_TRUE(checkEqual(a, b));
    }
}
