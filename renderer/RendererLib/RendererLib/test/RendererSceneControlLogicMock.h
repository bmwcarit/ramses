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

        MOCK_METHOD2(setSceneState, void(SceneId, RendererSceneState));
        MOCK_METHOD2(setSceneMapping, void(SceneId, DisplayHandle));
        MOCK_METHOD3(setSceneDisplayBufferAssignment, void(SceneId, OffscreenBufferHandle, int32_t));
        MOCK_CONST_METHOD5(getSceneInfo, void(SceneId, RendererSceneState&, DisplayHandle&, OffscreenBufferHandle&, int32_t&));
    };
}

#endif
