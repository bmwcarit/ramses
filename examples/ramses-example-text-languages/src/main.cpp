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
* @example ramses-example-text-languages/src/main.cpp
* @brief Text Languages Example
*/

int main(int argc, char* argv[])
{
    // Text is rendered in a screen-space pixel precise coordinate system
    // To achieve a texel-to-pixel ratio of 1 when rendering the text, it has to be
    // rendered with an orthographic camera with planes matching the display size (pixel buffer)
    const uint32_t displayWidth(1280);
    const uint32_t displayHeight(480);

    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient client("ExampleTextDirections", framework);

    ramses::Scene* scene = client.createScene(123u);

    // create font registry to hold font memory and text cache to cache text meshes
    ramses::FontRegistry fontRegistry;
    ramses::TextCache textCache(*scene, fontRegistry, 2048u, 2048u);

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

    // create appearance to be used in text
    ramses::EffectDescription effectDesc;
    effectDesc.setAttributeSemantic("a_position", ramses::EEffectAttributeSemantic_TextPositions);
    effectDesc.setAttributeSemantic("a_texcoord", ramses::EEffectAttributeSemantic_TextTextureCoordinates);
    effectDesc.setUniformSemantic("u_texture", ramses::EEffectUniformSemantic_TextTexture);
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    effectDesc.setVertexShaderFromFile("res/ramses-example-text-languages-effect.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-text-languages-effect.frag");
    const ramses::Effect* textEffect = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "simpleTextShader");

    // create font instances
    const ramses::FontId hebrewFont   = fontRegistry.createFreetype2Font("res/ramses-example-text-languages-Arimo-Regular.ttf");
    const ramses::FontId japaneseFont = fontRegistry.createFreetype2Font("res/ramses-example-text-languages-WenQuanYiMicroHei.ttf");
    const ramses::FontId cyrillicFont = fontRegistry.createFreetype2Font("res/ramses-example-text-languages-Roboto-Regular.ttf");
    const ramses::FontId arabicFont   = fontRegistry.createFreetype2Font("res/ramses-example-text-languages-DroidKufi-Regular.ttf");

    const ramses::FontInstanceId cyrillicFontInst         = fontRegistry.createFreetype2FontInstance(cyrillicFont, 40);
    const ramses::FontInstanceId japaneseFontInst         = fontRegistry.createFreetype2FontInstance(japaneseFont, 40);
    const ramses::FontInstanceId hebrewFontInst           = fontRegistry.createFreetype2FontInstance(hebrewFont, 40);
    const ramses::FontInstanceId arabicFontInst           = fontRegistry.createFreetype2FontInstance(arabicFont, 36);
    const ramses::FontInstanceId arabicFontInst_shaped    = fontRegistry.createFreetype2FontInstanceWithHarfBuzz(arabicFont, 36);

    // Using UTF32 string to store translations of the word "chocolate" in different languages
    const std::u32string cyrillicString = { 0x00000448, 0x0000043e, 0x0000043a, 0x0000043e, 0x0000043b, 0x00000430, 0x00000434 };               // "шоколад"
    const std::u32string japaneseString = { 0x000030e7, 0x000030b3, 0x000030ec, 0x000030fc, 0x000030c8, };                                      // "チョコレート"
    const std::u32string hebrewString   = { 0x000005e9, 0x000005d5, 0x000005e7, 0x000005d5, 0x000005dc, 0x000005d3, };                          // "שוקולד"
    const std::u32string arabicString   = { 0x00000634, 0x00000648, 0x00000643, 0x00000648, 0x00000644, 0x00000627, 0x0000062a, 0x00000629, };  //"شوكولاتة"

    // create RAMSES meshes/texture page to hold the glyphs and text geometry
    const ramses::TextLineId textId1 = textCache.createTextLine(textCache.getPositionedGlyphs(cyrillicString, cyrillicFontInst), *textEffect);
    const ramses::TextLineId textId2 = textCache.createTextLine(textCache.getPositionedGlyphs(japaneseString, japaneseFontInst), *textEffect);
    const ramses::TextLineId textId3 = textCache.createTextLine(textCache.getPositionedGlyphs(hebrewString, hebrewFontInst), *textEffect);
    const ramses::TextLineId textId4 = textCache.createTextLine(textCache.getPositionedGlyphs(arabicString, arabicFontInst), *textEffect);
    const ramses::TextLineId textId5 = textCache.createTextLine(textCache.getPositionedGlyphs(arabicString, arabicFontInst_shaped), *textEffect);

    // position meshes for each text line on screen
    ramses::TextLine* textLine1 = textCache.getTextLine(textId1);
    ramses::TextLine* textLine2 = textCache.getTextLine(textId2);
    ramses::TextLine* textLine3 = textCache.getTextLine(textId3);
    ramses::TextLine* textLine4 = textCache.getTextLine(textId4);
    ramses::TextLine* textLine5 = textCache.getTextLine(textId5);

    for (auto textLine : { textLine1, textLine2, textLine3, textLine4, textLine5 })
    {
        textLine->meshNode->getAppearance()->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
        textLine->meshNode->getAppearance()->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha);
    }

    textLine1->meshNode->setTranslation(20.0f, 220.0f, -0.5f);
    textLine2->meshNode->setTranslation(20.0f, 170.0f, -0.5f);
    textLine3->meshNode->setTranslation(20.0f, 120.0f, -0.5f);
    textLine4->meshNode->setTranslation(20.0f, 70.0f , -0.5f);
    textLine5->meshNode->setTranslation(20.0f, 20.0f , -0.5f);

    renderGroup->addMeshNode(*textLine1->meshNode);
    renderGroup->addMeshNode(*textLine2->meshNode);
    renderGroup->addMeshNode(*textLine3->meshNode);
    renderGroup->addMeshNode(*textLine4->meshNode);
    renderGroup->addMeshNode(*textLine5->meshNode);

    scene->flush();
    scene->publish();
    std::this_thread::sleep_for(std::chrono::seconds(30));

    scene->unpublish();
    framework.disconnect();

    return 0;
}
