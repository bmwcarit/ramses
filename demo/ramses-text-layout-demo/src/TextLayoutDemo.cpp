//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TextLayoutDemo.h"
#include "ramses-text-api/LayoutUtils.h"
#include "ramses-text-api/GlyphMetrics.h"
#include <string.h>
#include <thread>
#include <numeric>
#include <algorithm>
#include <assert.h>

namespace
{
    const uint16_t QuadIndices[] = { 0, 1, 2, 2, 1, 3 };
    const float QuadVertices[] = { 0.0f, 0.0f, 0.f, 1.0f, 0.0f, 0.f, 0.0f, 1.0f, 0.f, 1.0f, 1.0f, 0.f };
}

TextLayoutDemo::TextLayoutDemo(int argc, char* argv[])
    : m_framework(argc, argv)
    , m_client("ExampleFontMetrics", m_framework)
    , m_scene(*m_client.createScene(123u))
    , m_textCache(m_scene, m_fontRegistry, 1024u, 1024u)
    , m_textEffect(createTextEffect())
    , m_colorQuadEffect(createColorQuadEffect())
    , m_renderPass(*m_scene.createRenderPass())
    , m_renderGroup(*m_scene.createRenderGroup())
    , m_quadIndices(*m_client.createConstUInt16Array(6, QuadIndices))
    , m_quadVertices(*m_client.createConstVector3fArray(4, QuadVertices))
    , m_windowWidth(1280)
    , m_windowHeight(480)
{
    m_colorQuadEffect.findAttributeInput("a_position", m_colorQuadEffectPositionsInput);
    m_colorQuadEffect.findUniformInput("u_boxSize", m_colorQuadEffectBoxSizeInput);
    m_colorQuadEffect.findUniformInput("u_color", m_colorQuadEffectColorInput);

    m_renderPass.setClearFlags(ramses::EClearFlags_None);
    m_renderPass.setCamera(*createOrthographicCamera());
    m_renderPass.addRenderGroup(m_renderGroup);

    const auto fontLatin = m_fontRegistry.createFreetype2Font("res/ramses-text-layout-demo-Roboto-Regular.ttf");
    const auto fontChinese = m_fontRegistry.createFreetype2Font("res/ramses-text-layout-demo-WenQuanYiMicroHei.ttf");
    const auto fontInstLatin24 = m_fontRegistry.createFreetype2FontInstance(fontLatin, 24u);
    const auto fontInstLatin48 = m_fontRegistry.createFreetype2FontInstance(fontLatin, 48u);
    const auto fontInstChinese24 = m_fontRegistry.createFreetype2FontInstance(fontChinese, 24u);
    const auto fontInstChinese48 = m_fontRegistry.createFreetype2FontInstance(fontChinese, 48u);

    const float boxY = 440.0f;
    const float boxWidth = 400.0f;
    const float boxHeight = 400.0f;
    const float leftBoxX = 50.0f;
    const float rightBoxX = 550.0f;

    std::u32string layoutedText = U"This text will be aligned to fit in a box, and also uses multiple fonts: ";
    const auto chineseTextOffset = layoutedText.size();
    layoutedText.append({ 0x6211, 0x5011, 0x5728, 0x5929, 0x4e0a, 0x7684, 0x7236 });  // U"我們在天上的父";
    const auto restOffset = layoutedText.size();
    layoutedText.append(U".");
    const ramses::FontInstanceOffsets fonts24{ { fontInstLatin24, 0u }, { fontInstChinese24, chineseTextOffset }, { fontInstLatin24, restOffset } };
    const ramses::FontInstanceOffsets fonts48{ { fontInstLatin48, 0u }, { fontInstChinese48, chineseTextOffset }, { fontInstLatin48, restOffset } };

    layoutTextInABox(layoutedText, fonts24, leftBoxX, boxY, boxWidth, boxHeight);
    layoutTextInABox(layoutedText, fonts48, rightBoxX, boxY, boxWidth, boxHeight);

    m_scene.flush();

    m_framework.connect();
    m_scene.publish();
    std::this_thread::sleep_for(std::chrono::seconds(500));
}

TextLayoutDemo::~TextLayoutDemo()
{
    m_scene.unpublish();
    m_client.destroy(m_scene);
    m_framework.disconnect();
}

void TextLayoutDemo::addColoredBox(ramses::Node& parent, float x, float y, float width, float height, float r, float g, float b, int32_t renderOrder)
{
    ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(m_colorQuadEffect, "quad geometry");
    geometry->setIndices(m_quadIndices);
    geometry->setInputBuffer(m_colorQuadEffectPositionsInput, m_quadVertices);

    ramses::Appearance* quadAppearance = m_scene.createAppearance(m_colorQuadEffect, "quad appearance");
    quadAppearance->setDepthWrite(ramses::EDepthWrite_Disabled);//text must be able to overwrite existing quad pixels
    quadAppearance->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
    quadAppearance->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);

    quadAppearance->setInputValueVector4f(m_colorQuadEffectColorInput, r, g, b, 0.5f);
    quadAppearance->setInputValueVector4f(m_colorQuadEffectBoxSizeInput, x, y, width, height);

    ramses::MeshNode* meshNode = m_scene.createMeshNode("quad mesh node");
    meshNode->setAppearance(*quadAppearance);
    meshNode->setGeometryBinding(*geometry);

    parent.addChild(*meshNode);
    m_renderGroup.addMeshNode(*meshNode, renderOrder);
}

void TextLayoutDemo::layoutTextInABox(const std::u32string& string, const ramses::FontInstanceOffsets& fonts, float boxX, float boxY, float boxWidth, float boxHeight)
{
    const int ascender = std::accumulate(fonts.cbegin(), fonts.cend(), -1000, [this](int val, const ramses::FontInstanceOffset& o) { return std::max(val, m_fontRegistry.getFontInstance(o.fontInstance)->getAscender()); });
    const int descender = std::accumulate(fonts.cbegin(), fonts.cend(), 1000, [this](int val, const ramses::FontInstanceOffset& o) { return std::min(val, m_fontRegistry.getFontInstance(o.fontInstance)->getDescender()); });
    const int fontHeight = std::accumulate(fonts.cbegin(), fonts.cend(), -1000, [this](int val, const ramses::FontInstanceOffset& o) { return std::max(val, m_fontRegistry.getFontInstance(o.fontInstance)->getHeight()); });

    ramses::Node& boxTopLeftOrigin = *m_scene.createNode();
    boxTopLeftOrigin.setTranslation(boxX, boxY, 0.0f);

    addColoredBox(boxTopLeftOrigin, 0.0f, -boxHeight, boxWidth, boxHeight, 0.2f, 0.2f, 0.2f, 1);

    int lineVerticalOffset = -ascender;

    const auto glyphs = m_textCache.getPositionedGlyphs(string, fonts);
    auto itLineBegin = glyphs.cbegin();
    while (itLineBegin != glyphs.cend())
    {
        auto isSpace = [](ramses::GlyphMetricsVector::const_iterator it)
        {
            constexpr uint32_t SpaceCharcode = 4u;
            return it->key.identifier.getValue() == SpaceCharcode;
        };

        // remove spaces from beginning
        while (itLineBegin != glyphs.cend() && isSpace(itLineBegin))
            ++itLineBegin;

        auto itLineEnd = ramses::LayoutUtils::FindFittingSubstring(itLineBegin, glyphs.cend(), static_cast<uint32_t>(boxWidth));

        // check if we had to cut the line
        if (itLineEnd != glyphs.cend())
        {
            // check if we cut word in the middle (line ends with non-space char and next char is also non-space)
            if (!isSpace(itLineEnd - 1) && !isSpace(itLineEnd))
            {
                // find last fitting whole word
                auto newLineEnd = itLineEnd;
                while (newLineEnd > itLineBegin && !isSpace(newLineEnd))
                    --newLineEnd;

                // set new line end if there is any word fitting, otherwise keep hard cut
                if (newLineEnd > itLineBegin)
                    itLineEnd = newLineEnd;
            }

            // remove spaces from end
            while (itLineEnd - 1 > itLineBegin && isSpace(itLineEnd - 1))
                --itLineEnd;
        }

        if (itLineBegin == itLineEnd)
            break;

        const ramses::GlyphMetricsVector lineGlyphs{ itLineBegin, itLineEnd };
        const auto lineId = m_textCache.createTextLine(lineGlyphs, m_textEffect);
        const auto lineData = m_textCache.getTextLine(lineId);
        assert(lineData != nullptr);
        const auto lineMesh = lineData->meshNode;
        assert(lineMesh != nullptr);

        ramses::Node& lineOffset = *m_scene.createNode();
        lineOffset.setTranslation(0.f, float(lineVerticalOffset), -0.5f);

        lineMesh->getAppearance()->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
        lineMesh->getAppearance()->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);

        const auto bbox = ramses::LayoutUtils::GetBoundingBoxForString(lineGlyphs.cbegin(), lineGlyphs.cend());
        const float highlightBoxOffset = static_cast<float>(bbox.offsetX);
        const float highlightBoxWidth = static_cast<float>(bbox.width);
        /// Add ascender/descender highlight boxes
        addColoredBox(lineOffset, highlightBoxOffset, 0.f, highlightBoxWidth, float(ascender), 0.0, 0.0f, 0.5, 2);
        addColoredBox(lineOffset, highlightBoxOffset, float(descender), highlightBoxWidth, float(-descender), 0.0, 0.5f, 0.0, 2);

        boxTopLeftOrigin.addChild(lineOffset);
        lineOffset.addChild(*lineMesh);
        m_renderGroup.addMeshNode(*lineMesh, 3);

        lineVerticalOffset -= fontHeight;

        itLineBegin = itLineEnd;
    }
}

ramses::Effect& TextLayoutDemo::createTextEffect()
{
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-layout-demo-red-text.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-layout-demo-red-text.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    effectDesc.setAttributeSemantic("a_position", ramses::EEffectAttributeSemantic_TextPositions);
    effectDesc.setAttributeSemantic("a_texcoord", ramses::EEffectAttributeSemantic_TextTextureCoordinates);
    effectDesc.setUniformSemantic("u_texture", ramses::EEffectUniformSemantic_TextTexture);

    return *m_client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "text effect");
}

ramses::Effect& TextLayoutDemo::createColorQuadEffect()
{
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-layout-demo-colored-quad.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-layout-demo-colored-quad.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    return *m_client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "quad effect");
}

ramses::Camera* TextLayoutDemo::createOrthographicCamera()
{
    /// Create orthographic camera, so that text pixels will match screen pixels
    ramses::OrthographicCamera* camera = m_scene.createOrthographicCamera();
    camera->setFrustum(0.0f, static_cast<float>(m_windowWidth), 0.0f, static_cast<float>(m_windowHeight), 0.1f, 1.f);
    camera->setViewport(0, 0, m_windowWidth, m_windowHeight);

    return camera;
}
