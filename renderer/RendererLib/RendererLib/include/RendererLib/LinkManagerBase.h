//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LINKMANAGERBASE_H
#define RAMSES_LINKMANAGERBASE_H

#include "RendererLib/SceneLinks.h"
#include "SceneDependencyChecker.h"

namespace ramses_internal
{
    class RendererScenes;

    class LinkManagerBase
    {
    public:
        explicit LinkManagerBase(RendererScenes& rendererScenes);

        void                          removeSceneLinks(SceneId sceneId);

        Bool                          createDataLink(SceneId providerSceneId, DataSlotHandle providerSlotHandle, SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);
        Bool                          removeDataLink(SceneId consumerSceneId, DataSlotHandle consumerSlotHandle);

        const SceneDependencyChecker& getDependencyChecker() const;
        const SceneLinks&             getSceneLinks() const;

    protected:
        RendererScenes& m_scenes;

    private:
        SceneLinks             m_sceneLinks;
        SceneDependencyChecker m_dependencyChecker;
    };
}

#endif
