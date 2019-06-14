//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayManager/UnsubscribeScene.h"

namespace ramses_display_manager
{
    UnsubscribeScene::UnsubscribeScene(IDisplayManager& displayManager)
        : m_displayManager(displayManager)
    {
        description = "Unsubscribe renderer from a scene it is subscribed to";
        registerKeyword("unsubscribeScene");

        getArgument<0>()
            .registerKeyword("sceneId")
            .setDescription("Scene id");
    }

    bool UnsubscribeScene::execute(uint64_t& sceneId) const
    {
        m_displayManager.setSceneState(ramses::sceneId_t{ sceneId }, ramses_display_manager::SceneState::Unavailable);

        return true;
    }
}
