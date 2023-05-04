//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRANSFORMATIONCACHEDSCENE_H
#define RAMSES_TRANSFORMATIONCACHEDSCENE_H

#include "Scene/Scene.h"
#include "Scene/MatrixCacheEntry.h"
#include "Utils/MemoryPool.h"
#include "Utils/MemoryPoolExplicit.h"
#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    template <template<typename, typename> class MEMORYPOOL>
    class TransformationCachedSceneT;

    using TransformationCachedScene = TransformationCachedSceneT<MemoryPool>;
    using TransformationCachedSceneWithExplicitMemory = TransformationCachedSceneT<MemoryPoolExplicit>;

    template <template<typename, typename> class MEMORYPOOL>
    class TransformationCachedSceneT : public SceneT<MEMORYPOOL>
    {
    public:
        explicit TransformationCachedSceneT(const SceneInfo& sceneInfo = SceneInfo());

        void                    preallocateSceneSize(const SceneSizeInformation& sizeInfo) override;

        // From IScene
        NodeHandle              allocateNode(UInt32 childrenCount = 0u, NodeHandle node = NodeHandle::Invalid()) override;
        void                    releaseNode(NodeHandle node) override;

        void                    addChildToNode(NodeHandle parent, NodeHandle child) override;
        void                    removeChildFromNode(NodeHandle parent, NodeHandle child) override;

        TransformHandle         allocateTransform(NodeHandle nodeHandle, TransformHandle handle = TransformHandle::Invalid()) override;
        void                    releaseTransform(TransformHandle transform) override;

        void                    setTranslation(TransformHandle transform, const glm::vec3& translation) override;
        void                    setRotation(TransformHandle transform, const glm::vec4& rotation, ERotationType rotationType) override;
        void                    setScaling(TransformHandle transform, const glm::vec3& scaling) override;

        glm::mat4                       updateMatrixCache(ETransformationMatrixType matrixType, NodeHandle node) const;
        bool                            isMatrixCacheDirty(ETransformationMatrixType matrixType, NodeHandle node) const;

    protected:
        MatrixCacheEntry&           getMatrixCacheEntry(NodeHandle nodeHandle) const;
        bool                        markDirty(NodeHandle node) const;

        const glm::mat4&            findCleanAncestorMatrixAndCollectDirtyNodesOnTheWay(ETransformationMatrixType matrixType, NodeHandle node, NodeHandleVector& dirtyNodes) const;
        void                        computeMatrixForNode(ETransformationMatrixType matrixType, NodeHandle node, glm::mat4& chainMatrix) const;
        void                        setMatrixCache(ETransformationMatrixType matrixType, MatrixCacheEntry& matrixCache, const glm::mat4& matrix) const;

        // A (local) member variable used by propagateDirty(...) and propagateDirtyToConsumers(...).,
        // in order to avoid creating a new Vector each time a method is called.
        mutable NodeHandleVector m_dirtyPropagationTraversalBuffer;

    private:
        void                        updateMatrixCacheForDirtyNodes(ETransformationMatrixType matrixType, glm::mat4& chainMatrix, const NodeHandleVector& dirtyNodes) const;

        void                        computeWorldMatrixForNode(NodeHandle node, glm::mat4& chainMatrix) const;
        void                        computeObjectMatrixForNode(NodeHandle node, glm::mat4& chainMatrix) const;
        void                        propagateDirty(NodeHandle node) const;

        // Cache
        using MatrixCachePool = MEMORYPOOL<MatrixCacheEntry, NodeHandle>;
        mutable MatrixCachePool m_matrixCachePool;

        HashMap<NodeHandle, TransformHandle> m_nodeToTransformMap;

        // to avoid memory allocations the pool for dirty nodes is member variable
        // even though it is used in the scope of matrix cache update only
        mutable NodeHandleVector m_dirtyNodes;
    };
}

#endif
