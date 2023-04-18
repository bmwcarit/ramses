//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRANSFORMATIONLINKMANAGER_H
#define RAMSES_TRANSFORMATIONLINKMANAGER_H

#include "RendererLib/LinkManagerBase.h"
#include "Scene/ETransformMatrixType.h"
#include "Math3d/Matrix44f.h"

namespace ramses_internal
{
    class RendererScenes;

    class TransformationLinkManager : private LinkManagerBase
    {
    public:
        explicit TransformationLinkManager(RendererScenes& rendererScenes);

        void                      removeSceneLinks(SceneId sceneId);

        Bool                      createDataLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        Bool                      removeDataLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle, SceneId* providerSceneIdOut = nullptr);

        [[nodiscard]] Bool                      nodeHasDataLinkToProvider(SceneId consumerSceneID, NodeHandle consumerNodeHandle) const;

        [[nodiscard]] Matrix44f                 getLinkedTransformationFromDataProvider(ETransformationMatrixType matrixType, SceneId consumerSceneId, NodeHandle consumerNodeHandle) const;
        void                      propagateTransformationDirtinessToConsumers(SceneId providerSceneId, NodeHandle providerNodeHandle) const;

        using LinkManagerBase::getDependencyChecker;
        using LinkManagerBase::getSceneLinks;

    private:
        [[nodiscard]] DataSlotHandle getDataSlotForNode(SceneId sceneId, NodeHandle node) const;

        using NodeToSlotMap = HashMap<NodeHandle, DataSlotHandle>;
        using SceneToNodeSlotMap = HashMap<SceneId, NodeToSlotMap>;
        SceneToNodeSlotMap m_nodesToDataSlots;
    };
}

#endif
