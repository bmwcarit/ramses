//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENERENDERERHANDLERMOCK_H
#define RAMSES_SCENERENDERERHANDLERMOCK_H

#include "Components/ISceneRendererHandler.h"
#include "Collections/Guid.h"
#include "Components/SceneUpdate.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    class SceneRendererHandlerMock : public ISceneRendererHandler
    {
    public:
        SceneRendererHandlerMock();
        virtual ~SceneRendererHandlerMock() override;

        MOCK_METHOD(void, handleInitializeScene, (const SceneInfo& sceneInfo, const Guid& providerID), (override));
        MOCK_METHOD(void, handleNewSceneAvailable, (const SceneInfo& newScene, const Guid& providerID), (override));
        MOCK_METHOD(void, handleSceneBecameUnavailable, (const SceneId& unavailableScene, const Guid& providerID), (override));
        MOCK_METHOD(void, handleSceneUpdate_rvr, (const SceneId& sceneId, const SceneUpdate&& sceneUpdate, const Guid& providerID), ());

        virtual void handleSceneUpdate(const SceneId& sceneId, SceneUpdate&& sceneUpdate, const Guid& providerID) override
        {
            handleSceneUpdate_rvr(sceneId, std::move(sceneUpdate), providerID);
        }
    };
}

#endif
