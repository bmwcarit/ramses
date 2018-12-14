//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENES_H
#define RAMSES_RENDERERSCENES_H

#include "RendererLib/StagingInfo.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/SceneLinksManager.h"
#include "Utils/ScopedPointer.h"

namespace ramses_internal
{
    class RendererEventCollector;

    struct RendererSceneInfo
    {
        RendererCachedScene* scene;
        StagingInfo* stagingInfo;
    };
    typedef HashMap<SceneId, RendererSceneInfo> RendererSceneInfoMap;

    class RendererScenes : private RendererSceneInfoMap
    {
    public:
        explicit RendererScenes(RendererEventCollector& eventCollector);
        ~RendererScenes();

        RendererCachedScene&       createScene(const SceneInfo& sceneInfo);
        void                       destroyScene(SceneId sceneID);

        Bool                       hasScene(SceneId sceneID) const;
        const RendererCachedScene& getScene(SceneId sceneID) const;
        RendererCachedScene&       getScene(SceneId sceneID);

        const StagingInfo&         getStagingInfo(SceneId sceneID) const;
        StagingInfo&               getStagingInfo(SceneId sceneID);

        const SceneLinksManager&   getSceneLinksManager() const;
        SceneLinksManager&         getSceneLinksManager();

        using RendererSceneInfoMap::begin;
        using RendererSceneInfoMap::end;
        using RendererSceneInfoMap::count;

    private:
        // Scoped ptr due to dependency on *this which cannot be passed in member initialization list
        ScopedPointer<SceneLinksManager> m_sceneLinksManager;
    };
}

#endif
