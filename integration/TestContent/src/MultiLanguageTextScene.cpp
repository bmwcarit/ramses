//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#if defined(RAMSES_TEXT_ENABLED)

#include "TestScenes/MultiLanguageTextScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-utils.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/Effect.h"

namespace ramses_internal
{
    MultiLanguageTextScene::MultiLanguageTextScene(ramses::Scene& scene, UInt32 /*state*/, const glm::vec3& cameraPosition)
        : TextScene_Base(scene, cameraPosition)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-test-client-text.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-test-client-text.frag");

        effectDesc.setAttributeSemantic("a_position", ramses::EEffectAttributeSemantic::TextPositions);
        effectDesc.setAttributeSemantic("a_texcoord", ramses::EEffectAttributeSemantic::TextTextureCoordinates);
        effectDesc.setUniformSemantic("u_texture", ramses::EEffectUniformSemantic::TextTexture);
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

        ramses::Effect* effect = scene.createEffect(effectDesc);
        ramses::UniformInput colorInput;
        effect->findUniformInput("u_color", colorInput);

        /// Create fonts:
        const ramses::FontId font = m_fontRegistry.createFreetype2Font("res/ramses-test-client-Roboto-Bold.ttf");
        const ramses::FontInstanceId fontInst = m_fontRegistry.createFreetype2FontInstance(font, 64u);

        const ramses::FontId chineseFont = m_fontRegistry.createFreetype2Font("res/ramses-test-client-WenQuanYiMicroHei.ttf");
        const ramses::FontInstanceId chineseFontInst = m_fontRegistry.createFreetype2FontInstance(chineseFont, 32u);

        const ramses::FontId japaneseFont = m_fontRegistry.createFreetype2Font("res/ramses-test-client-mplus-1p-regular.ttf");
        const ramses::FontInstanceId japaneseFontInst = m_fontRegistry.createFreetype2FontInstance(japaneseFont, 32u);

        const ramses::FontId arabicFont = m_fontRegistry.createFreetype2Font("res/ramses-test-client-DroidKufi-Regular.ttf");
        const ramses::FontInstanceId arabicFontInst = m_fontRegistry.createFreetype2FontInstance(arabicFont, 48u);

        const ramses::FontId hebrewFont = m_fontRegistry.createFreetype2Font("res/ramses-test-client-Arimo-Regular.ttf");
        const ramses::FontInstanceId hebrewFontInst = m_fontRegistry.createFreetype2FontInstance(hebrewFont, 48u);

        const std::u32string demoTextChinese = { 0x5de7, 0x514b, 0x529b, 0x7cd6 }; // U"巧克力糖"
        const std::u32string demoTextJapanese = { 0x30c1, 0x30e7, 0x30b3, 0x30ec, 0x30fc, 0x30c8 }; // U"チョコレート";
        const std::u32string demoTextHebrew = { 0x05e9, 0x05d5, 0x05e7, 0x05d5, 0x05dc, 0x05d3 }; // U"שוקולד";
        const std::u32string demoTextArabicReshaped = { 0xFEB7, 0xFEEE, 0xFEDB, 0xFEEE, 0xFEFB, 0xFE97, 0xFE94 };
        const std::u32string demoTextCyrillic = { 0x0448, 0x043e, 0x043a, 0x043e, 0x043b, 0x0430, 0x0434 }; // U"шоколад";

        const auto chineseTextId = m_textCache.createTextLine(m_textCache.getPositionedGlyphs(demoTextChinese, chineseFontInst), *effect);
        ramses::MeshNode* textChinese = m_textCache.getTextLine(chineseTextId)->meshNode;
        const auto japaneseTextId = m_textCache.createTextLine(m_textCache.getPositionedGlyphs(demoTextJapanese, japaneseFontInst), *effect);
        ramses::MeshNode* textJapanese = m_textCache.getTextLine(japaneseTextId)->meshNode;
        const auto arabicTextId = m_textCache.createTextLine(m_textCache.getPositionedGlyphs(demoTextArabicReshaped, arabicFontInst), *effect);
        ramses::MeshNode* textArabic = m_textCache.getTextLine(arabicTextId)->meshNode;
        const auto hebrewTextId = m_textCache.createTextLine(m_textCache.getPositionedGlyphs(demoTextHebrew, hebrewFontInst), *effect);
        ramses::MeshNode* textHebrew = m_textCache.getTextLine(hebrewTextId)->meshNode;
        const auto cyrillicTextId = m_textCache.createTextLine(m_textCache.getPositionedGlyphs(demoTextCyrillic, fontInst), *effect);//cyrillic reuses latin style
        ramses::MeshNode* textCyrillic = m_textCache.getTextLine(cyrillicTextId)->meshNode;

        for (auto textMesh : { textChinese, textJapanese, textArabic, textHebrew, textCyrillic })
        {
            textMesh->getAppearance()->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
            textMesh->getAppearance()->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha);
        }
        textCyrillic->getAppearance()->setInputValue(colorInput, ramses::vec4f{ 1.0f, 0.0f, 0.0f, 1.0f });
        textChinese->getAppearance()->setInputValue(colorInput, ramses::vec4f{ 0.0f, 0.0f, 1.0f, 1.0f });
        textHebrew->getAppearance()->setInputValue(colorInput, ramses::vec4f{ 0.0f, 1.0f, 1.0f, 1.0f });
        textArabic->getAppearance()->setInputValue(colorInput, ramses::vec4f{ 1.0f, 0.0f, 1.0f, 1.0f });
        textJapanese->getAppearance()->setInputValue(colorInput, ramses::vec4f{ 1.0f, 1.0f, 0.0f, 1.0f });

        ramses::Node* translateChinese = m_scene.createNode();
        ramses::Node* translateJapanese = m_scene.createNode();
        ramses::Node* translateArabic = m_scene.createNode();
        ramses::Node* translateHebrew = m_scene.createNode();
        ramses::Node* translateCyrillic = m_scene.createNode();

        translateJapanese->setTranslation({2, 160, -1});
        translateCyrillic->setTranslation({2, 150, -1});
        translateHebrew->setTranslation({2, 100, -1});
        translateChinese->setTranslation({2, 75, -1});
        translateArabic->setTranslation({2, 40, -1});

        translateJapanese->addChild(*textJapanese);
        translateCyrillic->addChild(*textCyrillic);
        translateHebrew->addChild(*textHebrew);
        translateChinese->addChild(*textChinese);
        translateArabic->addChild(*textArabic);

        addMeshNodeToDefaultRenderGroup(*textJapanese, 2);
        addMeshNodeToDefaultRenderGroup(*textCyrillic, 1);
        addMeshNodeToDefaultRenderGroup(*textHebrew, 1);
        addMeshNodeToDefaultRenderGroup(*textChinese, 1);
        addMeshNodeToDefaultRenderGroup(*textArabic, 1);
    }
}

#endif
