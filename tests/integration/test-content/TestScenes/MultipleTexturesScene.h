//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "ramses/client/MeshNode.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"


namespace ramses::internal
{
    class MultipleTexturesScene : public IntegrationScene
    {
    public:
        MultipleTexturesScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            THREE_MULTIPLEXED_TEXTURES = 0,
            THREE_MULTIPLEXED_TEXTURES_UBO,
        };

    private:
        Effect* createEffect(uint32_t state);
    };
}
