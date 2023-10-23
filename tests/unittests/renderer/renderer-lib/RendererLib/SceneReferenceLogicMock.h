//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/RendererLib/SceneReferenceLogic.h"

namespace ramses::internal
{
    class SceneReferenceLogicMock : public ISceneReferenceLogic
    {
    public:
        SceneReferenceLogicMock();
        ~SceneReferenceLogicMock() override;

        MOCK_METHOD(void, addActions, (SceneId, const SceneReferenceActionVector&), (override));
        MOCK_METHOD(void, update, (), (override));
    };
}
