//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#if defined(RAMSES_TEXT_ENABLED)

#include "TestScenes/TextScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "ramses-utils.h"

namespace ramses_internal
{
    TextScene::TextScene(ramses::Scene& scene, UInt32 state, const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : TextScene_Base(scene, cameraPosition, vpWidth, vpHeight)
    {
        const ramses::FontId font = m_fontRegistry.createFreetype2Font("res/ramses-test-client-Roboto-Bold.ttf");
        const ramses::FontId chineseFont = m_fontRegistry.createFreetype2Font("res/ramses-test-client-WenQuanYiMicroHei.ttf");
        const ramses::FontId lightFont = m_fontRegistry.createFreetype2Font("res/ramses-test-client-Roboto-Light.ttf");
        const ramses::FontId arabicFont = m_fontRegistry.createFreetype2Font("res/ramses-test-client-DroidKufi-Regular.ttf");

        m_font = m_fontRegistry.createFreetype2FontInstance(font, 60u);
        m_fontSmall = m_fontRegistry.createFreetype2FontInstance(font, 40u);
        m_chineseFont = m_fontRegistry.createFreetype2FontInstance(chineseFont, 32u);
        m_lightFont = m_fontRegistry.createFreetype2FontInstance(lightFont, 26u);
        m_lightAutoHintFont = m_fontRegistry.createFreetype2FontInstance(lightFont, 26u, true);
        m_lightAutoHintAndReshapeFont = m_fontRegistry.createFreetype2FontInstanceWithHarfBuzz(lightFont, 26u, true);
        m_shapingArabicFont = m_fontRegistry.createFreetype2FontInstanceWithHarfBuzz(arabicFont, 26u);
        m_shapingArabicAutoHintFont = m_fontRegistry.createFreetype2FontInstanceWithHarfBuzz(arabicFont, 26u, true);

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile("res/ramses-test-client-text.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-test-client-text.frag");

        effectDesc.setAttributeSemantic("a_position", ramses::EEffectAttributeSemantic::TextPositions);
        effectDesc.setAttributeSemantic("a_texcoord", ramses::EEffectAttributeSemantic::TextTextureCoordinates);
        effectDesc.setUniformSemantic("u_texture", ramses::EEffectUniformSemantic::TextTexture);
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

        ramses::Effect* textEffect = m_scene.createEffect(effectDesc);
        ramses::UniformInput colorInput;
        textEffect->findUniformInput("u_color", colorInput);

        /// Create text lines
        //const std::u32string str = U"ÄÖÜß";
        std::u32string str{ 196, 214, 220, 223 };
        auto glyphMetrics = m_textCache.getPositionedGlyphs(str, m_font);
        m_textUTF = m_textCache.createTextLine(glyphMetrics, *textEffect);
        m_meshUTF = m_textCache.getTextLine(m_textUTF)->meshNode;

        glyphMetrics = m_textCache.getPositionedGlyphs(U"Text", m_font);
        m_textASCII = m_textCache.createTextLine(glyphMetrics, *textEffect);
        m_meshASCII = m_textCache.getTextLine(m_textASCII)->meshNode;

        glyphMetrics = m_textCache.getPositionedGlyphs(U"12345", m_fontSmall);
        m_textDigits = m_textCache.createTextLine(glyphMetrics, *textEffect);
        m_meshDigits = m_textCache.getTextLine(m_textDigits)->meshNode;

        //str = U"我們在天上的父，";
        str = { 25105, 20497, 22312, 22825, 19978, 30340, 29238, 65292 };
        glyphMetrics = m_textCache.getPositionedGlyphs(str, m_chineseFont);
        m_textChinese = m_textCache.createTextLine(glyphMetrics, *textEffect);
        m_meshChinese = m_textCache.getTextLine(m_textChinese)->meshNode;

        glyphMetrics = m_textCache.getPositionedGlyphs(U"NO au7ohint1ng", m_lightFont);
        m_textLight = m_textCache.createTextLine(glyphMetrics, *textEffect);
        m_meshLight = m_textCache.getTextLine(m_textLight)->meshNode;

        glyphMetrics = m_textCache.getPositionedGlyphs(U"au7ohint1ng ON", m_lightAutoHintFont);
        m_textLightAutoHinting = m_textCache.createTextLine(glyphMetrics, *textEffect);
        m_meshLightAutoHinting = m_textCache.getTextLine(m_textLightAutoHinting)->meshNode;

        //str = U"الصحة";
        str = { 0x627, 0x644, 0x635, 0x62d, 0x629 };
        glyphMetrics = m_textCache.getPositionedGlyphs(str, m_shapingArabicFont);
        m_textShaping = m_textCache.createTextLine(glyphMetrics, *textEffect);
        m_meshShaping = m_textCache.getTextLine(m_textShaping)->meshNode;

        //str = U"الصحة (autohint)";
        str = { 0x627, 0x644, 0x635, 0x62d, 0x629, 0x20, 0x028, 0x061, 0x075, 0x074, 0x06f, 0x068, 0x069, 0x06e, 0x074, 0x029 };
        const ramses::FontInstanceOffsets fontOffsets{ { m_shapingArabicAutoHintFont, 0u },{ m_lightAutoHintAndReshapeFont, 5u } };
        glyphMetrics = m_textCache.getPositionedGlyphs(str, fontOffsets);
        m_textShapingAutoHint = m_textCache.createTextLine(glyphMetrics, *textEffect);
        m_meshShapingAutoHint = m_textCache.getTextLine(m_textShapingAutoHint)->meshNode;

        //str = U"るョン(X6)耀世!";
        str = { 0x308b, 0x30e7, 0x30f3, 0x0028, 0x0058, 0x0036, 0x0029, 0x8000, 0x4e16, 0x0021 };
        const ramses::FontInstanceId smallFont = m_fontRegistry.createFreetype2FontInstance(font, 26u);
        const ramses::FontInstanceId smallChineseFont = m_fontRegistry.createFreetype2FontInstance(chineseFont, 26u);
        const ramses::FontInstanceOffsets fontOffsets2{ { smallChineseFont, 0u },{ smallFont, 3u },{ smallChineseFont, 7u },{ smallFont, 9u } };
        glyphMetrics = m_textCache.getPositionedGlyphs(str, fontOffsets2);
        m_textFontCascade = m_textCache.createTextLine(glyphMetrics, *textEffect);
        m_meshFontCascade = m_textCache.getTextLine(m_textFontCascade)->meshNode;

        // add vertical offset for chinese font
        for (auto& glyph : glyphMetrics)
        {
            if (glyph.key.fontInstanceId == smallChineseFont)
                glyph.posY -= 40;
        }
        m_textFontCascadeWithVerticalOffset = m_textCache.createTextLine(glyphMetrics, *textEffect);
        m_meshFontCascadeWithVerticalOffset = m_textCache.getTextLine(m_textFontCascadeWithVerticalOffset)->meshNode;

        for (auto textMesh : { m_meshUTF, m_meshASCII, m_meshDigits, m_meshChinese, m_meshLight, m_meshLightAutoHinting, m_meshShaping, m_meshShapingAutoHint, m_meshFontCascade, m_meshFontCascadeWithVerticalOffset })
        {
            textMesh->getAppearance()->setInputValue(colorInput, ramses::vec4f{ 1.0f, 0.0f, 0.0f, 1.0f });
            textMesh->getAppearance()->setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
            textMesh->getAppearance()->setBlendingFactors(ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::OneMinusSrcAlpha, ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::OneMinusSrcAlpha);
        }
        for (auto textMesh : { m_meshChinese, m_meshFontCascade, m_meshFontCascadeWithVerticalOffset })
            textMesh->getAppearance()->setInputValue(colorInput, ramses::vec4f{ 0.0f, 0.0f, 1.0f, 1.0f });

        ramses::Node* translateUTF = m_scene.createNode();
        ramses::Node* translateASCII = m_scene.createNode();
        ramses::Node* translateDigits = m_scene.createNode();
        ramses::Node* translateChinese = m_scene.createNode();
        ramses::Node* translateLight = m_scene.createNode();
        ramses::Node* translateLightAutoHinting = m_scene.createNode();
        ramses::Node* translateFontCascade = m_scene.createNode();
        ramses::Node* translateShaping = m_scene.createNode();
        ramses::Node* translateShapingAutoHint = m_scene.createNode();

        translateUTF->setTranslation({2, 10, -1});
        translateASCII->setTranslation({2, 80, -1});
        translateDigits->setTranslation({2, 160, -1});
        translateChinese->setTranslation({2, 75, -1});
        translateLight->setTranslation({2, 120, -1});
        translateLightAutoHinting->setTranslation({2, 90, -1});
        translateFontCascade->setTranslation({2, 110, -1});
        translateShaping->setTranslation({2, 10, -1});
        translateShapingAutoHint->setTranslation({2, 80, -1});

        m_meshUTF->setParent(*translateUTF);
        m_meshASCII->setParent(*translateASCII);
        m_meshDigits->setParent(*translateDigits);
        m_meshChinese->setParent(*translateChinese);
        m_meshLight->setParent(*translateLight);
        m_meshLightAutoHinting->setParent(*translateLightAutoHinting);
        m_meshFontCascade->setParent(*translateFontCascade);
        m_meshFontCascadeWithVerticalOffset->setParent(*translateFontCascade);
        m_meshShaping->setParent(*translateShaping);
        m_meshShapingAutoHint->setParent(*translateShapingAutoHint);

        setState(state);
    }

    void TextScene::setState(UInt32 state)
    {
        switch (state)
        {
        case EState_INITIAL:
        case EState_INITIAL_128_BY_64_VIEWPORT:
            addMeshNodeToDefaultRenderGroup(*m_meshUTF, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshASCII, 2);
            addMeshNodeToDefaultRenderGroup(*m_meshDigits, 3);
            addMeshNodeToDefaultRenderGroup(*m_meshChinese, 0);
            break;

        case EState_DELETED_TEXTS:
        {
            addMeshNodeToDefaultRenderGroup(*m_meshUTF, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshASCII, 2);
            addMeshNodeToDefaultRenderGroup(*m_meshDigits, 3);
            addMeshNodeToDefaultRenderGroup(*m_meshChinese, 0);

            m_textCache.deleteTextLine(m_textUTF);
            m_textCache.deleteTextLine(m_textUTF);
            m_textCache.deleteTextLine(m_textDigits);
            m_textCache.deleteTextLine(m_textASCII);
            break;
        }

        case EState_FORCE_AUTO_HINTING:
        {
            addMeshNodeToDefaultRenderGroup(*m_meshLight, 0);
            addMeshNodeToDefaultRenderGroup(*m_meshLightAutoHinting, 0);
        }
        break;

        case EState_FONT_CASCADE:
        {
            addMeshNodeToDefaultRenderGroup(*m_meshFontCascade, 0);
        }
        break;

        case EState_FONT_CASCADE_WITH_VERTICAL_OFFSET:
        {
            addMeshNodeToDefaultRenderGroup(*m_meshFontCascadeWithVerticalOffset, 0);
        }
        break;

        case EState_SHAPING:
        {
            addMeshNodeToDefaultRenderGroup(*m_meshShaping, 0);
            addMeshNodeToDefaultRenderGroup(*m_meshShapingAutoHint, 0);
            break;
        }
        case EState_SMOKE_TEST:
            m_textOrthoCamera->setViewport(0, 0, 480, 480);
            const auto scaling = ramses::vec3f(0.5f);
            m_meshUTF->setScaling(scaling);
            m_meshASCII->setScaling(scaling);
            m_meshDigits->setScaling(scaling);
            m_meshChinese->setScaling(scaling);
            m_meshLight->setScaling(scaling);
            m_meshLightAutoHinting->setScaling(scaling);
            m_meshFontCascade->setScaling(scaling);
            m_meshFontCascadeWithVerticalOffset->setScaling(scaling);
            m_meshShaping->setScaling(scaling);
            m_meshShapingAutoHint->setScaling(scaling);

            m_meshUTF->translate({0.f, 110.f, 0.f});
            addMeshNodeToDefaultRenderGroup(*m_meshUTF, 0);

            m_meshASCII->translate({0.f, 95.f, 0.f});
            addMeshNodeToDefaultRenderGroup(*m_meshASCII, 0);

            m_meshDigits->translate({0.f, -10.f, 0.f});
            addMeshNodeToDefaultRenderGroup(*m_meshDigits, 0);

            m_meshChinese->translate({0.f, 0.f, 0.f});
            addMeshNodeToDefaultRenderGroup(*m_meshChinese, 0);

            m_meshLight->translate({0.f, -15.f, 0.f});
            addMeshNodeToDefaultRenderGroup(*m_meshLight, 0);

            m_meshLightAutoHinting->translate({0.f, 0.f, 0.f});
            addMeshNodeToDefaultRenderGroup(*m_meshLightAutoHinting, 0);

            m_meshFontCascade->translate({0.f, -50.f, 0.f});
            addMeshNodeToDefaultRenderGroup(*m_meshFontCascade, 0);

            m_meshFontCascadeWithVerticalOffset->translate({0.f, -85.f, 0.f});
            addMeshNodeToDefaultRenderGroup(*m_meshFontCascadeWithVerticalOffset, 0);

            m_meshShaping->translate({0.f, 38.f, 0.f});
            addMeshNodeToDefaultRenderGroup(*m_meshShaping, 0);

            m_meshShapingAutoHint->translate({0.f, -43.f, 0.f});
            addMeshNodeToDefaultRenderGroup(*m_meshShapingAutoHint, 0);
        }

        if (EState_INITIAL_128_BY_64_VIEWPORT == state)
        {
            addMeshNodeToDefaultRenderGroup(*m_meshUTF, 1);
            addMeshNodeToDefaultRenderGroup(*m_meshASCII, 2);
            addMeshNodeToDefaultRenderGroup(*m_meshDigits, 3);
            addMeshNodeToDefaultRenderGroup(*m_meshChinese, 0);

            m_textOrthoCamera->setViewport(0, 0, 128, 64);
        }
    }
}

#endif
