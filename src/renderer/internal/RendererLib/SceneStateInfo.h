//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/RendererLib/Types.h"
#include "internal/RendererLib/Enums/ESceneState.h"
#include "internal/SceneGraph/Scene/EScenePublicationMode.h"
#include <optional>

namespace ramses::internal
{

    class SceneStateInfo
    {
    public:
        void addScene(SceneId sceneId, EScenePublicationMode mode);
        [[nodiscard]] bool hasScene(SceneId sceneId) const;
        void removeScene(SceneId sceneId);
        void setSceneState(SceneId sceneId, ESceneState sceneState);
        [[nodiscard]] ESceneState getSceneState(SceneId sceneId) const;
        [[nodiscard]] std::optional<EScenePublicationMode> getScenePublicationMode(SceneId sceneId) const;
        void getKnownSceneIds(SceneIdVector& knownIds) const;

    private:
        struct SceneInfo
        {
            ESceneState state;
            EScenePublicationMode publicationMode;
        };

        HashMap<SceneId, SceneInfo> m_scenesInfo;
    };
}
