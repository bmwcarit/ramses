//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/Guid.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/SceneTypes.h"

namespace ramses::internal
{
    class ISceneRendererHandler;
    struct SceneReferenceEvent;
    struct ResourceAvailabilityEvent;

    class ISceneGraphConsumerComponent
    {
    public:
        virtual ~ISceneGraphConsumerComponent() = default;

        virtual void setSceneRendererHandler(ISceneRendererHandler* sceneRendererHandler) = 0;
        virtual void subscribeScene(const Guid& to, SceneId sceneId) = 0;
        virtual void unsubscribeScene(const Guid& to, SceneId sceneId) = 0;
        virtual void sendSceneReferenceEvent(const Guid& to, SceneReferenceEvent const& event) = 0;
        virtual void sendResourceAvailabilityEvent(const Guid& to, ResourceAvailabilityEvent const& event) = 0;
    };
}
