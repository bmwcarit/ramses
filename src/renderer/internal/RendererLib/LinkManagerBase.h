//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/SceneLinks.h"
#include "internal/RendererLib/SceneDependencyChecker.h"

namespace ramses::internal
{
    class RendererScenes;

    class LinkManagerBase
    {
    public:
        explicit LinkManagerBase(RendererScenes& rendererScenes);

        void                          removeSceneLinks(SceneId sceneId);

        bool                          createDataLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        bool                          removeDataLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);

        [[nodiscard]] const SceneDependencyChecker& getDependencyChecker() const;
        [[nodiscard]] const SceneLinks&             getSceneLinks() const;

    protected:
        RendererScenes& m_scenes;

    private:
        SceneLinks             m_sceneLinks;
        SceneDependencyChecker m_dependencyChecker;
    };
}
