//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceStressTestScene.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/RenderTargetDescription.h"

#include "PlatformAbstraction/PlatformMath.h"

namespace ramses_internal
{
    ResourceStressTestScene::ResourceStressTestScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId, const TextureConsumerDataIds& texConsumerDataIds, const ScreenspaceQuad& screenspaceQuad)
        : m_client(client)
        , m_sceneId(sceneId)
        , m_scene(*m_client.createScene(m_sceneId))
        , m_camera(screenspaceQuad.createOrthoCamera(m_scene))
        , m_offscreenRenderPass(CreateRenderPass(m_scene, m_camera))
        , m_finalRenderPass(CreateRenderPass(m_scene, m_camera))
        , m_quadWithClientResources              (m_client, m_scene                       , screenspaceQuad.createSubQuad({0.1f, 0.1f, 0.4f, 0.4f}))
        , m_quadWithSceneResources               (m_client, m_scene                       , screenspaceQuad.createSubQuad({0.1f, 0.5f, 0.4f, 0.8f}))
        , m_quadWithRenderTarget                 (m_client, m_scene, m_offscreenRenderPass, screenspaceQuad.createSubQuad({0.6f, 0.1f, 0.9f, 0.4f}))
        , m_quadsWithTextureConsumerLinks(texConsumerDataIds.size())
    {
        const ScreenspaceQuad quadForAllTextureConsumers = screenspaceQuad.createSubQuad({ 0.6f, 0.5f, 0.9f, 0.8f });
        const ScreenspaceQuad subQuadsForTextureConsumers[4] =
        {
            quadForAllTextureConsumers.createSubQuad({ 0.0f, 0.0f, 0.5f, 0.5f }),
            quadForAllTextureConsumers.createSubQuad({ 0.5f, 0.0f, 1.0f, 0.5f }),
            quadForAllTextureConsumers.createSubQuad({ 0.5f, 0.5f, 1.0f, 1.0f }),
            quadForAllTextureConsumers.createSubQuad({ 0.0f, 0.5f, 0.5f, 1.0f })
        };

        for (size_t i = 0; i < texConsumerDataIds.size(); ++i)
        {
            m_quadsWithTextureConsumerLinks[i].reset(new DynamicQuad_ClientResources(m_client, m_scene, subQuadsForTextureConsumers[i]));
            m_quadsWithTextureConsumerLinks[i]->createTextureDataConsumer(texConsumerDataIds[i]);
        }

        m_offscreenRenderPass.setRenderOrder(0);
        m_offscreenRenderPass.setClearFlags(ramses::EClearFlags_All);
        m_offscreenRenderPass.setClearColor(0.4f, 0.1f, 0.0f, 1.0f);

        m_finalRenderPass.setClearFlags(ramses::EClearFlags_None);
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
        m_scene.publish(ramses::EScenePublicationMode_LocalOnly);
    }

    ResourceStressTestScene::~ResourceStressTestScene()
    {
        m_client.destroy(m_scene);

        m_quadWithClientResources.markSceneObjectsDestroyed();
        for(auto& linkedQuad : m_quadsWithTextureConsumerLinks)
            linkedQuad->markSceneObjectsDestroyed();
        m_quadWithSceneResources.markSceneObjectsDestroyed();
        m_quadWithRenderTarget.markSceneObjectsDestroyed();
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

    ramses::RenderPass& ResourceStressTestScene::CreateRenderPass(ramses::Scene& scene, ramses::OrthographicCamera& camera)
    {
        ramses::RenderPass* renderPass = scene.createRenderPass();
        renderPass->setCamera(camera);
        return *renderPass;
    }
}
