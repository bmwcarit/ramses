//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include "ramses-text-api/FontRegistry.h"
#include "ramses-text-api/TextCache.h"

#include <thread>

/**
 * @example ramses-example-text-basic/src/main.cpp
 * @brief Minimal text example. Shows the string "Hello World!" on the screen.
 */
int main(int argc, char* argv[])
{
    /// [Basic Text Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // Text is rendered in a screen-space pixel precise coordinate system
    // To achieve a texel-to-pixel ratio of 1 when rendering the text, it has to be
    // rendered with an orthographic camera with planes matching the display size (pixel buffer)
    const uint32_t displayWidth(1280);
    const uint32_t displayHeight(480);

    ramses::RamsesFramework framework{ ramses::RamsesFrameworkConfig{ argc, argv } };
    ramses::RamsesClient& client(*framework.createClient("ExampleTextBasic"));

    ramses::Scene* scene = client.createScene(ramses::sceneId_t(123u));

    // create font registry to hold font memory and text cache to cache text meshes
    ramses::FontRegistry fontRegistry;
    ramses::TextCache textCache(*scene, fontRegistry, 1024u, 1024u);

    framework.connect();

    // Create orthographic camera, so that text pixels will match screen pixels
    ramses::OrthographicCamera* camera = scene->createOrthographicCamera();
    camera->setFrustum(0.0f, static_cast<float>(displayWidth), 0.0f, static_cast<float>(displayHeight), 0.1f, 1.f);
    camera->setViewport(0, 0, displayWidth, displayHeight);

    // create render pass
    ramses::RenderPass* renderPass = scene->createRenderPass();
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // create appearance to be used for text
    ramses::EffectDescription effectDesc;
    effectDesc.setAttributeSemantic("a_position", ramses::EEffectAttributeSemantic::TextPositions);
    effectDesc.setAttributeSemantic("a_texcoord", ramses::EEffectAttributeSemantic::TextTextureCoordinates);
    effectDesc.setUniformSemantic("u_texture", ramses::EEffectUniformSemantic::TextTexture);
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);
    effectDesc.setVertexShaderFromFile("res/ramses-example-text-basic-effect.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-text-basic-effect.frag");
    ramses::Effect* textEffect = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "simpleTextShader");

    // create font instance
    ramses::FontId font = fontRegistry.createFreetype2Font("res/ramses-example-text-basic-Roboto-Bold.ttf");
    const int32_t fontSize = 64;
    ramses::FontInstanceId fontInstance = fontRegistry.createFreetype2FontInstance(font, fontSize);

    // load rasterized glyphs for each character
    const std::u32string string = U"Hello World!";
    ramses::GlyphMetricsVector positionedGlyphs = textCache.getPositionedGlyphs(string, fontInstance);
    ramses::GlyphMetricsVector trackedPositionedGlyphs = textCache.getPositionedGlyphs(string, fontInstance);

    // add some character tracking (unit is em --> relative to fontSize)
    // that reduces the spaces between the individual glyphs
    ramses::TextCache::ApplyTrackingToGlyphs(trackedPositionedGlyphs, -50, fontSize);

    // create RAMSES meshes/texture page to hold the glyphs and text geometry
    const ramses::TextLineId textId = textCache.createTextLine(positionedGlyphs, *textEffect);
    ramses::TextLine* textLine = textCache.getTextLine(textId);

    const ramses::TextLineId trackedTextId = textCache.createTextLine(trackedPositionedGlyphs, *textEffect);
    ramses::TextLine* trackedTextLine = textCache.getTextLine(trackedTextId);

    textLine->meshNode->getAppearance()->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
    textLine->meshNode->getAppearance()->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha);
    trackedTextLine->meshNode->getAppearance()->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
    trackedTextLine->meshNode->getAppearance()->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha);

    // add the text meshes to the render pass to show them
    textLine->meshNode->setTranslation(20.0f, 100.0f, -0.5f);
    renderGroup->addMeshNode(*textLine->meshNode);
    trackedTextLine->meshNode->setTranslation(20.0f, 20.0f, -0.5f);
    renderGroup->addMeshNode(*trackedTextLine->meshNode);

    // apply changes
    scene->flush();
    /// [Basic Text Example]

    scene->publish();
    std::this_thread::sleep_for(std::chrono::seconds(30));

    scene->unpublish();
    framework.disconnect();

    return 0;
}
