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
#include <memory>

namespace ramses_internal
{
    class RendererEventCollector;

    struct RendererSceneInfo
    {
        RendererCachedScene* scene;
        StagingInfo* stagingInfo;
    };

    class RendererScenes
    {
    public:
        explicit RendererScenes(RendererEventCollector& eventCollector);
        ~RendererScenes();

        RendererCachedScene&       createScene(const SceneInfo& sceneInfo);
        void                       destroyScene(SceneId sceneID);

        [[nodiscard]] bool                       hasScene(SceneId sceneID) const;
        [[nodiscard]] const RendererCachedScene& getScene(SceneId sceneID) const;
        RendererCachedScene&       getScene(SceneId sceneID);

        [[nodiscard]] const StagingInfo&         getStagingInfo(SceneId sceneID) const;
        StagingInfo&               getStagingInfo(SceneId sceneID);

        [[nodiscard]] const SceneLinksManager&   getSceneLinksManager() const;
        SceneLinksManager&         getSceneLinksManager();

        [[nodiscard]] HashMap<SceneId, RendererSceneInfo>::ConstIterator begin() const;
        [[nodiscard]] HashMap<SceneId, RendererSceneInfo>::ConstIterator end() const;
        [[nodiscard]] size_t size() const;

    private:
        HashMap<SceneId, RendererSceneInfo> m_rendererSceneInfos;
        // Scoped ptr due to dependency on *this which cannot be passed in member initialization list
        std::unique_ptr<SceneLinksManager> m_sceneLinksManager;
    };
}

#endif
