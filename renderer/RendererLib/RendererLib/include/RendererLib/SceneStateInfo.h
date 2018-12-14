//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENESTATEINFO_H
#define RAMSES_SCENESTATEINFO_H

#include "Collections/HashMap.h"
#include "SceneAPI/SceneId.h"
#include "RendererAPI/Types.h"
#include "Collections/Guid.h"
#include "RendererLib/ESceneState.h"
#include "Scene/EScenePublicationMode.h"

namespace ramses_internal
{

    class SceneStateInfo
    {
    public:
        void addScene(SceneId sceneId, const Guid& clientWhereSceneIsAvailable, EScenePublicationMode mode);
        Bool hasScene(SceneId sceneId) const;
        void removeScene(SceneId sceneId);
        void setSceneState(SceneId sceneId, ESceneState sceneState);
        ESceneState getSceneState(SceneId sceneId) const;
        EScenePublicationMode getScenePublicationMode(SceneId sceneId) const;
        Guid getSceneClientGuid(SceneId sceneId) const;
        void getKnownSceneIds(SceneIdVector& knownIds) const;

    private:
        struct SceneInfo
        {
            Guid clientWhereSceneIsAvailable;
            ESceneState state;
            EScenePublicationMode publicationMode;
        };

        HashMap<SceneId, SceneInfo> m_scenesInfo;
    };
}

#endif
