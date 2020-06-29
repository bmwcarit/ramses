//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererSceneControl_legacyMock.h"

using namespace testing;

namespace ramses
{
    RendererSceneControl_legacyMock::RendererSceneControl_legacyMock()
    {
        ON_CALL(*this, subscribeScene(_)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, unsubscribeScene(_)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, mapScene(_, _)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, unmapScene(_)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, showScene(_)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, hideScene(_)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, assignSceneToDisplayBuffer(_, _, _)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, setDisplayBufferClearColor(_, _, _, _, _, _)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, linkData(_, _, _, _)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, linkOffscreenBufferToSceneData(_, _, _)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, unlinkData(_, _)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, flush()).WillByDefault(Return(StatusOK));
        ON_CALL(*this, dispatchEvents(_)).WillByDefault(Return(StatusOK));
    }

    RendererSceneControl_legacyMock::~RendererSceneControl_legacyMock() = default;
}
