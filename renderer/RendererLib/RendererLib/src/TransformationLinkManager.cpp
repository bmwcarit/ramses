//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/TransformationLinkManager.h"
#include "RendererLib/TransformationLinkCachedScene.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/DataLinkUtils.h"

namespace ramses_internal
{
    TransformationLinkManager::TransformationLinkManager(RendererScenes& rendererScenes)
        : LinkManagerBase(rendererScenes)
    {
    }

    void TransformationLinkManager::removeSceneLinks(SceneId sceneId)
    {
        SceneLinkVector links;
        getSceneLinks().getLinkedConsumers(sceneId, links);
        for(const auto& link : links)
        {
            assert(link.providerSceneId == sceneId);
            removeDataLink(link.consumerSceneId, link.consumerSlot);
        }

        m_nodesToDataSlots.remove(sceneId);
        LinkManagerBase::removeSceneLinks(sceneId);
    }

    Bool TransformationLinkManager::createDataLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle)
    {
        assert(EDataSlotType_TransformationProvider == DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_scenes).type);
        assert(EDataSlotType_TransformationConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type);

        if (!LinkManagerBase::createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle))
        {
            return false;
        }

        const NodeHandle providerNodeHandle = DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_scenes).attachedNode;
        const NodeHandle consumerNodeHandle = DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).attachedNode;
        assert(providerNodeHandle.isValid());
        assert(consumerNodeHandle.isValid());

        if (!m_nodesToDataSlots.contains(providerSceneId))
        {
            m_nodesToDataSlots.put(providerSceneId, NodeToSlotMap());
        }
        m_nodesToDataSlots.get(providerSceneId)->put(providerNodeHandle, providerSlotHandle);

        if (!m_nodesToDataSlots.contains(consumerSceneId))
        {
            m_nodesToDataSlots.put(consumerSceneId, NodeToSlotMap());
        }
        m_nodesToDataSlots.get(consumerSceneId)->put(consumerNodeHandle, consumerSlotHandle);

        const TransformationLinkCachedScene& consumerScene = m_scenes.getScene(consumerSceneId);
        consumerScene.propagateDirtyToConsumers(consumerNodeHandle);

        return true;
    }

    Bool TransformationLinkManager::removeDataLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle, SceneId* providerSceneIdOut)
    {
        assert(EDataSlotType_TransformationConsumer == DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).type);

        const NodeHandle consumerNodeHandle = DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_scenes).attachedNode;
        if (getSceneLinks().hasLinkedProvider(consumerSceneId, consumerSlotHandle))
        {
            const TransformationLinkCachedScene& consumerScene = m_scenes.getScene(consumerSceneId);
            consumerScene.propagateDirtyToConsumers(consumerNodeHandle);

            if (providerSceneIdOut)
                *providerSceneIdOut = getSceneLinks().getLinkedProvider(consumerSceneId, consumerSlotHandle).providerSceneId;
        }

        if (LinkManagerBase::removeDataLink(consumerSceneId, consumerSlotHandle))
        {
            assert(m_nodesToDataSlots.contains(consumerSceneId));
            m_nodesToDataSlots.get(consumerSceneId)->remove(consumerNodeHandle);

            return true;
        }

        return false;
    }

    Bool TransformationLinkManager::nodeHasDataLinkToProvider(SceneId consumerSceneId, NodeHandle consumerNodeHandle) const
    {
        const DataSlotHandle consumerSlotHandle = getDataSlotForNode(consumerSceneId, consumerNodeHandle);
        return getSceneLinks().hasLinkedProvider(consumerSceneId, consumerSlotHandle);
    }

    Matrix44f TransformationLinkManager::getLinkedTransformationFromDataProvider(ETransformationMatrixType matrixType,
                                                                                 SceneId    consumerSceneId,
                                                                                 NodeHandle consumerNodeHandle) const
    {
        const DataSlotHandle consumerSlotHandle = getDataSlotForNode(consumerSceneId, consumerNodeHandle);
        const SceneLink&     link = getSceneLinks().getLinkedProvider(consumerSceneId, consumerSlotHandle);

        const TransformationLinkCachedScene& providerScene      = m_scenes.getScene(link.providerSceneId);
        const NodeHandle                     providerNodeHandle = providerScene.getDataSlot(link.providerSlot).attachedNode;

        return providerScene.updateMatrixCacheWithLinks(matrixType, providerNodeHandle);
    }

    void TransformationLinkManager::propagateTransformationDirtinessToConsumers(SceneId providerSceneId, NodeHandle providerNodeHandle) const
    {
        const DataSlotHandle providerSlotHandle = getDataSlotForNode(providerSceneId, providerNodeHandle);

        SceneLinkVector links;
        getSceneLinks().getLinkedConsumers(providerSceneId, providerSlotHandle, links);

        for(const auto& link : links)
        {
            assert(link.providerSceneId == providerSceneId);
            assert(link.providerSlot == providerSlotHandle);
            const TransformationLinkCachedScene& consumerScene = m_scenes.getScene(link.consumerSceneId);
            const NodeHandle consumerNodeHandle = consumerScene.getDataSlot(link.consumerSlot).attachedNode;

            consumerScene.propagateDirtyToConsumers(consumerNodeHandle);
        }
    }

    DataSlotHandle TransformationLinkManager::getDataSlotForNode(SceneId sceneId, NodeHandle node) const
    {
        const NodeToSlotMap* nodeToSlotMap = m_nodesToDataSlots.get(sceneId);
        if (nodeToSlotMap == nullptr)
        {
            return DataSlotHandle::Invalid();
        }

        DataSlotHandle* slotEntry = nodeToSlotMap->get(node);
        if (slotEntry == nullptr)
        {
            return DataSlotHandle::Invalid();
        }

        return *slotEntry;
    }
}
