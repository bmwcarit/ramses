//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "SceneAllocateHelper.h"
#include "internal/RendererLib/SceneLinksManager.h"

namespace ramses::internal {

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
        scene.allocateDataSlot({ (providerType ? EDataSlotType::TransformationProvider : EDataSlotType::TransformationConsumer), slotId, NodeHandle(12u), DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, slot);
    }

    template <>
    inline void createDataSlot<TransformationLinkManager>(SceneAllocateHelper& scene, DataSlotHandle slot, DataSlotId slotId, bool providerType)
    {
        const NodeHandle node = scene.allocateNode();
        scene.allocateTransform(node);
        scene.allocateDataSlot({ (providerType ? EDataSlotType::TransformationProvider : EDataSlotType::TransformationConsumer), slotId, node, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, slot);
    }

    template <>
    inline void createDataSlot<DataReferenceLinkManager>(SceneAllocateHelper& scene, DataSlotHandle slot, DataSlotId slotId, bool providerType)
    {
        const DataLayoutHandle layout = scene.allocateDataLayout({ DataFieldInfo(EDataType::Float) }, ResourceContentHash::Invalid());
        const DataInstanceHandle dataRef = scene.allocateDataInstance(layout);

        scene.allocateDataSlot({ (providerType ? EDataSlotType::DataProvider : EDataSlotType::DataConsumer), slotId, NodeHandle(), dataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, slot);
    }

    template <>
    inline void createDataSlot<TextureLinkManager>(SceneAllocateHelper& scene, DataSlotHandle slot, DataSlotId slotId, bool providerType)
    {
        if (providerType)
        {
            scene.allocateDataSlot({ EDataSlotType::TextureProvider, slotId, NodeHandle(), DataInstanceHandle::Invalid(), ResourceContentHash(1234u, 0), TextureSamplerHandle() }, slot);
        }
        else
        {
            const TextureSamplerHandle sampler = scene.allocateTextureSampler({ {}, RenderBufferHandle(999) });
            scene.allocateDataSlot({ EDataSlotType::TextureConsumer, slotId, NodeHandle(), DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), sampler }, slot);
        }
    }
}

