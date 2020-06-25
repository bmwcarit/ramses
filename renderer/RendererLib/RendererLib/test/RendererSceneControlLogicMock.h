//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENECONTROLLOGICMOCK_H
#define RAMSES_RENDERERSCENECONTROLLOGICMOCK_H

#include "gmock/gmock.h"
#include "RendererLib/RendererSceneControlLogic.h"

namespace ramses_internal
{
    class RendererSceneControlLogicMock : public IRendererSceneControlLogic
    {
    public:
        RendererSceneControlLogicMock();
        virtual ~RendererSceneControlLogicMock();

        MOCK_METHOD(void, setSceneState, (SceneId, RendererSceneState), (override));
        MOCK_METHOD(void, setSceneMapping, (SceneId, DisplayHandle), (override));
        MOCK_METHOD(void, setSceneDisplayBufferAssignment, (SceneId, OffscreenBufferHandle, int32_t), (override));
        MOCK_METHOD(void, getSceneInfo, (SceneId, RendererSceneState&, DisplayHandle&, OffscreenBufferHandle&, int32_t&), (const, override));
    };
}

#endif
