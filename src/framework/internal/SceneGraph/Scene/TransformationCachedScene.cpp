//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/Scene/TransformationCachedScene.h"
#include "internal/Core/Utils/MemoryPoolExplicit.h"
#include "internal/Core/Utils/MemoryPool.h"
#include "internal/Core/Math3d/Rotation.h"
#include "glm/gtx/transform.hpp"

namespace
{
    const auto Identity = glm::identity<glm::mat4>();
}

namespace ramses::internal
{
    template <template<typename, typename> class MEMORYPOOL>
    TransformationCachedSceneT<MEMORYPOOL>::TransformationCachedSceneT(const SceneInfo& sceneInfo)
        : BaseT(sceneInfo)
    {
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        BaseT::preallocateSceneSize(sizeInfo);

        m_nodeToTransformMap.reserve(sizeInfo.transformCount);
        m_matrixCachePool.preallocateSize(sizeInfo.nodeCount);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::removeChildFromNode(NodeHandle parent, NodeHandle child)
    {
        propagateDirty(child);
        BaseT::removeChildFromNode(parent, child);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::addChildToNode(NodeHandle parent, NodeHandle child)
    {
        propagateDirty(child);
        BaseT::addChildToNode(parent, child);
    }

    template <template<typename, typename> class MEMORYPOOL>
    TransformHandle TransformationCachedSceneT<MEMORYPOOL>::allocateTransform(NodeHandle nodeHandle, TransformHandle handle)
    {
        assert(nodeHandle.isValid());
        const TransformHandle actualHandle = BaseT::allocateTransform(nodeHandle, handle);
        m_nodeToTransformMap.put(nodeHandle, actualHandle);
        propagateDirty(nodeHandle);
        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::releaseTransform(TransformHandle transform)
    {
        const NodeHandle nodeHandle = this->getTransformNode(transform);
        assert(nodeHandle.isValid());
        BaseT::releaseTransform(transform);
        propagateDirty(nodeHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::setTranslation(TransformHandle transform, const glm::vec3& translation)
    {
        const NodeHandle nodeTransformIsConnectedTo = this->getTransformNode(transform);
        assert(nodeTransformIsConnectedTo.isValid());
        getMatrixCacheEntry(nodeTransformIsConnectedTo).m_isIdentity = false;
        propagateDirty(nodeTransformIsConnectedTo);
        BaseT::setTranslation(transform, translation);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::setRotation(TransformHandle transform, const glm::vec4& rotation, ERotationType rotationType)
    {
        const NodeHandle nodeTransformIsConnectedTo = this->getTransformNode(transform);
        assert(nodeTransformIsConnectedTo.isValid());
        getMatrixCacheEntry(nodeTransformIsConnectedTo).m_isIdentity = false;
        propagateDirty(nodeTransformIsConnectedTo);
        BaseT::setRotation(transform, rotation, rotationType);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::setScaling(TransformHandle transform, const glm::vec3& scaling)
    {
        const NodeHandle nodeTransformIsConnectedTo = this->getTransformNode(transform);
        assert(nodeTransformIsConnectedTo.isValid());
        getMatrixCacheEntry(nodeTransformIsConnectedTo).m_isIdentity = false;
        propagateDirty(nodeTransformIsConnectedTo);
        BaseT::setScaling(transform, scaling);
    }

    template <template<typename, typename> class MEMORYPOOL>
    NodeHandle TransformationCachedSceneT<MEMORYPOOL>::allocateNode(uint32_t childrenCount, NodeHandle node)
    {
        const NodeHandle _node = BaseT::allocateNode(childrenCount, node);
        m_matrixCachePool.allocate(_node);
        return _node;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::releaseNode(NodeHandle node)
    {
        m_matrixCachePool.release(node);
        m_nodeToTransformMap.remove(node);
        BaseT::releaseNode(node);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::setMatrixCache(ETransformationMatrixType matrixType, MatrixCacheEntry& matrixCache, const glm::mat4& matrix) const
    {
        matrixCache.m_matrix[matrixType] = matrix;
        matrixCache.m_matrixDirty[matrixType] = false;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::mat4& TransformationCachedSceneT<MEMORYPOOL>::findCleanAncestorMatrixAndCollectDirtyNodesOnTheWay(ETransformationMatrixType matrixType, NodeHandle node, NodeHandleVector& dirtyNodes) const
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
            dirtyNodes.push_back(currentNode);
            currentNode = BaseT::getParent(currentNode);
        }

        return Identity;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::updateMatrixCacheForDirtyNodes(ETransformationMatrixType matrixType, glm::mat4& chainMatrix, const NodeHandleVector& dirtyNodes) const
    {
        for (int32_t i = static_cast<int32_t>(dirtyNodes.size()) - 1; i >= 0; --i)
        {
            const NodeHandle dirtyNode = dirtyNodes[i];
            MatrixCacheEntry& matrixCache = getMatrixCacheEntry(dirtyNode);
            if (!matrixCache.m_isIdentity)
                computeMatrixForNode(matrixType, dirtyNode, chainMatrix);
            setMatrixCache(matrixType, matrixCache, chainMatrix);
        }
    }

    template <template<typename, typename> class MEMORYPOOL>
    glm::mat4 TransformationCachedSceneT<MEMORYPOOL>::updateMatrixCache(ETransformationMatrixType matrixType, NodeHandle node) const
    {
        auto chainMatrix = findCleanAncestorMatrixAndCollectDirtyNodesOnTheWay(matrixType, node, m_dirtyNodes);
        updateMatrixCacheForDirtyNodes(matrixType, chainMatrix, m_dirtyNodes);

        return chainMatrix;
    }


    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::computeMatrixForNode(ETransformationMatrixType matrixType, NodeHandle node, glm::mat4& chainMatrix) const
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
    bool TransformationCachedSceneT<MEMORYPOOL>::markDirty(NodeHandle node) const
    {
        MatrixCacheEntry& cacheEntry = getMatrixCacheEntry(node);
        const bool wasDirty = cacheEntry.m_matrixDirty[ETransformationMatrixType_Object] && cacheEntry.m_matrixDirty[ETransformationMatrixType_World];
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

            const bool wasDirty = markDirty(node);

            // If it was already dirty, no need to propagate further
            if (!wasDirty)
            {
                const NodeHandleVector& children = BaseT::getNode(node).children;
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
    bool ramses::internal::TransformationCachedSceneT<MEMORYPOOL>::isMatrixCacheDirty(ETransformationMatrixType matrixType, NodeHandle node) const
    {
        return getMatrixCacheEntry(node).m_matrixDirty[matrixType];
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::computeWorldMatrixForNode(NodeHandle node, glm::mat4& chainMatrix) const
    {
        const TransformHandle* transformHandlePtr = m_nodeToTransformMap.get(node);
        if (transformHandlePtr != nullptr)
        {
            const auto& transform = BaseT::getTransform(*transformHandlePtr);
            const auto matrix =
                glm::translate(transform.translation) *
                Math3d::Rotation(transform.rotation, transform.rotationType) *
                glm::scale(transform.scaling);

            chainMatrix *= matrix;
        }
    }

    template <template<typename, typename> class MEMORYPOOL>
    void TransformationCachedSceneT<MEMORYPOOL>::computeObjectMatrixForNode(NodeHandle node, glm::mat4& chainMatrix) const
    {
        const TransformHandle* transformHandlePtr = m_nodeToTransformMap.get(node);
        if (transformHandlePtr != nullptr)
        {
            const auto& transform = BaseT::getTransform(*transformHandlePtr);
            const auto matrix =
                glm::scale(glm::vec3(1.f) / transform.scaling) *
                glm::transpose(Math3d::Rotation(transform.rotation, transform.rotationType)) *
                glm::translate(-transform.translation);

            chainMatrix = matrix * chainMatrix;
        }
    }

    template class TransformationCachedSceneT < MemoryPool >;
    template class TransformationCachedSceneT < MemoryPoolExplicit >;
}
