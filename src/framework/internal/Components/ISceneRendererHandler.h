//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneId.h"

namespace ramses::internal
{
    class Guid;
    struct SceneUpdate;

    class ISceneRendererHandler
    {
    public:
        virtual ~ISceneRendererHandler() = default;

        virtual void handleInitializeScene(const SceneInfo& sceneInfo, const Guid& providerID) = 0;
        virtual void handleSceneUpdate(const SceneId& sceneId, SceneUpdate&& sceneUpdate, const Guid& providerID) = 0;
        virtual void handleNewSceneAvailable(const SceneInfo& newScene, const Guid& providerID) = 0;
        virtual void handleSceneBecameUnavailable(const SceneId& unavailableScene, const Guid& providerID) = 0;
    };
}
