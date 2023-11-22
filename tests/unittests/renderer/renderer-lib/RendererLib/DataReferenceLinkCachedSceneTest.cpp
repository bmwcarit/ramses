//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/DataReferenceLinkCachedScene.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "SceneAllocateHelper.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    using namespace testing;

    class ADataReferenceLinkCachedScene : public ::testing::Test
    {
    public:
        ADataReferenceLinkCachedScene()
            : rendererScenes(rendererEventCollector)
            , scene(rendererScenes.createScene(SceneInfo(SceneId(3u))))
            , sceneAllocator(scene)
        {
            const DataLayoutHandle layout = sceneAllocator.allocateDataLayout({ DataFieldInfo(EDataType::Int32) }, ResourceContentHash::Invalid());
            dataRef = sceneAllocator.allocateDataInstance(layout);

            dataSlot = sceneAllocator.allocateDataSlot({ EDataSlotType::DataConsumer, DataSlotId(1u), NodeHandle(), dataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() });
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
        scene.setValueWithoutUpdatingFallbackValue(dataRef, DataFieldHandle(0u), DataInstanceValueVariant(33));
        EXPECT_EQ(33, scene.getDataSingleInteger(dataRef, DataFieldHandle(0u)));
    }

    TEST_F(ADataReferenceLinkCachedScene, canRestoreFallbackValue)
    {
        scene.setDataSingleInteger(dataRef, DataFieldHandle(0u), 13);

        scene.setValueWithoutUpdatingFallbackValue(dataRef, DataFieldHandle(0u), DataInstanceValueVariant(33));

        scene.restoreFallbackValue(dataRef, DataFieldHandle(0u));
        EXPECT_EQ(13, scene.getDataSingleInteger(dataRef, DataFieldHandle(0u)));
    }

    TEST_F(ADataReferenceLinkCachedScene, storesFallbackValueWhenSlotAllocated)
    {
        scene.releaseDataSlot(dataSlot);

        scene.setDataSingleInteger(dataRef, DataFieldHandle(0u), 13);

        sceneAllocator.allocateDataSlot({ EDataSlotType::DataConsumer, DataSlotId(1u), NodeHandle(), dataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() });

        scene.restoreFallbackValue(dataRef, DataFieldHandle(0u));
        EXPECT_EQ(13, scene.getDataSingleInteger(dataRef, DataFieldHandle(0u)));
    }
}
