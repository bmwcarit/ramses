//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/TransformationLinkCachedScene.h"
#include "internal/RendererLib/SceneLinksManager.h"

namespace ramses::internal
{
    TransformationLinkCachedScene::TransformationLinkCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo)
        : BaseT(sceneLinksManager, sceneInfo)
    {
    }

    void TransformationLinkCachedScene::removeChildFromNode(NodeHandle parent, NodeHandle child)
    {
        propagateDirtyToConsumers(child);
        BaseT::removeChildFromNode(parent, child);
    }

    void TransformationLinkCachedScene::addChildToNode(NodeHandle parent, NodeHandle child)
    {
        propagateDirtyToConsumers(child);
        BaseT::addChildToNode(parent, child);
    }

    void TransformationLinkCachedScene::setRotation(TransformHandle transform, const glm::vec4& rotation, ERotationType rotationType)
    {
        const NodeHandle nodeTransformIsConnectedTo = getTransformNode(transform);
        assert(nodeTransformIsConnectedTo.isValid());
        propagateDirtyToConsumers(nodeTransformIsConnectedTo);
        BaseT::setRotation(transform, rotation, rotationType);
    }

    void TransformationLinkCachedScene::setScaling(TransformHandle transform, const glm::vec3& scaling)
    {
        const NodeHandle nodeTransformIsConnectedTo = getTransformNode(transform);
        assert(nodeTransformIsConnectedTo.isValid());
        propagateDirtyToConsumers(nodeTransformIsConnectedTo);
        BaseT::setScaling(transform, scaling);
    }

    void TransformationLinkCachedScene::setTranslation(TransformHandle transform, const glm::vec3& translation)
    {
        const NodeHandle nodeTransformIsConnectedTo = getTransformNode(transform);
        assert(nodeTransformIsConnectedTo.isValid());
        propagateDirtyToConsumers(nodeTransformIsConnectedTo);
        BaseT::setTranslation(transform, translation);
    }

    void TransformationLinkCachedScene::releaseDataSlot(DataSlotHandle handle)
    {
        const DataSlot& dataSlot = getDataSlot(handle);
        if (dataSlot.type == EDataSlotType::TransformationProvider || dataSlot.type == EDataSlotType::TransformationConsumer)
        {
            propagateDirtyToConsumers(dataSlot.attachedNode);
        }

        BaseT::releaseDataSlot(handle);
    }

    void TransformationLinkCachedScene::propagateDirtyToConsumers(NodeHandle startNode) const
    {
        assert(m_dirtyPropagationTraversalBuffer.empty());
        m_dirtyPropagationTraversalBuffer.push_back(startNode);

        while (!m_dirtyPropagationTraversalBuffer.empty())
        {
            NodeHandle node = m_dirtyPropagationTraversalBuffer.back();
            m_dirtyPropagationTraversalBuffer.pop_back();

            const bool wasDirty = markDirty(node);
            m_sceneLinksManager.getTransformationLinkManager().propagateTransformationDirtinessToConsumers(getSceneId(), node);

            if (!wasDirty)
            {
                const NodeHandleVector& children = getNode(node).children;
                m_dirtyPropagationTraversalBuffer.insert(m_dirtyPropagationTraversalBuffer.end(), children.cbegin(), children.cend());
            }
        }
    }

    glm::mat4 TransformationLinkCachedScene::updateMatrixCacheWithLinks(ETransformationMatrixType matrixType, NodeHandle node) const
    {
        if (!m_sceneLinksManager.getTransformationLinkManager().getDependencyChecker().hasDependencyAsConsumer(getSceneId()))
        {
            // early out, if no links need to be resolved fall back to standard transformation scene
            return BaseT::updateMatrixCache(matrixType, node);
        }

        glm::mat4 chainMatrix = BaseT::findCleanAncestorMatrixAndCollectDirtyNodesOnTheWay(matrixType, node, m_dirtyNodes);

        // update cache for all transforms for the nodes we collected
        for (int32_t i = static_cast<int32_t>(m_dirtyNodes.size()) - 1; i >= 0; --i)
        {
            const NodeHandle nodeToUpdate = m_dirtyNodes[i];
            getMatrixForNode(matrixType, nodeToUpdate, chainMatrix);
            setMatrixCache(matrixType, getMatrixCacheEntry(nodeToUpdate), chainMatrix);
        }

        return chainMatrix;
    }

    void TransformationLinkCachedScene::getMatrixForNode(ETransformationMatrixType matrixType, NodeHandle node, glm::mat4& chainMatrix) const
    {
        if (m_sceneLinksManager.getTransformationLinkManager().nodeHasDataLinkToProvider(getSceneId(), node))
        {
            resolveMatrix(matrixType, node, chainMatrix);
        }
        else
        {
            computeMatrixForNode(matrixType, node, chainMatrix);
        }
    }

    void TransformationLinkCachedScene::resolveMatrix(ETransformationMatrixType matrixType, NodeHandle node, glm::mat4& chainMatrix) const
    {
        chainMatrix = m_sceneLinksManager.getTransformationLinkManager().getLinkedTransformationFromDataProvider(matrixType, getSceneId(), node);
    }
}
