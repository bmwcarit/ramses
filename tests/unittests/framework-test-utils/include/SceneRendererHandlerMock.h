//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Components/ISceneRendererHandler.h"
#include "internal/PlatformAbstraction/Collections/Guid.h"
#include "internal/Components/SceneUpdate.h"
#include "gmock/gmock.h"

namespace ramses::internal
{
    class SceneRendererHandlerMock : public ISceneRendererHandler
    {
    public:
        SceneRendererHandlerMock();
        ~SceneRendererHandlerMock() override;

        MOCK_METHOD(void, handleInitializeScene, (const SceneInfo& sceneInfo, const Guid& providerID), (override));
        MOCK_METHOD(void, handleNewSceneAvailable, (const SceneInfo& newScene, const Guid& providerID), (override));
        MOCK_METHOD(void, handleSceneBecameUnavailable, (const SceneId& unavailableScene, const Guid& providerID), (override));
        MOCK_METHOD(void, handleSceneUpdate_rvr, (const SceneId& sceneId, const SceneUpdate& sceneUpdate, const Guid& providerID), ());

        void handleSceneUpdate(const SceneId& sceneId, SceneUpdate&& sceneUpdate, const Guid& providerID) override
        {
            handleSceneUpdate_rvr(sceneId, sceneUpdate, providerID);
        }
    };
}
