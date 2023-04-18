//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/TransformationLinkCachedScene.h"
#include "RendererLib/SceneLinksManager.h"

namespace ramses_internal
{
    TransformationLinkCachedScene::TransformationLinkCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo)
        : SceneLinkScene(sceneLinksManager, sceneInfo)
    {
    }

    void TransformationLinkCachedScene::removeChildFromNode(NodeHandle parent, NodeHandle child)
    {
        propagateDirtyToConsumers(child);
        SceneLinkScene::removeChildFromNode(parent, child);
    }

    void TransformationLinkCachedScene::addChildToNode(NodeHandle parent, NodeHandle child)
    {
        propagateDirtyToConsumers(child);
        SceneLinkScene::addChildToNode(parent, child);
    }

    void TransformationLinkCachedScene::setRotation(TransformHandle transform, const Vector4& rotation, ERotationType rotationType)
    {
        const NodeHandle nodeTransformIsConnectedTo = getTransformNode(transform);
        assert(nodeTransformIsConnectedTo.isValid());
        propagateDirtyToConsumers(nodeTransformIsConnectedTo);
        SceneLinkScene::setRotation(transform, rotation, rotationType);
    }

    void TransformationLinkCachedScene::setScaling(TransformHandle transform, const Vector3& scaling)
    {
        const NodeHandle nodeTransformIsConnectedTo = getTransformNode(transform);
        assert(nodeTransformIsConnectedTo.isValid());
        propagateDirtyToConsumers(nodeTransformIsConnectedTo);
        SceneLinkScene::setScaling(transform, scaling);
    }

    void TransformationLinkCachedScene::setTranslation(TransformHandle transform, const Vector3& translation)
    {
        const NodeHandle nodeTransformIsConnectedTo = getTransformNode(transform);
        assert(nodeTransformIsConnectedTo.isValid());
        propagateDirtyToConsumers(nodeTransformIsConnectedTo);
        SceneLinkScene::setTranslation(transform, translation);
    }

    void TransformationLinkCachedScene::releaseDataSlot(DataSlotHandle handle)
    {
        const DataSlot& dataSlot = getDataSlot(handle);
        if (dataSlot.type == EDataSlotType_TransformationProvider || dataSlot.type == EDataSlotType_TransformationConsumer)
        {
            propagateDirtyToConsumers(dataSlot.attachedNode);
        }

        SceneLinkScene::releaseDataSlot(handle);
    }

    void TransformationLinkCachedScene::propagateDirtyToConsumers(NodeHandle startNode) const
    {
        assert(m_dirtyPropagationTraversalBuffer.empty());
        m_dirtyPropagationTraversalBuffer.push_back(startNode);

        while (!m_dirtyPropagationTraversalBuffer.empty())
        {
            NodeHandle node = m_dirtyPropagationTraversalBuffer.back();
            m_dirtyPropagationTraversalBuffer.pop_back();

            const Bool wasDirty = markDirty(node);
            m_sceneLinksManager.getTransformationLinkManager().propagateTransformationDirtinessToConsumers(getSceneId(), node);

            if (!wasDirty)
            {
                const NodeHandleVector& children = getNode(node).children;
                m_dirtyPropagationTraversalBuffer.insert(m_dirtyPropagationTraversalBuffer.end(), children.cbegin(), children.cend());
            }
        }
    }

    Matrix44f TransformationLinkCachedScene::updateMatrixCacheWithLinks(ETransformationMatrixType matrixType, NodeHandle node) const
    {
        if (!m_sceneLinksManager.getTransformationLinkManager().getDependencyChecker().hasDependencyAsConsumer(getSceneId()))
        {
            // early out, if no links need to be resolved fall back to standard transformation scene
            return SceneLinkScene::updateMatrixCache(matrixType, node);
        }

        Matrix44f chainMatrix = SceneLinkScene::findCleanAncestorMatrixAndCollectDirtyNodesOnTheWay(matrixType, node, m_dirtyNodes);

        // update cache for all transforms for the nodes we collected
        for (Int32 i = static_cast<Int32>(m_dirtyNodes.size()) - 1; i >= 0; --i)
        {
            const NodeHandle nodeToUpdate = m_dirtyNodes[i];
            getMatrixForNode(matrixType, nodeToUpdate, chainMatrix);
            setMatrixCache(matrixType, getMatrixCacheEntry(nodeToUpdate), chainMatrix);
        }

        return chainMatrix;
    }

    void TransformationLinkCachedScene::getMatrixForNode(ETransformationMatrixType matrixType, NodeHandle node, Matrix44f& chainMatrix) const
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

    void TransformationLinkCachedScene::resolveMatrix(ETransformationMatrixType matrixType, NodeHandle node, Matrix44f& chainMatrix) const
    {
        chainMatrix = m_sceneLinksManager.getTransformationLinkManager().getLinkedTransformationFromDataProvider(matrixType, getSceneId(), node);
    }
}
