//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <gmock/gmock.h>
#include "internal/Communication/TransportCommon/ServiceHandlerInterfaces.h"
#include "internal/SceneReferencing/SceneReferenceEvent.h"
#include "internal/Communication/TransportCommon/ServiceHandlerInterfaces.h"
#include "internal/Components/ISceneProviderEventConsumer.h"
#include "TestEqualHelper.h"

namespace ramses::internal
{
    class SceneProviderServiceHandlerMock : public ISceneProviderServiceHandler
    {
    public:
        SceneProviderServiceHandlerMock();
        ~SceneProviderServiceHandlerMock() override;

        MOCK_METHOD(void, handleSubscribeScene, (const SceneId& sceneId, const Guid& consumerID), (override));
        MOCK_METHOD(void, handleUnsubscribeScene, (const SceneId& sceneId, const Guid& consumerID), (override));
        MOCK_METHOD(void, handleRendererEvent, (const SceneId& sceneId, const std::vector<std::byte>& data, const Guid& rendererID), (override));
    };

    class SceneRendererServiceHandlerMock : public ISceneRendererServiceHandler
    {
    public:
        SceneRendererServiceHandlerMock();
        ~SceneRendererServiceHandlerMock() override;

        MOCK_METHOD(void, handleNewScenesAvailable, (const SceneInfoVector& newScenes, const Guid& providerID, EFeatureLevel featureLevel), (override));
        MOCK_METHOD(void, handleScenesBecameUnavailable, (const SceneInfoVector& unavailableScenes, const Guid& providerID), (override));

        MOCK_METHOD(void, handleSceneNotAvailable, (const SceneId& sceneId, const Guid& providerID), (override));

        MOCK_METHOD(void, handleInitializeScene, (const SceneId& sceneId, const Guid& providerID), (override));
        MOCK_METHOD(void, handleSceneUpdate, (const SceneId& sceneId, absl::Span<const std::byte> actionData, const Guid& providerID), (override));
    };
}
