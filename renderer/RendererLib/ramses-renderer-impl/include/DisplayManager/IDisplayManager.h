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

namespace ramses
{
    class IRendererEventHandler;
    class IRendererSceneControlEventHandler_legacy;
}

namespace ramses_internal
{
    enum class SceneState
    {
        Unavailable,
        Available,
        Ready,
        Rendered
    };

    class IEventHandler;

    class IDisplayManager
    {
    public:
        virtual ~IDisplayManager() = default;

        virtual bool setSceneState(ramses::sceneId_t sceneId, SceneState state, const char* confirmationText = "") = 0;
        virtual bool setSceneMapping(ramses::sceneId_t sceneId, ramses::displayId_t displayId) = 0;
        virtual bool setSceneDisplayBufferAssignment(ramses::sceneId_t sceneId, ramses::displayBufferId_t displayBuffer, int32_t sceneRenderOrder = 0) = 0;
        virtual bool setDisplayBufferClearColor(ramses::displayBufferId_t displayBuffer, float r, float g, float b, float a) = 0;
        virtual void linkOffscreenBuffer(ramses::displayBufferId_t offscreenBufferId, ramses::sceneId_t consumerSceneId, ramses::dataConsumerId_t consumerDataSlotId) = 0;
        virtual void linkData(ramses::sceneId_t providerSceneId, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerSceneId, ramses::dataConsumerId_t consumerId) = 0;
        virtual void processConfirmationEchoCommand(const char* text) = 0;
        virtual void dispatchAndFlush(IEventHandler* eventHandler = nullptr, ramses::IRendererEventHandler* customRendererEventHandler = nullptr, ramses::IRendererSceneControlEventHandler_legacy* customSceneControlHandler = nullptr) = 0;
    };

    static constexpr int GetNumSceneStates()
    {
        return static_cast<int>(SceneState::Rendered) + 1;
    }

    static inline const char* SceneStateName(SceneState state)
    {
        static const char* DMSceneStateNames[] =
        {
            "UNAVAILABLE",
            "AVAILABLE",
            "READY",
            "RENDERED"
        };
        static_assert(GetNumSceneStates() == sizeof(DMSceneStateNames) / sizeof(DMSceneStateNames[0]), "missing state name");

        return DMSceneStateNames[int(state)];
    }

    class IEventHandler
    {
    public:
        virtual ~IEventHandler() = default;

        virtual void scenePublished(ramses::sceneId_t sceneId) = 0;
        virtual void sceneStateChanged(ramses::sceneId_t sceneId, SceneState state, ramses::displayId_t displaySceneIsMappedTo) = 0;
        virtual void offscreenBufferLinked(ramses::displayBufferId_t offscreenBufferId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId, bool success) = 0;
        virtual void dataLinked(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId, bool success) = 0;
    };
}

#endif
