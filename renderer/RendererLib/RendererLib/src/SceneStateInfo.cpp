//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/SceneStateInfo.h"

namespace ramses_internal
{
    void SceneStateInfo::addScene(SceneId sceneId, const Guid& clientWhereSceneIsAvailable, EScenePublicationMode mode)
    {
        assert(!m_scenesInfo.contains(sceneId));
        SceneInfo sceneInfo = { clientWhereSceneIsAvailable, ESceneState::Published, mode };
        m_scenesInfo.put(sceneId, sceneInfo);
    }

    Bool SceneStateInfo::hasScene(SceneId sceneId) const
    {
        return m_scenesInfo.contains(sceneId);
    }

    void SceneStateInfo::removeScene(SceneId sceneId)
    {
        assert(m_scenesInfo.contains(sceneId));
        m_scenesInfo.remove(sceneId);
    }

    void SceneStateInfo::setSceneState(SceneId sceneId, ESceneState sceneState)
    {
        SceneInfo* sceneInfo = m_scenesInfo.get(sceneId);
        assert(0 != sceneInfo);
        sceneInfo->state = sceneState;
    }

    ESceneState SceneStateInfo::getSceneState(SceneId sceneId) const
    {
        SceneInfo* sceneInfo = m_scenesInfo.get(sceneId);
        if (!sceneInfo)
            return ESceneState::Unknown;
        return sceneInfo->state;
    }

    EScenePublicationMode SceneStateInfo::getScenePublicationMode(SceneId sceneId) const
    {
        SceneInfo* sceneInfo = m_scenesInfo.get(sceneId);
        if (!sceneInfo)
            return EScenePublicationMode_Unpublished;
        return sceneInfo->publicationMode;
    }

    Guid SceneStateInfo::getSceneClientGuid(SceneId sceneId) const
    {
        SceneInfo* sceneInfo = m_scenesInfo.get(sceneId);
        assert(0 != sceneInfo);
        return sceneInfo->clientWhereSceneIsAvailable;
    }

    void SceneStateInfo::getKnownSceneIds(SceneIdVector& knownIds) const
    {
        assert(knownIds.empty());
        knownIds.reserve(m_scenesInfo.count());
        for(const auto& sceneInfo : m_scenesInfo)
        {
            knownIds.push_back(sceneInfo.key);
        }
    }
}
