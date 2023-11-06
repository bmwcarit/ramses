//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/DataReferenceLinkManager.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/DataLinkUtils.h"
#include "internal/SceneGraph/SceneUtils/DataInstanceHelper.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    DataReferenceLinkManager::DataReferenceLinkManager(RendererScenes& rendererScenes)
        : LinkManagerBase(rendererScenes)
    {
    }

    void DataReferenceLinkManager::removeSceneLinks(SceneId sceneId)
    {
        SceneLinkVector links;
        getSceneLinks().getLinkedConsumers(sceneId, links);
        for(const auto& link : links)
        {
            assert(link.providerSceneId == sceneId);
            removeDataLink(link.consumerSceneId, link.consumerSlot);
        }

        LinkManagerBase::removeSceneLinks(sceneId);
    }

    bool DataReferenceLinkManager::createDataLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        assert(EDataSlotType::DataProvider == DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_scenes).type);
        assert(EDataSlotType::DataConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type);

        const EDataType providerDataType = DataLinkUtils::GetSlotDataReferenceType(providerSceneId, providerSlotHandle, m_scenes);
        const EDataType consumerDataType = DataLinkUtils::GetSlotDataReferenceType(consumerSceneId, consumerSlotHandle, m_scenes);
        if (providerDataType != consumerDataType)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: data types of provider (Scene:" << providerSceneId << ") and consumer (Scene: " << consumerSceneId << ") do not match!");
            return false;
        }

        return LinkManagerBase::createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle);
    }

    bool DataReferenceLinkManager::removeDataLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle, SceneId* providerSceneIdOut)
    {
        assert(EDataSlotType::DataConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type);

        if (providerSceneIdOut && getSceneLinks().hasLinkedProvider(consumerSceneId, consumerSlotHandle))
            *providerSceneIdOut = getSceneLinks().getLinkedProvider(consumerSceneId, consumerSlotHandle).providerSceneId;

        if (!LinkManagerBase::removeDataLink(consumerSceneId, consumerSlotHandle))
        {
            return false;
        }

        DataReferenceLinkCachedScene& consumerScene = m_scenes.getScene(consumerSceneId);
        const DataInstanceHandle dataRef = consumerScene.getDataSlot(consumerSlotHandle).attachedDataReference;
        consumerScene.restoreFallbackValue(dataRef, DataFieldHandle(0u));

        return true;
    }

    void DataReferenceLinkManager::resolveLinksForConsumerScene(DataReferenceLinkCachedScene& consumerScene) const
    {
        const SceneId consumerSceneId = consumerScene.getSceneId();
        SceneLinkVector links;
        getSceneLinks().getLinkedProviders(consumerSceneId, links);

        for(const auto& link : links)
        {
            assert(link.consumerSceneId == consumerSceneId);
            const DataInstanceHandle consumerDataRef = consumerScene.getDataSlot(link.consumerSlot).attachedDataReference;

            const IScene& providerScene = m_scenes.getScene(link.providerSceneId);
            const DataInstanceHandle providerDataRef = providerScene.getDataSlot(link.providerSlot).attachedDataReference;

            DataInstanceValueVariant value;
            DataInstanceHelper::GetInstanceFieldData(providerScene, providerDataRef, DataFieldHandle(0u), value);
            consumerScene.setValueWithoutUpdatingFallbackValue(consumerDataRef, DataFieldHandle(0u), value);
        }
    }
}
