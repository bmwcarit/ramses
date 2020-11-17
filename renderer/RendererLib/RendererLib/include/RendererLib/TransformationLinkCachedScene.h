//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRANSFORMATIONLINKCACHEDSCENE_H
#define RAMSES_TRANSFORMATIONLINKCACHEDSCENE_H

#include "RendererLib/SceneLinkScene.h"

namespace ramses_internal
{
    class TransformationLinkCachedScene : public SceneLinkScene
    {
    public:
        explicit TransformationLinkCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo = SceneInfo());

        // From IScene
        virtual void                    addChildToNode(NodeHandle parent, NodeHandle child) override;
        virtual void                    removeChildFromNode(NodeHandle parent, NodeHandle child) override;

        virtual void                    setTranslation(TransformHandle transform, const Vector3& translation) override;
        virtual void                    setRotation(TransformHandle transform, const Vector3& rotation, ERotationConvention convention) override;
        virtual void                    setScaling(TransformHandle transform, const Vector3& scaling) override;

        virtual void                    releaseDataSlot(DataSlotHandle handle) override;
        Matrix44f updateMatrixCacheWithLinks(ETransformationMatrixType matrixType, NodeHandle node) const;
        void      propagateDirtyToConsumers(NodeHandle node) const;

    private:
        void getMatrixForNode(ETransformationMatrixType matrixType, NodeHandle node, Matrix44f& chainMatrix) const;
        void resolveMatrix(ETransformationMatrixType matrixType, NodeHandle node, Matrix44f& chainMatrix) const;

        // to avoid memory allocations the pool for dirty nodes is member variable
        // even though it is used in the scope of matrix cache update only
        mutable NodeHandleVector m_dirtyNodes;
    };
}

#endif
