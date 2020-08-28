//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "OverlayClient.h"

#include "ramses-client.h"
#include "ramses-text-api/FontRegistry.h"
#include "ramses-text-api/TextCache.h"

#include <cassert>

constexpr ramses::sceneId_t OverlayClient::Overlay1SceneId;
constexpr ramses::sceneId_t OverlayClient::Overlay2SceneId;

constexpr ramses::dataConsumerId_t OverlayClient::Overlay1ViewportOffsetId;
constexpr ramses::dataConsumerId_t OverlayClient::Overlay2ViewportOffsetId;

namespace
{
    ramses::Scene* createOverlayScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId, const std::u32string& overlayText, ramses::dataConsumerId_t vpOffsetId)
    {
        ramses::Scene* scene = client.createScene(sceneId);

        // create font registry to hold font memory and text cache to cache text meshes
        ramses::FontRegistry fontRegistry;
        ramses::TextCache textCache(*scene, fontRegistry, 1024u, 1024u);

        // Create orthographic camera, so that text pixels will match screen pixels
        constexpr uint32_t vpWidth{ 480 };
        constexpr uint32_t vpHeight{ 80 };
        ramses::OrthographicCamera* camera = scene->createOrthographicCamera();
        camera->setFrustum(0.0f, static_cast<float>(vpWidth), 0.0f, static_cast<float>(vpHeight), 0.1f, 1.f);
        camera->setViewport(0, 0, vpWidth, vpHeight);

        // create render pass
        ramses::RenderPass* renderPass = scene->createRenderPass();
        renderPass->setClearFlags(ramses::EClearFlags_None);
        renderPass->setCamera(*camera);
        ramses::RenderGroup* renderGroup = scene->createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);

        // create appearance to be used for text
        ramses::EffectDescription effectDesc;
        effectDesc.setAttributeSemantic("a_position", ramses::EEffectAttributeSemantic_TextPositions);
        effectDesc.setAttributeSemantic("a_texcoord", ramses::EEffectAttributeSemantic_TextTextureCoordinates);
        effectDesc.setUniformSemantic("u_texture", ramses::EEffectUniformSemantic_TextTexture);
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
        effectDesc.setVertexShaderFromFile("res/ramses-demo-scene-references-text-effect.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-demo-scene-references-text-effect.frag");
        ramses::Effect* textEffect = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "simpleTextShader");

        // create font instance
        ramses::FontId font = fontRegistry.createFreetype2Font("res/ramses-demo-scene-references-roboto-regular.ttf");
        ramses::FontInstanceId fontInstance = fontRegistry.createFreetype2FontInstance(font, 40);

        // load rasterized glyphs for each character
        const ramses::GlyphMetricsVector positionedGlyphs = textCache.getPositionedGlyphs(overlayText, fontInstance);

        // create RAMSES meshes/texture page to hold the glyphs and text geometry
        const ramses::TextLineId textId = textCache.createTextLine(positionedGlyphs, *textEffect);
        ramses::TextLine* textLine = textCache.getTextLine(textId);

        textLine->meshNode->getAppearance()->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
        textLine->meshNode->getAppearance()->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha);
        textLine->meshNode->getAppearance()->setDepthFunction(ramses::EDepthFunc_Disabled);

        // add the text meshes to the render pass to show them
        textLine->meshNode->setTranslation(20.0f, 20.0f, -0.5f);
        renderGroup->addMeshNode(*textLine->meshNode);

        const auto vpOffsetData = scene->createDataVector2i("vpOffset");
        vpOffsetData->setValue(0, 0);
        camera->bindViewportOffset(*vpOffsetData);
        scene->createDataConsumer(*vpOffsetData, vpOffsetId);

        // apply changes
        scene->flush();
        scene->publish();

        return scene;
    }
}

OverlayClient::OverlayClient(ramses::RamsesFramework& framework)
    : m_framework(framework)
    , m_client(*m_framework.createClient("OverlayClient"))
{
}

OverlayClient::~OverlayClient()
{
    m_framework.destroyClient(m_client);
}

void OverlayClient::createOverlayScenes()
{
    m_overlay1Scene = createOverlayScene(m_client, Overlay1SceneId, U"First Overlay", Overlay1ViewportOffsetId);
    m_overlay2Scene = createOverlayScene(m_client, Overlay2SceneId, U"Second Overlay", Overlay2ViewportOffsetId);
}
