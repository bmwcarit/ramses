//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEREFERENCEOWNERSHIP_H
#define RAMSES_SCENEREFERENCEOWNERSHIP_H

#include "RendererAPI/Types.h"
#include "SceneAPI/SceneId.h"
#include <mutex>
#include <unordered_map>

namespace ramses_internal
{
    class SceneReferenceOwnership
    {
    public:
        void setOwner(SceneId refScene, SceneId ownerScene)
        {
            std::lock_guard<std::mutex> l{ m_lock };
            m_refSceneToOwner[refScene] = ownerScene;
            if (!ownerScene.isValid())
                m_refSceneToOwner.erase(refScene);
        }

        SceneId getSceneOwner(SceneId refScene) const
        {
            std::lock_guard<std::mutex> l{ m_lock };
            const auto it = m_refSceneToOwner.find(refScene);
            return it != m_refSceneToOwner.cend() ? it->second : SceneId::Invalid();
        }

    private:
        mutable std::mutex m_lock;
        std::unordered_map<SceneId, SceneId> m_refSceneToOwner;
    };
}

#endif
