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

        virtual void                    preallocateSceneSize(const SceneSizeInformation& sizeInfo) override;

        // From IScene
        virtual NodeHandle              allocateNode(UInt32 childrenCount = 0u, NodeHandle node = NodeHandle::Invalid()) override;
        virtual void                    releaseNode(NodeHandle node) override;

        virtual void                    addChildToNode(NodeHandle parent, NodeHandle child) override;
        virtual void                    removeChildFromNode(NodeHandle parent, NodeHandle child) override;

        virtual TransformHandle         allocateTransform(NodeHandle nodeHandle, TransformHandle handle = TransformHandle::Invalid()) override;
        virtual void                    releaseTransform(TransformHandle transform) override;

        virtual void                    setTranslation(TransformHandle transform, const Vector3& translation) override;
        virtual void                    setRotation(TransformHandle transform, const Vector3& rotation, ERotationConvention convention) override;
        virtual void                    setScaling(TransformHandle transform, const Vector3& scaling) override;

        Matrix44f                       updateMatrixCache(ETransformationMatrixType matrixType, NodeHandle node) const;
        bool                            isMatrixCacheDirty(ETransformationMatrixType matrixType, NodeHandle node) const;

    protected:
        MatrixCacheEntry&           getMatrixCacheEntry(NodeHandle nodeHandle) const;
        bool                        markDirty(NodeHandle node) const;

        const Matrix44f&            findCleanAncestorMatrixAndCollectDirtyNodesOnTheWay(ETransformationMatrixType matrixType, NodeHandle node, NodeHandleVector& dirtyNodes) const;
        void                        computeMatrixForNode(ETransformationMatrixType matrixType, NodeHandle node, Matrix44f& chainMatrix) const;
        void                        setMatrixCache(ETransformationMatrixType matrixType, MatrixCacheEntry& matrixCache, const Matrix44f& matrix) const;

        // A (local) member variable used by propagateDirty(...) and propagateDirtyToConsumers(...).,
        // in order to avoid creating a new Vector each time a method is called.
        mutable NodeHandleVector m_dirtyPropagationTraversalBuffer;

    private:
        void                        updateMatrixCacheForDirtyNodes(ETransformationMatrixType matrixType, Matrix44f& chainMatrix, const NodeHandleVector& dirtyNodes) const;

        void                        computeWorldMatrixForNode(NodeHandle node, Matrix44f& chainMatrix) const;
        void                        computeObjectMatrixForNode(NodeHandle node, Matrix44f& chainMatrix) const;
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
