//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Scene/DataLayoutCachedScene.h"

using namespace testing;

namespace ramses_internal
{
    class ADataLayoutCachedScene : public testing::Test
    {
    public:
        ADataLayoutCachedScene()
            : scene(SceneInfo())
        {
        }

    protected:
        DataLayoutCachedScene scene;
    };

    TEST_F(ADataLayoutCachedScene, canAllocateDataLayout)
    {
        const DataLayoutHandle dataLayout = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, ResourceContentHash(123u, 0u));
        EXPECT_TRUE(dataLayout.isValid());
        EXPECT_EQ(1u, scene.getNumDataLayoutReferences(dataLayout));
    }

    TEST_F(ADataLayoutCachedScene, canAllocateEmptyDataLayout)
    {
        const DataLayoutHandle dataLayout = scene.allocateDataLayout({}, ResourceContentHash(123u, 0u));
        EXPECT_TRUE(dataLayout.isValid());
        EXPECT_EQ(1u, scene.getNumDataLayoutReferences(dataLayout));
    }

    TEST_F(ADataLayoutCachedScene, willGiveSameHandleForExistingLayout)
    {
        const DataLayoutHandle dataLayout1 = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float, 2u, EFixedSemantics_ModelMatrix) }, ResourceContentHash(123u, 0u));
        const DataLayoutHandle dataLayout2 = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float, 2u, EFixedSemantics_ModelMatrix) }, ResourceContentHash(123u, 0u));
        EXPECT_EQ(dataLayout1, dataLayout2);
        EXPECT_EQ(2u, scene.getNumDataLayoutReferences(dataLayout1));
    }

    TEST_F(ADataLayoutCachedScene, willAllocateNewHandleForNonMatchingDataFieldTypes)
    {
        DataLayoutHandle dataLayout1 = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, {});
        DataLayoutHandle dataLayout2 = scene.allocateDataLayout({ DataFieldInfo(EDataType_Int32) }, {});
        EXPECT_NE(dataLayout1, dataLayout2);
        EXPECT_EQ(1u, scene.getNumDataLayoutReferences(dataLayout1));
        EXPECT_EQ(1u, scene.getNumDataLayoutReferences(dataLayout2));
    }

    TEST_F(ADataLayoutCachedScene, willAllocateNewHandleForNonMatchingDataFieldElementCounts)
    {
        DataLayoutHandle dataLayout1 = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float, 2u) }, {});
        DataLayoutHandle dataLayout2 = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float, 3u) }, {});
        EXPECT_NE(dataLayout1, dataLayout2);
        EXPECT_EQ(1u, scene.getNumDataLayoutReferences(dataLayout1));
        EXPECT_EQ(1u, scene.getNumDataLayoutReferences(dataLayout2));
    }

    TEST_F(ADataLayoutCachedScene, willAllocateNewHandleForNonMatchingDataFieldSemantics)
    {
        const auto dataLayout1 = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float, 1u, EFixedSemantics_CameraViewMatrix) }, {});
        const auto dataLayout2 = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float, 1u, EFixedSemantics_ModelMatrix) }, {});
        EXPECT_NE(dataLayout1, dataLayout2);
        EXPECT_EQ(1u, scene.getNumDataLayoutReferences(dataLayout1));
        EXPECT_EQ(1u, scene.getNumDataLayoutReferences(dataLayout2));
    }

    TEST_F(ADataLayoutCachedScene, willAllocateNewHandleForNonMatchingEffectHash)
    {
        const auto dataLayout1 = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, ResourceContentHash{123u, 0u});
        const auto dataLayout2 = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, ResourceContentHash{124u, 0u});
        EXPECT_NE(dataLayout1, dataLayout2);
        EXPECT_EQ(1u, scene.getNumDataLayoutReferences(dataLayout1));
        EXPECT_EQ(1u, scene.getNumDataLayoutReferences(dataLayout2));
    }

    TEST_F(ADataLayoutCachedScene, keepsDataLayoutUntilItsUsageCountDropsToZero)
    {
        const DataLayoutHandle dataLayout = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, ResourceContentHash(123u, 0u));
        EXPECT_EQ(dataLayout, scene.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, ResourceContentHash(123u, 0u)));
        EXPECT_EQ(dataLayout, scene.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, ResourceContentHash(123u, 0u)));
        EXPECT_EQ(3u, scene.getNumDataLayoutReferences(dataLayout));

        scene.releaseDataLayout(dataLayout);
        EXPECT_TRUE(scene.isDataLayoutAllocated(dataLayout));
        EXPECT_EQ(2u, scene.getNumDataLayoutReferences(dataLayout));

        scene.releaseDataLayout(dataLayout);
        EXPECT_TRUE(scene.isDataLayoutAllocated(dataLayout));
        EXPECT_EQ(1u, scene.getNumDataLayoutReferences(dataLayout));

        scene.releaseDataLayout(dataLayout);
        EXPECT_FALSE(scene.isDataLayoutAllocated(dataLayout));
    }

    TEST_F(ADataLayoutCachedScene, confidenceTest_canHandleCreationAndCacheLargeDataLayout)
    {
        const DataLayoutHandle dataLayoutPrevious = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, ResourceContentHash(123u, 0u));

        const DataFieldInfoVector dataFields(100u, DataFieldInfo(EDataType_Int32));
        const DataLayoutHandle dataLayout1 = scene.allocateDataLayout(dataFields, ResourceContentHash(123u, 0u));
        const DataLayoutHandle dataLayout2 = scene.allocateDataLayout(dataFields, ResourceContentHash(123u, 0u));
        EXPECT_TRUE(dataLayout1.isValid());
        EXPECT_EQ(dataLayout1, dataLayout2);
        EXPECT_EQ(2u, scene.getNumDataLayoutReferences(dataLayout1));

        // check that previous data layout exists and is cached
        EXPECT_TRUE(scene.isDataLayoutAllocated(dataLayoutPrevious));
        EXPECT_EQ(dataLayoutPrevious, scene.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, ResourceContentHash(123u, 0u)));
        EXPECT_EQ(2u, scene.getNumDataLayoutReferences(dataLayout1));
    }
}
