//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SINGLEAPPEARANCESCENE_H
#define RAMSES_SINGLEAPPEARANCESCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/MeshNode.h"
#include "Collections/Vector.h"


namespace ramses_internal
{
    class Vector4;

    class SingleAppearanceScene : public IntegrationScene
    {
    public:
        SingleAppearanceScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            RED_TRIANGLES = 0,
            GREEN_TRIANGLES,
            BLUE_TRIANGLES,
            CHANGE_APPEARANCE
        };

    private:
        void setAppearanceAndGeometryToAllMeshNodes(ramses::Appearance& appearance, ramses::GeometryBinding& geometry);
        void setTriangleColor(ramses::Appearance& appearance, const ramses::Effect& effect, const Vector4& color);

        std::vector<ramses::MeshNode*> m_meshNodes;
    };
}

#endif
