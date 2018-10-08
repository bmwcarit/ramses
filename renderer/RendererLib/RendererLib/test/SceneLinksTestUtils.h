//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENELINKSTESTUTILS_H
#define RAMSES_SCENELINKSTESTUTILS_H

#include "SceneAPI/IScene.h"
#include "SceneAllocateHelper.h"
#include "RendererLib/SceneLinksManager.h"

namespace ramses_internal {

template <typename T>
const T& GetConcreteLinkManager(const SceneLinksManager& sceneLinksManager)
{
    assert(false);
    return sceneLinksManager.getTransformationLinkManager();
}

template <>
inline const TransformationLinkManager& GetConcreteLinkManager<TransformationLinkManager>(const SceneLinksManager& sceneLinksManager)
{
    return sceneLinksManager.getTransformationLinkManager();
}

template <>
inline const DataReferenceLinkManager& GetConcreteLinkManager<DataReferenceLinkManager>(const SceneLinksManager& sceneLinksManager)
{
    return sceneLinksManager.getDataReferenceLinkManager();
}

template <>
inline const TextureLinkManager& GetConcreteLinkManager<TextureLinkManager>(const SceneLinksManager& sceneLinksManager)
{
    return sceneLinksManager.getTextureLinkManager();
}

template <typename T>
void createDataSlot(SceneAllocateHelper& scene, DataSlotHandle slot, DataSlotId slotId, bool providerType)
{
    scene.allocateDataSlot({ (providerType ? EDataSlotType_TransformationProvider : EDataSlotType_TransformationConsumer), slotId, NodeHandle(12u), DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, slot);
}

template <>
inline void createDataSlot<TransformationLinkManager>(SceneAllocateHelper& scene, DataSlotHandle slot, DataSlotId slotId, bool providerType)
{
    const NodeHandle node = scene.allocateNode();
    scene.allocateTransform(node);
    scene.allocateDataSlot({ (providerType ? EDataSlotType_TransformationProvider : EDataSlotType_TransformationConsumer), slotId, node, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, slot);
}

template <>
inline void createDataSlot<DataReferenceLinkManager>(SceneAllocateHelper& scene, DataSlotHandle slot, DataSlotId slotId, bool providerType)
{
    const DataLayoutHandle layout = scene.allocateDataLayout({ DataFieldInfo(EDataType_Float) });
    const DataInstanceHandle dataRef = scene.allocateDataInstance(layout);

    scene.allocateDataSlot({ (providerType ? EDataSlotType_DataProvider : EDataSlotType_DataConsumer), slotId, NodeHandle(), dataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, slot);
}

template <>
inline void createDataSlot<TextureLinkManager>(SceneAllocateHelper& scene, DataSlotHandle slot, DataSlotId slotId, bool providerType)
{
    if (providerType)
    {
        scene.allocateDataSlot({ EDataSlotType_TextureProvider, slotId, NodeHandle(), DataInstanceHandle::Invalid(), ResourceContentHash(1234u, 0), TextureSamplerHandle() }, slot);
    }
    else
    {
        const TextureSamplerHandle sampler = scene.allocateTextureSampler({ {}, RenderBufferHandle(999) });
        scene.allocateDataSlot({ EDataSlotType_TextureConsumer, slotId, NodeHandle(), DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), sampler }, slot);
    }
}
}
#endif
