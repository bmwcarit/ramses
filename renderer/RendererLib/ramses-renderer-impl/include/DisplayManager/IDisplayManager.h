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
#include <cassert>

namespace ramses_display_manager
{
    enum class SceneState
    {
        Unavailable,
        Available,
        Ready,
        Rendered
    };

    class IDisplayManager
    {
    public:
        virtual ~IDisplayManager() = default;

        virtual bool setSceneState(ramses::sceneId_t sceneId, SceneState state, const char* confirmationText = "") = 0;
        virtual bool setSceneMapping(ramses::sceneId_t sceneId, ramses::displayId_t displayId, int32_t sceneRenderOrder = 0) = 0;
        virtual void linkData(ramses::sceneId_t providerSceneId, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerSceneId, ramses::dataConsumerId_t consumerId) = 0;
        virtual void processConfirmationEchoCommand(const char* text) = 0;
    };

    static constexpr int GetNumSceneStates()
    {
        return static_cast<int>(SceneState::Rendered) + 1;
    }

    static inline const char* SceneStateName(SceneState state)
    {
        static const char* SceneStateNames[] =
        {
            "UNAVAILABLE",
            "AVAILABLE",
            "READY",
            "RENDERED"
        };
        static_assert(GetNumSceneStates() == sizeof(SceneStateNames) / sizeof(SceneStateNames[0]), "missing state name");

        return SceneStateNames[int(state)];
    }

    class IEventHandler
    {
    public:
        virtual ~IEventHandler() = default;

        virtual void sceneStateChanged(ramses::sceneId_t sceneId, SceneState state, ramses::displayId_t displaySceneIsMappedTo) = 0;
    };
}

#endif
