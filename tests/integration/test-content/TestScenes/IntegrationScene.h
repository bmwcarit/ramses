//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "impl/DataTypesImpl.h"

#include <cstdint>
#include <string>

namespace ramses
{
    class RamsesClient;
    class Scene;
    class RenderGroup;
    class RenderPass;
    class Node;
    class MeshNode;
    class Camera;
    class PerspectiveCamera;
    class Effect;
}

namespace ramses::internal
{
    class IntegrationScene
    {
    public:
        IntegrationScene(ramses::Scene& scene, const glm::vec3& cameraPosition, uint32_t vpWidth = DefaultViewportWidth, uint32_t vpHeight = DefaultViewportHeight);
        virtual ~IntegrationScene();
        virtual void dispatchHandler(){};

        static constexpr uint32_t DefaultViewportWidth = 200u;
        static constexpr uint32_t DefaultViewportHeight = 200u;

    protected:
        ramses::Effect* getTestEffect(const std::string& nameOrShaderFile);

        void                   addMeshNodeToDefaultRenderGroup(const ramses::MeshNode& mesh, int32_t orderWithinGroup = 0);
        void                   setCameraToDefaultRenderPass(const ramses::Camera* camera);
        ramses::Node&          getDefaultCameraTranslationNode();
        ramses::PerspectiveCamera& getDefaultCamera();
        ramses::PerspectiveCamera& createCameraWithDefaultParameters();

        ramses::Scene&         m_scene;

    private:
        ramses::RenderGroup&       m_defaultRenderGroup;
        ramses::RenderPass&        m_defaultRenderPass;
        ramses::Node&              m_defaultCameraTranslationNode;
        ramses::PerspectiveCamera& m_defaultCamera;
    };
}
