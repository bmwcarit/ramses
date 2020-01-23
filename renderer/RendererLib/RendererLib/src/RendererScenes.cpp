//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererScenes.h"

namespace ramses_internal
{
    RendererScenes::RendererScenes(RendererEventCollector& eventCollector)
    {
        m_sceneLinksManager.reset(new SceneLinksManager(*this, eventCollector));
    }

    RendererScenes::~RendererScenes()
    {
        while (size() != 0u)
        {
            destroyScene(begin()->key);
        }
    }

    RendererCachedScene& RendererScenes::createScene(const SceneInfo& sceneInfo)
    {
        const SceneId sceneID = sceneInfo.sceneID;
        assert(!hasScene(sceneID));

        RendererSceneInfo rendererSceneInfo;
        rendererSceneInfo.scene = new RendererCachedScene(*m_sceneLinksManager, sceneInfo);
        rendererSceneInfo.stagingInfo = new StagingInfo;
        put(sceneID, rendererSceneInfo);

        return *rendererSceneInfo.scene;
    }

    void RendererScenes::destroyScene(SceneId sceneID)
    {
        assert(hasScene(sceneID));

        m_sceneLinksManager->handleSceneRemoved(sceneID);

        RendererSceneInfo& sceneInfo = *get(sceneID);
        delete(sceneInfo.stagingInfo);
        delete(sceneInfo.scene);

        remove(sceneID);
    }

    Bool RendererScenes::hasScene(SceneId sceneID) const
    {
        return contains(sceneID);
    }

    const RendererCachedScene& RendererScenes::getScene(SceneId sceneID) const
    {
        assert(hasScene(sceneID));
        return *get(sceneID)->scene;
    }

    RendererCachedScene& RendererScenes::getScene(SceneId sceneID)
    {
        assert(hasScene(sceneID));
        return *get(sceneID)->scene;
    }

    const StagingInfo& RendererScenes::getStagingInfo(SceneId sceneID) const
    {
        assert(hasScene(sceneID));
        return *get(sceneID)->stagingInfo;
    }

    StagingInfo& RendererScenes::getStagingInfo(SceneId sceneID)
    {
        assert(hasScene(sceneID));
        return *get(sceneID)->stagingInfo;
    }

    const SceneLinksManager& RendererScenes::getSceneLinksManager() const
    {
        return *m_sceneLinksManager;
    }

    SceneLinksManager& RendererScenes::getSceneLinksManager()
    {
        return *m_sceneLinksManager;
    }
}
