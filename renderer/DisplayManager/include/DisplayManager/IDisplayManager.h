//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDISPLAYMANAGER_H
#define RAMSES_IDISPLAYMANAGER_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses_display_manager
{
    class IDisplayManager
    {
    public:
        virtual ~IDisplayManager()
        {
        }

        virtual void showSceneOnDisplay(ramses::sceneId_t sceneId, ramses::displayId_t displayId, int32_t sceneRenderOrder, const char* confirmationText) = 0;
        virtual void unsubscribeScene (ramses::sceneId_t sceneId) = 0;
        virtual void unmapScene(ramses::sceneId_t sceneId) = 0;
        virtual void hideScene(ramses::sceneId_t sceneId) = 0;
        virtual void linkData(ramses::sceneId_t providerSceneId, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerSceneId, ramses::dataConsumerId_t consumerId) = 0;
        virtual void processConfirmationEchoCommand(const char* text) = 0;
    };
}

#endif
