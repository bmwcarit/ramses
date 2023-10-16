//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceStressTestScene.h"

#include "ramses/client/Scene.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/Camera.h"
#include "ramses/client/Effect.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/RenderTargetDescription.h"

#include "internal/PlatformAbstraction/PlatformMath.h"

namespace ramses::internal
{
    ResourceStressTestScene::ResourceStressTestScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId, const TextureConsumerDataIds& texConsumerDataIds, const ScreenspaceQuad& screenspaceQuad)
        : m_client(client)
        , m_sceneId(sceneId)
        , m_scene(*m_client.createScene(m_sceneId))
        , m_camera(screenspaceQuad.createOrthoCamera(m_scene))
        , m_offscreenRenderPass(CreateRenderPass(m_scene, m_camera))
        , m_finalRenderPass(CreateRenderPass(m_scene, m_camera))
        , m_quadWithClientResources              (m_scene                       , screenspaceQuad.createSubQuad({0.1f, 0.1f, 0.4f, 0.4f}))
        , m_quadWithSceneResources               (m_scene                       , screenspaceQuad.createSubQuad({0.1f, 0.5f, 0.4f, 0.8f}))
        , m_quadWithRenderTarget                 (m_scene, m_offscreenRenderPass, screenspaceQuad.createSubQuad({0.6f, 0.1f, 0.9f, 0.4f}))
        , m_quadsWithTextureConsumerLinks(texConsumerDataIds.size())
    {
        const ScreenspaceQuad quadForAllTextureConsumers = screenspaceQuad.createSubQuad({ 0.6f, 0.5f, 0.9f, 0.8f });
        const std::array subQuadsForTextureConsumers =
        {
            quadForAllTextureConsumers.createSubQuad({ 0.0f, 0.0f, 0.5f, 0.5f }),
            quadForAllTextureConsumers.createSubQuad({ 0.5f, 0.0f, 1.0f, 0.5f }),
            quadForAllTextureConsumers.createSubQuad({ 0.5f, 0.5f, 1.0f, 1.0f }),
            quadForAllTextureConsumers.createSubQuad({ 0.0f, 0.5f, 0.5f, 1.0f })
        };

        for (size_t i = 0; i < texConsumerDataIds.size(); ++i)
        {
            m_quadsWithTextureConsumerLinks[i] = std::make_unique<DynamicQuad_Resources>(m_scene, subQuadsForTextureConsumers[i]);
            m_quadsWithTextureConsumerLinks[i]->createTextureDataConsumer(texConsumerDataIds[i]);
        }

        m_offscreenRenderPass.setRenderOrder(0);
        m_offscreenRenderPass.setClearFlags(ramses::EClearFlag::All);
        m_offscreenRenderPass.setClearColor({0.4f, 0.1f, 0.0f, 1.0f});

        m_finalRenderPass.setClearFlags(ramses::EClearFlag::None);
        m_finalRenderPass.setRenderOrder(1);

        // Linking meshes, render groups, and render passes together
        m_offscreenRenderPass.addRenderGroup(m_quadWithClientResources.getRenderGroup());
        m_quadWithRenderTarget.getRenderGroup().addMeshNode(m_quadWithRenderTarget.getMeshNode());
        m_finalRenderPass.addRenderGroup(m_quadWithClientResources.getRenderGroup());
        m_finalRenderPass.addRenderGroup(m_quadWithSceneResources.getRenderGroup());
        m_finalRenderPass.addRenderGroup(m_quadWithRenderTarget.getRenderGroup());
        for(auto& linkedQuad : m_quadsWithTextureConsumerLinks)
            m_finalRenderPass.addRenderGroup(linkedQuad->getRenderGroup());

        m_scene.flush();
        m_scene.publish(ramses::EScenePublicationMode::LocalOnly);
    }

    ResourceStressTestScene::~ResourceStressTestScene()
    {
        m_quadWithClientResources.markSceneObjectsDestroyed();
        for (auto& linkedQuad : m_quadsWithTextureConsumerLinks)
            linkedQuad->markSceneObjectsDestroyed();
        m_quadWithSceneResources.markSceneObjectsDestroyed();
        m_quadWithRenderTarget.markSceneObjectsDestroyed();

        m_client.destroy(m_scene);
    }

    void ResourceStressTestScene::recreateResources(bool recreateClientResources, bool recreateSceneResources, bool recreateSceneRenderTargets)
    {
        // m_quadWithClientResources_TextureLinked is not recreated, because then texture consumer would be destroyed
        // Although this can be properly handled, it would add too much sync points in the stress tests and slow them down
        // This would contradict the idea that stress tests overload ramses with data and API abuse
        if (recreateClientResources)
            m_quadWithClientResources.recreate();
        if(recreateSceneResources)
            m_quadWithSceneResources.recreate();
        if(recreateSceneRenderTargets)
            m_quadWithRenderTarget.recreate();
    }

    void ResourceStressTestScene::flush(ramses::sceneVersionTag_t flushName)
    {
        m_scene.flush(flushName);
    }

    ramses::Scene& ResourceStressTestScene::getScene()
    {
        return m_scene;
    }

    ramses::RenderPass& ResourceStressTestScene::CreateRenderPass(ramses::Scene& scene, ramses::OrthographicCamera& camera)
    {
        ramses::RenderPass* renderPass = scene.createRenderPass();
        renderPass->setCamera(camera);
        return *renderPass;
    }
}
