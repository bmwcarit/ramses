//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "Triangle.h"

namespace ramses
{
    class OrthographicCamera;
    class RenderBuffer;
}

namespace ramses::internal
{
    class RenderPassOnceScene : public IntegrationScene
    {
    public:
        RenderPassOnceScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        void setState(uint32_t state);

        enum
        {
            INITIAL_RENDER_ONCE = 0,
            CHANGE_CLEAR_COLOR,
            RETRIGGER_PASS
        };

    private:
        void initInputRenderPass();
        void initFinalRenderPass();

        ramses::OrthographicCamera& m_camera;
        ramses::RenderPass& m_renderPass;
        const ramses::RenderBuffer& m_renderBuffer;
    };
}
