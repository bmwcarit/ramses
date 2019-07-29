//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/TransformationCachedScene.h"
#include "Utils/MemoryPoolExplicit.h"
#include "Utils/MemoryPool.h"

namespace ramses_internal
{
    template <template<typename, typename> class MEMORYPOOL>
    TransformationCachedSceneT<MEMORYPOOL>::TransformationCachedSceneT(const SceneInfo& sceneInfo)
        : SceneT<MEMORYPOOL>(sceneInfo)
    {
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        SceneT<MEMORYPOOL>::preallocateSceneSize(sizeInfo);

        m_nodeToTransformMap.reserve(sizeInfo.transformCount);
        m_matrixCachePool.preallocateSize(sizeInfo.nodeCount);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::removeChildFromNode(NodeHandle parent, NodeHandle child)
    {
        propagateDirty(child);
        SceneT<MEMORYPOOL>::removeChildFromNode(parent, child);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::addChildToNode(NodeHandle parent, NodeHandle child)
    {
        propagateDirty(child);
        SceneT<MEMORYPOOL>::addChildToNode(parent, child);
    }

    template <template<typename, typename> class MEMORYPOOL>
    TransformHandle TransformationCachedSceneT<MEMORYPOOL>::allocateTransform(NodeHandle nodeHandle, TransformHandle handle)
    {
        assert(nodeHandle.isValid());
        const TransformHandle actualHandle = SceneT<MEMORYPOOL>::allocateTransform(nodeHandle, handle);
        m_nodeToTransformMap.put(nodeHandle, actualHandle);
        propagateDirty(nodeHandle);
        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::releaseTransform(TransformHandle transform)
    {
        const NodeHandle nodeHandle = this->getTransformNode(transform);
        assert(nodeHandle.isValid());
        SceneT<MEMORYPOOL>::releaseTransform(transform);
        propagateDirty(nodeHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::setTranslation(TransformHandle transform, const Vector3& translation)
    {
        const NodeHandle nodeTransformIsConnectedTo = this->getTransformNode(transform);
        assert(nodeTransformIsConnectedTo.isValid());
        getMatrixCacheEntry(nodeTransformIsConnectedTo).m_isIdentity = false;
        propagateDirty(nodeTransformIsConnectedTo);
        SceneT<MEMORYPOOL>::setTranslation(transform, translation);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::setRotation(TransformHandle transform, const Vector3& rotation)
    {
        const NodeHandle nodeTransformIsConnectedTo = this->getTransformNode(transform);
        assert(nodeTransformIsConnectedTo.isValid());
        getMatrixCacheEntry(nodeTransformIsConnectedTo).m_isIdentity = false;
        propagateDirty(nodeTransformIsConnectedTo);
        SceneT<MEMORYPOOL>::setRotation(transform, rotation);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::setScaling(TransformHandle transform, const Vector3& scaling)
    {
        const NodeHandle nodeTransformIsConnectedTo = this->getTransformNode(transform);
        assert(nodeTransformIsConnectedTo.isValid());
        getMatrixCacheEntry(nodeTransformIsConnectedTo).m_isIdentity = false;
        propagateDirty(nodeTransformIsConnectedTo);
        SceneT<MEMORYPOOL>::setScaling(transform, scaling);
    }

    template <template<typename, typename> class MEMORYPOOL>
    NodeHandle TransformationCachedSceneT<MEMORYPOOL>::allocateNode(UInt32 childrenCount, NodeHandle node)
    {
        const NodeHandle _node = SceneT<MEMORYPOOL>::allocateNode(childrenCount, node);
        m_matrixCachePool.allocate(_node);
        return _node;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::releaseNode(NodeHandle node)
    {
        m_matrixCachePool.release(node);
        m_nodeToTransformMap.remove(node);
        SceneT<MEMORYPOOL>::releaseNode(node);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::setMatrixCache(ETransformationMatrixType matrixType, MatrixCacheEntry& matrixCache, const Matrix44f& matrix) const
    {
        matrixCache.m_matrix[matrixType] = matrix;
        matrixCache.m_matrixDirty[matrixType] = false;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Matrix44f& TransformationCachedSceneT<MEMORYPOOL>::findCleanAncestorMatrixAndCollectDirtyNodesOnTheWay(ETransformationMatrixType matrixType, NodeHandle node, NodeHandleVector& dirtyNodes) const
    {
        dirtyNodes.clear();
        NodeHandle currentNode = node;
        while (currentNode.isValid())
        {
            const MatrixCacheEntry& cacheEntry = getMatrixCacheEntry(currentNode);
            if (!cacheEntry.m_matrixDirty[matrixType])
            {
                return cacheEntry.m_matrix[matrixType];
            }
            else
            {
                dirtyNodes.push_back(currentNode);
                currentNode = SceneT<MEMORYPOOL>::getParent(currentNode);
            }
        }

        return Matrix44f::Identity;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::updateMatrixCacheForDirtyNodes(ETransformationMatrixType matrixType, Matrix44f& chainMatrix, const NodeHandleVector& dirtyNodes) const
    {
        for (Int32 i = static_cast<Int32>(dirtyNodes.size()) - 1; i >= 0; --i)
        {
            const NodeHandle dirtyNode = dirtyNodes[i];
            MatrixCacheEntry& matrixCache = getMatrixCacheEntry(dirtyNode);
            if (!matrixCache.m_isIdentity)
                computeMatrixForNode(matrixType, dirtyNode, chainMatrix);
            setMatrixCache(matrixType, matrixCache, chainMatrix);
        }
    }

    template <template<typename, typename> class MEMORYPOOL>
    Matrix44f TransformationCachedSceneT<MEMORYPOOL>::updateMatrixCache(ETransformationMatrixType matrixType, NodeHandle node) const
    {
        Matrix44f chainMatrix = findCleanAncestorMatrixAndCollectDirtyNodesOnTheWay(matrixType, node, m_dirtyNodes);
        updateMatrixCacheForDirtyNodes(matrixType, chainMatrix, m_dirtyNodes);

        return chainMatrix;
    }


    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::computeMatrixForNode(ETransformationMatrixType matrixType, NodeHandle node, Matrix44f& chainMatrix) const
    {
        switch (matrixType)
        {
        case ETransformationMatrixType_World:
            computeWorldMatrixForNode(node, chainMatrix);
            break;

        case ETransformationMatrixType_Object:
            computeObjectMatrixForNode(node, chainMatrix);
            break;
        default:
            assert(false);
        }
    }

    template <template<typename, typename> class MEMORYPOOL>
    Bool TransformationCachedSceneT<MEMORYPOOL>::markDirty(NodeHandle node) const
    {
        MatrixCacheEntry& cacheEntry = getMatrixCacheEntry(node);
        const Bool wasDirty = cacheEntry.m_matrixDirty[ETransformationMatrixType_Object] && cacheEntry.m_matrixDirty[ETransformationMatrixType_World];
        cacheEntry.setDirty();
        return wasDirty;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::propagateDirty(NodeHandle startNode) const
    {
        assert(m_dirtyPropagationTraversalBuffer.empty());
        m_dirtyPropagationTraversalBuffer.push_back(startNode);

        while (!m_dirtyPropagationTraversalBuffer.empty())
        {
            const NodeHandle node = m_dirtyPropagationTraversalBuffer.back();
            m_dirtyPropagationTraversalBuffer.pop_back();

            const Bool wasDirty = markDirty(node);

            // If it was already dirty, no need to propagate further
            if (!wasDirty)
            {
                const NodeHandleVector& children = SceneT<MEMORYPOOL>::getNode(node).children;
                m_dirtyPropagationTraversalBuffer.insert(m_dirtyPropagationTraversalBuffer.end(), children.cbegin(), children.cend());
            }
        }
    }

    template <template<typename, typename> class MEMORYPOOL>
    MatrixCacheEntry& TransformationCachedSceneT<MEMORYPOOL>::getMatrixCacheEntry(NodeHandle nodeHandle) const
    {
        assert(m_matrixCachePool.isAllocated(nodeHandle));
        return *m_matrixCachePool.getMemory(nodeHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    Bool ramses_internal::TransformationCachedSceneT<MEMORYPOOL>::isMatrixCacheDirty(ETransformationMatrixType matrixType, NodeHandle node) const
    {
        return getMatrixCacheEntry(node).m_matrixDirty[matrixType];
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::computeWorldMatrixForNode(NodeHandle node, Matrix44f& chainMatrix) const
    {
        const TransformHandle* transformPtr = m_nodeToTransformMap.get(node);
        if (transformPtr != nullptr)
        {
            const TransformHandle transform = *transformPtr;
            const Matrix44f matrix =
                Matrix44f::Translation(SceneT<MEMORYPOOL>::getTranslation(transform)) *
                Matrix44f::Scaling(SceneT<MEMORYPOOL>::getScaling(transform)) *
                Matrix44f::RotationEulerZYX(SceneT<MEMORYPOOL>::getRotation(transform));

            chainMatrix *= matrix;
        }
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::computeObjectMatrixForNode(NodeHandle node, Matrix44f& chainMatrix) const
    {
        const TransformHandle* transformPtr = m_nodeToTransformMap.get(node);
        if (transformPtr != nullptr)
        {
            const TransformHandle transform = *transformPtr;
            const Matrix44f matrix =
                Matrix44f::RotationEulerZYX(SceneT<MEMORYPOOL>::getRotation(transform)).transpose() *
                Matrix44f::Scaling(SceneT<MEMORYPOOL>::getScaling(transform).inverse()) *
                Matrix44f::Translation(-SceneT<MEMORYPOOL>::getTranslation(transform));

            chainMatrix = matrix * chainMatrix;
        }
    }

    template class TransformationCachedSceneT < MemoryPool >;
    template class TransformationCachedSceneT < MemoryPoolExplicit >;
}
