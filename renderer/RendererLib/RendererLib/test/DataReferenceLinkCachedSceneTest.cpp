//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/DataReferenceLinkCachedScene.h"
#include "RendererLib/RendererScenes.h"
#include "RendererEventCollector.h"
#include "SceneAllocateHelper.h"

using namespace testing;
using namespace ramses_internal;

class ADataReferenceLinkCachedScene : public ::testing::Test
{
public:
    ADataReferenceLinkCachedScene()
        : rendererScenes(rendererEventCollector)
        , scene(rendererScenes.createScene(SceneInfo(SceneId(3u))))
        , sceneAllocator(scene)
    {
        const DataLayoutHandle layout = sceneAllocator.allocateDataLayout({ DataFieldInfo(EDataType_Int32) });
        dataRef = sceneAllocator.allocateDataInstance(layout);

        dataSlot = sceneAllocator.allocateDataSlot({ EDataSlotType_DataConsumer, DataSlotId(1u), NodeHandle(), dataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() });
    }

protected:
    RendererEventCollector rendererEventCollector;
    RendererScenes rendererScenes;
    DataReferenceLinkCachedScene& scene;
    SceneAllocateHelper sceneAllocator;

    DataInstanceHandle dataRef;
    DataSlotHandle dataSlot;
};

TEST_F(ADataReferenceLinkCachedScene, canSetAndGetValue)
{
    scene.setDataSingleInteger(dataRef, DataFieldHandle(0u), 13);
    EXPECT_EQ(13, scene.getDataSingleInteger(dataRef, DataFieldHandle(0u)));
}

TEST_F(ADataReferenceLinkCachedScene, canSetValueDirectlyWithoutUpdatingFallbackValue)
{
    Variant value;
    value.setValue(33);
    scene.setValueWithoutUpdatingFallbackValue(dataRef, DataFieldHandle(0u), value);
    EXPECT_EQ(33, scene.getDataSingleInteger(dataRef, DataFieldHandle(0u)));
}

TEST_F(ADataReferenceLinkCachedScene, canRestoreFallbackValue)
{
    scene.setDataSingleInteger(dataRef, DataFieldHandle(0u), 13);

    Variant value;
    value.setValue(33);
    scene.setValueWithoutUpdatingFallbackValue(dataRef, DataFieldHandle(0u), value);

    scene.restoreFallbackValue(dataRef, DataFieldHandle(0u));
    EXPECT_EQ(13, scene.getDataSingleInteger(dataRef, DataFieldHandle(0u)));
}

TEST_F(ADataReferenceLinkCachedScene, storesFallbackValueWhenSlotAllocated)
{
    scene.releaseDataSlot(dataSlot);

    scene.setDataSingleInteger(dataRef, DataFieldHandle(0u), 13);

    sceneAllocator.allocateDataSlot({ EDataSlotType_DataConsumer, DataSlotId(1u), NodeHandle(), dataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() });

    scene.restoreFallbackValue(dataRef, DataFieldHandle(0u));
    EXPECT_EQ(13, scene.getDataSingleInteger(dataRef, DataFieldHandle(0u)));
}
