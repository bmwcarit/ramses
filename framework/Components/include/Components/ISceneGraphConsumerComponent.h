//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ISCENEGRAPHCONSUMERCOMPONENT_H
#define RAMSES_ISCENEGRAPHCONSUMERCOMPONENT_H

#include "Collections/Guid.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/SceneTypes.h"

namespace ramses_internal
{
    class ISceneRendererServiceHandler;
    struct SceneReferenceEvent;

    class ISceneGraphConsumerComponent
    {
    public:
        virtual ~ISceneGraphConsumerComponent() {}

        virtual void setSceneRendererServiceHandler(ISceneRendererServiceHandler* sceneRendererHandler) = 0;
        virtual void subscribeScene(const Guid& to, SceneId sceneId) = 0;
        virtual void unsubscribeScene(const Guid& to, SceneId sceneId) = 0;
        virtual void sendSceneReferenceEvent(const Guid& to, SceneReferenceEvent const& event) = 0;
    };
}

#endif
