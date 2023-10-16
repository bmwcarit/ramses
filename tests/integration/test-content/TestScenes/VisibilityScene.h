//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"

namespace ramses::internal
{
    class VisibilityScene : public IntegrationScene
    {
    public:
        VisibilityScene(ramses::Scene& scene, uint32_t state, const glm::vec3& position, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        void setState(uint32_t state);

        enum
        {
            VISIBILITY_ALL_OFF = 0,
            VISIBILITY_OFF_AND_INVISIBLE,
            VISIBILITY_OFF_AND_VISIBLE,
            VISIBILITY_ALL_VISIBLE
        };
    };
}
