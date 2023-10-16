//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/TransformationCachedScene.h"

namespace ramses::internal
{
    class SceneLinksManager;

    class SceneLinkScene : public TransformationCachedSceneWithExplicitMemory
    {
    public:
        explicit SceneLinkScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo = SceneInfo());

        // From IScene
        DataSlotHandle          allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle) override;
        void                    releaseDataSlot(DataSlotHandle handle) override;

    protected:
        SceneLinksManager& m_sceneLinksManager;
    };
}
