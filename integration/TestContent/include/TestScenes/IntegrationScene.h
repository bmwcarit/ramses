//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTEGRATIONSCENE_H
#define RAMSES_INTEGRATIONSCENE_H

#include "Collections/Vector.h"
#include "Collections/String.h"

namespace ramses
{
    class RamsesClient;
    class Scene;
    class RenderGroup;
    class RenderPass;
    class MeshNode;
    class Camera;
    class Effect;
    class TranslateNode;
}

namespace ramses_internal
{
    class Vector3;

    class IntegrationScene
    {
    public:
        IntegrationScene(ramses::RamsesClient& client, ramses::Scene& scene, const Vector3& cameraPosition);
        virtual ~IntegrationScene();

        static const UInt32 DefaultDisplayWidth;
        static const UInt32 DefaultDisplayHeight;

    protected:
        ramses::Effect* getTestEffect(const String& nameOrShaderFile);

        void                   addMeshNodeToDefaultRenderGroup(const ramses::MeshNode& mesh, int32_t orderWithinGroup = 0);
        void                   setCameraToDefaultRenderPass(const ramses::Camera* camera);
        ramses::TranslateNode& getDefaultCameraTranslationNode();
        ramses::Camera&        getDefaultCamera();

        ramses::RamsesClient&  m_client;
        ramses::Scene&         m_scene;

    private:
        ramses::RenderGroup&   m_defaultRenderGroup;
        ramses::RenderPass&    m_defaultRenderPass;
        ramses::TranslateNode& m_defaultCameraTranslationNode;
        ramses::Camera*        m_defaultCamera;
    };
}

#endif
