//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENELINKSCENE_H
#define RAMSES_SCENELINKSCENE_H

#include "Scene/TransformationCachedScene.h"

namespace ramses_internal
{
    class SceneLinksManager;

    class SceneLinkScene : public TransformationCachedSceneWithExplicitMemory
    {
    public:
        SceneLinkScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo = SceneInfo());

        // From IScene
        virtual DataSlotHandle          allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        virtual void                    releaseDataSlot(DataSlotHandle handle) override;

    protected:
        SceneLinksManager& m_sceneLinksManager;
    };
}

#endif
