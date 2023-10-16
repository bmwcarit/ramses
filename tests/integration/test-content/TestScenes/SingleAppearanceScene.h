//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
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
    class SingleAppearanceScene : public IntegrationScene
    {
    public:
        SingleAppearanceScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            RED_TRIANGLES = 0,
            GREEN_TRIANGLES,
            BLUE_TRIANGLES,
            CHANGE_APPEARANCE
        };

    private:
        void setAppearanceAndGeometryToAllMeshNodes(ramses::Appearance& appearance, ramses::Geometry& geometry);
        static void SetTriangleColor(ramses::Appearance& appearance, const ramses::Effect& effect, const glm::vec4& color);

        std::vector<ramses::MeshNode*> m_meshNodes;
    };
}
