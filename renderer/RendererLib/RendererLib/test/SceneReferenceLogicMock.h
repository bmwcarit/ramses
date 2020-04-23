//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEREFERENCELOGICMOCK_H
#define RAMSES_SCENEREFERENCELOGICMOCK_H

#include "gmock/gmock.h"
#include "RendererLib/SceneReferenceLogic.h"

namespace ramses_internal
{
    class SceneReferenceLogicMock : public ISceneReferenceLogic
    {
    public:
        SceneReferenceLogicMock();
        virtual ~SceneReferenceLogicMock();

        MOCK_METHOD2(addActions, void(SceneId, const SceneReferenceActionVector&));
        MOCK_METHOD0(update, void());
    };
}

#endif
