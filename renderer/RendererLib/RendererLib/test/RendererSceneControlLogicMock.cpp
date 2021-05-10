//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererSceneControlLogicMock.h"

using namespace testing;

namespace ramses_internal
{
    RendererSceneControlLogicMock::RendererSceneControlLogicMock()
    {
        // by default scene is unknown and logic returns invalid/default parameters
        ON_CALL(*this, getSceneInfo(_, _, _, _)).WillByDefault(Invoke([](auto, auto& targetState, auto& ob, auto& renderOrder)
        {
            targetState = RendererSceneState::Unavailable;
            ob = OffscreenBufferHandle::Invalid();
            renderOrder = 0;
        }));
    }

    RendererSceneControlLogicMock::~RendererSceneControlLogicMock() = default;
}
