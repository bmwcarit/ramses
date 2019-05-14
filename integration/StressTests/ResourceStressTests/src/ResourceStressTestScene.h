//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCESTRESSTESTSCENE_H
#define RAMSES_RESOURCESTRESSTESTSCENE_H

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/EScenePublicationMode.h"
#include "DynamicQuad_ClientResources.h"
#include "DynamicQuad_OffscreenRenderTarget.h"
#include "DynamicQuad_SceneResources.h"
#include <array>
#include <memory>

namespace ramses
{
    class RenderPass;
    class RenderGroup;
    class RenderBuffer;
    class RenderTarget;
    class Scene;
    class RamsesClient;
    class Appearance;
    class GeometryBinding;
    class OrthographicCamera;
    class Effect;
    class MeshNode;
    class TextureSampler;
}

namespace ramses_internal
{
    using TextureConsumerDataIds = std::vector<ramses::dataConsumerId_t>;

    class ResourceStressTestScene
    {
    public:
        ResourceStressTestScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId, const TextureConsumerDataIds& texConsumerDataIds, const ScreenspaceQuad& screenspaceQuad);
        ~ResourceStressTestScene();

        void recreateResources(bool recreateClientResources = true, bool recreateSceneResources = true, bool recreateSceneRenderTargets = true);
        void flush(ramses::sceneVersionTag_t flushName);

    private:
        static ramses::RenderPass& CreateRenderPass(ramses::Scene& scene, ramses::OrthographicCamera& camera);

        ramses::RamsesClient&           m_client;
        const ramses::sceneId_t         m_sceneId;

        ramses::Scene&                  m_scene;
        ramses::OrthographicCamera&     m_camera;
        ramses::RenderPass&             m_offscreenRenderPass;
        ramses::RenderPass&             m_finalRenderPass;

        DynamicQuad_ClientResources                 m_quadWithClientResources;
        DynamicQuad_SceneResources                  m_quadWithSceneResources;
        DynamicQuad_OffscreenRenderTarget           m_quadWithRenderTarget;
        std::vector<std::unique_ptr<DynamicQuad_ClientResources>>  m_quadsWithTextureConsumerLinks;
    };
}

#endif
