//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/SceneLinkScene.h"

namespace ramses::internal
{
    class TransformationLinkCachedScene : public SceneLinkScene
    {
    public:
        explicit TransformationLinkCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo = SceneInfo());

        // From IScene
        void                    addChildToNode(NodeHandle parent, NodeHandle child) override;
        void                    removeChildFromNode(NodeHandle parent, NodeHandle child) override;

        void                    setTranslation(TransformHandle transform, const glm::vec3& translation) override;
        void                    setRotation(TransformHandle transform, const glm::vec4& rotation, ERotationType rotationType) override;
        void                    setScaling(TransformHandle transform, const glm::vec3& scaling) override;

        void                    releaseDataSlot(DataSlotHandle handle) override;
        [[nodiscard]] glm::mat4 updateMatrixCacheWithLinks(ETransformationMatrixType matrixType, NodeHandle node) const;
        void      propagateDirtyToConsumers(NodeHandle node) const;

    private:
        void getMatrixForNode(ETransformationMatrixType matrixType, NodeHandle node, glm::mat4& chainMatrix) const;
        void resolveMatrix(ETransformationMatrixType matrixType, NodeHandle node, glm::mat4& chainMatrix) const;

        // to avoid memory allocations the pool for dirty nodes is member variable
        // even though it is used in the scope of matrix cache update only
        mutable NodeHandleVector m_dirtyNodes;
    };
}
