//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/IntegrationScene.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "Math3d/Vector3.h"
#include <cassert>

namespace ramses_internal
{
    const UInt32 IntegrationScene::DefaultDisplayWidth(200u);
    const UInt32 IntegrationScene::DefaultDisplayHeight(200u);

    IntegrationScene::IntegrationScene(ramses::RamsesClient& client, ramses::Scene& scene,  const Vector3& cameraPosition)
        : m_client(client)
        , m_scene(scene)
        , m_defaultRenderGroup(*m_scene.createRenderGroup("defaultRenderGroup"))
        , m_defaultRenderPass(*m_scene.createRenderPass("defaultRenderPass"))
        , m_defaultCameraTranslationNode(*m_scene.createNode("defaultCameraTranslation"))
        , m_defaultCamera(m_scene.createRemoteCamera("defaultCamera"))
    {
        m_defaultCameraTranslationNode.setTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);
        m_defaultCamera->setParent(m_defaultCameraTranslationNode);
        m_defaultRenderPass.setCamera(*m_defaultCamera);
        m_defaultRenderPass.addRenderGroup(m_defaultRenderGroup);
        m_defaultRenderPass.setClearFlags(ramses::EClearFlags_None);
    }

    IntegrationScene::~IntegrationScene()
    {
    }

    ramses::Effect* IntegrationScene::getTestEffect(const String& nameOrShaderFile)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile(("res/" + nameOrShaderFile + ".vert").c_str());
        effectDesc.setFragmentShaderFromFile(("res/" + nameOrShaderFile + ".frag").c_str());
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
        effectDesc.setUniformSemantic("viewMatrix", ramses::EEffectUniformSemantic_ViewMatrix);
        effectDesc.setUniformSemantic("modelMatrix", ramses::EEffectUniformSemantic_ModelMatrix);
        effectDesc.setUniformSemantic("cameraPosition", ramses::EEffectUniformSemantic_CameraWorldPosition);
        effectDesc.setUniformSemantic("u_customTexture", ramses::EEffectUniformSemantic_TextTexture);
        effectDesc.setAttributeSemantic("a_customPosition", ramses::EEffectAttributeSemantic_TextPositions);
        effectDesc.setAttributeSemantic("a_customTexCoord", ramses::EEffectAttributeSemantic_TextTextureCoordinates);

        ramses::Effect* effect = m_client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, nameOrShaderFile.c_str());
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

    ramses::Camera& IntegrationScene::getDefaultCamera()
    {
        return *m_defaultCamera;
    }
}
