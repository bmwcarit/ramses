//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/SceneDisplayTracker.h"
#include <numeric>

namespace ramses_internal
{
    void SceneDisplayTracker::setSceneOwnership(SceneId scene, DisplayHandle display)
    {
        assert(display.isValid());
        m_sceneToDisplay[scene] = display;
    }

    DisplayHandle SceneDisplayTracker::getSceneOwnership(SceneId scene) const
    {
        const auto it = m_sceneToDisplay.find(scene);
        return (it != m_sceneToDisplay.cend() ? it->second : DisplayHandle::Invalid());
    }

    void SceneDisplayTracker::unregisterDisplay(DisplayHandle display)
    {
        for (auto it = m_sceneToDisplay.begin(); it != m_sceneToDisplay.end(); )
        {
            if (it->second == display)
                it = m_sceneToDisplay.erase(it);
            else
                ++it;
        }
    }
}
