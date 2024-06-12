//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/IntegrationScene.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/Scene.h"
#include "ramses/client/Effect.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include <cassert>

namespace ramses::internal
{
    IntegrationScene::IntegrationScene(ramses::Scene& scene,  const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : m_scene(scene)
        , m_defaultRenderGroup(*m_scene.createRenderGroup("defaultRenderGroup"))
        , m_defaultRenderPass(*m_scene.createRenderPass("defaultRenderPass"))
        , m_defaultCameraTranslationNode(*m_scene.createNode("defaultCameraTranslation"))
        , m_defaultCamera(*m_scene.createPerspectiveCamera("defaultCamera"))
    {
        m_defaultCamera.setViewport(0, 0, vpWidth, vpHeight);
        m_defaultCamera.setFrustum(19.f, float(vpWidth) / vpHeight, 0.1f, 1500.f);
        m_defaultCameraTranslationNode.setTranslation({cameraPosition.x, cameraPosition.y, cameraPosition.z});
        m_defaultCamera.setParent(m_defaultCameraTranslationNode);
        m_defaultRenderPass.setCamera(m_defaultCamera);
        m_defaultRenderPass.addRenderGroup(m_defaultRenderGroup);
        m_defaultRenderPass.setClearFlags(ramses::EClearFlag::None);
    }

    IntegrationScene::~IntegrationScene() = default;

    ramses::Effect* IntegrationScene::getTestEffect(const std::string& nameOrShaderFile)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile(("res/" + nameOrShaderFile + ".vert").c_str());
        effectDesc.setFragmentShaderFromFile(("res/" + nameOrShaderFile + ".frag").c_str());
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);
        effectDesc.setUniformSemantic("viewMatrix", ramses::EEffectUniformSemantic::ViewMatrix);
        effectDesc.setUniformSemantic("modelMatrix", ramses::EEffectUniformSemantic::ModelMatrix);
        effectDesc.setUniformSemantic("cameraPosition", ramses::EEffectUniformSemantic::CameraWorldPosition);
        effectDesc.setUniformSemantic("u_customTexture", ramses::EEffectUniformSemantic::TextTexture);
        effectDesc.setUniformSemantic("modelUbo", ramses::EEffectUniformSemantic::ModelBlock);
        effectDesc.setUniformSemantic("cameraUbo", ramses::EEffectUniformSemantic::CameraBlock);
        effectDesc.setUniformSemantic("modelCameraUbo", ramses::EEffectUniformSemantic::ModelCameraBlock);
        effectDesc.setAttributeSemantic("a_customPosition", ramses::EEffectAttributeSemantic::TextPositions);
        effectDesc.setAttributeSemantic("a_customTexCoord", ramses::EEffectAttributeSemantic::TextTextureCoordinates);

        ramses::Effect* effect = m_scene.createEffect(effectDesc, nameOrShaderFile.c_str());
        assert(nullptr != effect);
        return effect;
    }

    void IntegrationScene::addMeshNodeToDefaultRenderGroup(const ramses::MeshNode& mesh, int32_t orderWithinGroup)
    {
        if (m_defaultRenderGroup.containsMeshNode(mesh))
        {
            m_defaultRenderGroup.removeMeshNode(mesh);
        }
        m_defaultRenderGroup.addMeshNode(mesh, orderWithinGroup);
    }

    void IntegrationScene::setCameraToDefaultRenderPass(const ramses::Camera* camera)
    {
        m_defaultRenderPass.setCamera(*camera);
    }

    ramses::Node& IntegrationScene::getDefaultCameraTranslationNode()
    {
        return m_defaultCameraTranslationNode;
    }

    ramses::PerspectiveCamera& IntegrationScene::getDefaultCamera()
    {
        return m_defaultCamera;
    }

    ramses::PerspectiveCamera& IntegrationScene::createCameraWithDefaultParameters()
    {
        auto camera = m_scene.createPerspectiveCamera();
        camera->setViewport(0, 0, m_defaultCamera.getViewportWidth(), m_defaultCamera.getViewportHeight());
        camera->setFrustum(m_defaultCamera.getVerticalFieldOfView(), m_defaultCamera.getAspectRatio(), m_defaultCamera.getNearPlane(), m_defaultCamera.getFarPlane());
        return *camera;
    }

}
