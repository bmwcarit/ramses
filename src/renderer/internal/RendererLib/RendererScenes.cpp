//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RendererScenes.h"

namespace ramses::internal
{
    RendererScenes::RendererScenes(RendererEventCollector& eventCollector)
    {
        m_sceneLinksManager = std::make_unique<SceneLinksManager>(*this, eventCollector);
    }

    RendererScenes::~RendererScenes()
    {
        while (m_rendererSceneInfos.size() != 0u)
        {
            destroyScene(m_rendererSceneInfos.begin()->key);
        }
    }

    RendererCachedScene& RendererScenes::createScene(const SceneInfo& sceneInfo)
    {
        const SceneId sceneID = sceneInfo.sceneID;
        assert(!hasScene(sceneID));

        RendererSceneInfo rendererSceneInfo;
        rendererSceneInfo.scene = new RendererCachedScene(*m_sceneLinksManager, sceneInfo);
        rendererSceneInfo.stagingInfo = new StagingInfo;
        m_rendererSceneInfos.put(sceneID, rendererSceneInfo);

        return *rendererSceneInfo.scene;
    }

    void RendererScenes::destroyScene(SceneId sceneID)
    {
        assert(hasScene(sceneID));

        m_sceneLinksManager->handleSceneRemoved(sceneID);

        RendererSceneInfo& sceneInfo = *m_rendererSceneInfos.get(sceneID);
        delete(sceneInfo.stagingInfo);
        delete(sceneInfo.scene);

        m_rendererSceneInfos.remove(sceneID);
    }

    bool RendererScenes::hasScene(SceneId sceneID) const
    {
        return m_rendererSceneInfos.contains(sceneID);
    }

    const RendererCachedScene& RendererScenes::getScene(SceneId sceneID) const
    {
        assert(hasScene(sceneID));
        return *m_rendererSceneInfos.get(sceneID)->scene;
    }

    RendererCachedScene& RendererScenes::getScene(SceneId sceneID)
    {
        assert(hasScene(sceneID));
        return *m_rendererSceneInfos.get(sceneID)->scene;
    }

    const StagingInfo& RendererScenes::getStagingInfo(SceneId sceneID) const
    {
        assert(hasScene(sceneID));
        return *m_rendererSceneInfos.get(sceneID)->stagingInfo;
    }

    StagingInfo& RendererScenes::getStagingInfo(SceneId sceneID)
    {
        assert(hasScene(sceneID));
        return *m_rendererSceneInfos.get(sceneID)->stagingInfo;
    }

    const SceneLinksManager& RendererScenes::getSceneLinksManager() const
    {
        return *m_sceneLinksManager;
    }

    SceneLinksManager& RendererScenes::getSceneLinksManager()
    {
        return *m_sceneLinksManager;
    }

    HashMap<SceneId, RendererSceneInfo>::ConstIterator RendererScenes::begin() const
    {
        return m_rendererSceneInfos.begin();
    }

    HashMap<SceneId, RendererSceneInfo>::ConstIterator RendererScenes::end() const
    {
        return m_rendererSceneInfos.end();
    }

    size_t RendererScenes::size() const
    {
        return m_rendererSceneInfos.size();
    }
}
