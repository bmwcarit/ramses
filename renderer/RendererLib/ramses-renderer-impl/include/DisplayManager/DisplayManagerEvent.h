//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYMANAGEREVENT_H
#define RAMSES_DISPLAYMANAGEREVENT_H

#include "DisplayManager/IDisplayManager.h"

namespace ramses_internal
{
    struct DisplayManagerEvent
    {
        enum class Type
        {
            ScenePublished,
            SceneStateChanged,
            OffscreenBufferLinked,
            DataLinked
        };

        Type type = Type::DataLinked;
        ramses::sceneId_t sceneId{ 0 };
        SceneState state = SceneState::Unavailable;
        ramses::displayId_t displaySceneIsMappedTo = ramses::displayId_t::Invalid();
        ramses::displayBufferId_t displayBufferId = ramses::displayBufferId_t::Invalid();

        ramses::sceneId_t providerSceneId{ 0 };
        ramses::sceneId_t consumerSceneId{ 0 };
        ramses::dataProviderId_t providerId{ 0 };
        ramses::dataConsumerId_t consumerId{ 0 };
        bool dataLinked = false;
    };
}

#endif
