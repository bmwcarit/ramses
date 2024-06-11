//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "Triangle.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

namespace ramses
{
    class MeshNode;
}

namespace ramses::internal
{
    class LogicScene : public IntegrationScene
    {
    public:
        LogicScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        enum
        {
            TRIANGLE_LOGIC = 0,
        };

    private:
        ramses::MeshNode* m_meshNode = nullptr;
        Triangle m_triangle;
    };
}
