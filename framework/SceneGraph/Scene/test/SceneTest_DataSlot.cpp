//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "SceneTest.h"

using namespace testing;

namespace ramses_internal
{
    TYPED_TEST_CASE(AScene, SceneTypes);

    TYPED_TEST(AScene, ContainsZeroTotalDataSlotsUponCreation)
    {
        EXPECT_EQ(0u, this->m_scene.getDataSlotCount());
    }

    TYPED_TEST(AScene, CreatesATransformDataSlot)
    {
        const DataSlotHandle requestedHandle(0);
        const DataInstanceHandle dataRef(13u);
        const ResourceContentHash textureRes(123u, 0);
        const TextureSamplerHandle sampler(16u);
        const DataSlotHandle handle = this->m_scene.allocateDataSlot({ EDataSlotType_TransformationConsumer, DataSlotId(6), NodeHandle(5), dataRef, textureRes, sampler }, requestedHandle);

        const DataSlot& dataSlot = this->m_scene.getDataSlot(handle);
        EXPECT_EQ(handle, requestedHandle);
        EXPECT_EQ(1u, this->m_scene.getDataSlotCount());
        EXPECT_TRUE(this->m_scene.isDataSlotAllocated(handle));
        EXPECT_EQ(EDataSlotType_TransformationConsumer, dataSlot.type);
        EXPECT_EQ(DataSlotId(6), dataSlot.id);
        EXPECT_EQ(NodeHandle(5), dataSlot.attachedNode);
        EXPECT_EQ(dataRef, dataSlot.attachedDataReference);
        EXPECT_EQ(textureRes, dataSlot.attachedTexture);
        EXPECT_EQ(sampler, dataSlot.attachedTextureSampler);
    }

    TYPED_TEST(AScene, ReleasesDataSlot)
    {
        const DataSlotHandle handle = this->m_scene.allocateDataSlot({ EDataSlotType_TransformationConsumer, DataSlotId(6), NodeHandle(5), DataInstanceHandle(), ResourceContentHash::Invalid(), TextureSamplerHandle() });
        this->m_scene.releaseDataSlot(handle);

        EXPECT_FALSE(this->m_scene.isDataSlotAllocated(handle));
    }
}
