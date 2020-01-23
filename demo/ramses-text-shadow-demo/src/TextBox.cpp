//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TextBox.h"
#include "ramses-utils.h"
#include <algorithm>

TextBox::TextBox(std::u32string         string,
                 ramses::TextCache&     textCache,
                 ramses::FontInstanceId fontInstance,
                 int32_t                lineHeight,
                 ramses::RamsesClient&  client,
                 ramses::Scene&         scene,
                 ramses::RenderGroup*   renderGroup,
                 int32_t                renderOrder)
    : GraphicalItem(scene, renderGroup)
    , m_translateNode(*scene.createNode())
{
    ramses::LayoutUtils::StringBoundingBox boundingBox;
    createTextNodes(string, textCache, fontInstance, lineHeight, client, m_translateNode, boundingBox);

    const uint32_t width  = boundingBox.width;
    const uint32_t height = boundingBox.height;

    if (nullptr == renderGroup)
    {
        initOutputBuffer(width, height, renderOrder);

        m_offsetX = boundingBox.offsetX;
        m_offsetY = boundingBox.offsetY;
    }

    for (auto textLine : m_textLines)
    {
        m_renderGroup->addMeshNode(*textCache.getTextLine(textLine)->meshNode, (nullptr != renderGroup) ? renderOrder : 0);
    }
    setPosition(0, 0);
}

void TextBox::createTextNodes(std::u32string             string,
                              ramses::TextCache&         textCache,
                              ramses::FontInstanceId     fontInstance,
                              int32_t lineHeight,
                              ramses::RamsesClient&      client,
                              ramses::Node&              node,
                              ramses::LayoutUtils::StringBoundingBox& boundingBox)
{
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-text-shadow.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-text-shadow.frag");

    effectDesc.setAttributeSemantic("a_position", ramses::EEffectAttributeSemantic_TextPositions);
    effectDesc.setAttributeSemantic("a_texcoord", ramses::EEffectAttributeSemantic_TextTextureCoordinates);
    effectDesc.setUniformSemantic("u_texture", ramses::EEffectUniformSemantic_TextTexture);
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    ramses::Effect* textEffect = client.createEffect(effectDesc);
    ramses::UniformInput colorInput;
    textEffect->findUniformInput("u_color", colorInput);

    int32_t verticalOffset = 0;

    int32_t bbxmin = std::numeric_limits<int32_t>::max();
    int32_t bbymin = std::numeric_limits<int32_t>::max();
    int32_t bbxmax = std::numeric_limits<int32_t>::min();
    int32_t bbymax = std::numeric_limits<int32_t>::min();

    bool lastLine = false;
    size_t currentOffset = 0;
    while (!lastLine)
    {
        size_t nextNewLine = string.find_first_of('\n', currentOffset);

        lastLine = (nextNewLine >= string.size());
        nextNewLine = std::min(nextNewLine, string.size());
        const auto line = string.substr(currentOffset, nextNewLine - currentOffset);

        const auto positionedGlyphs = textCache.getPositionedGlyphs(line, fontInstance);
        const auto textLineId = textCache.createTextLine(positionedGlyphs, *textEffect);

        auto textLine = textCache.getTextLine(textLineId);

        ramses::Appearance& textAppearance = *textLine->meshNode->getAppearance();
        textAppearance.setInputValueVector4f(colorInput, 1.0f, 1.0f, 0.95f, 1.0f);
        textAppearance.setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
        textAppearance.setBlendingFactors(ramses::EBlendFactor_SrcAlpha,
            ramses::EBlendFactor_OneMinusSrcAlpha,
            ramses::EBlendFactor_One,
            ramses::EBlendFactor_OneMinusSrcAlpha);

        textLine->meshNode->setTranslation(0.0f, static_cast<float>(verticalOffset), 0.0f);
        node.addChild(*textLine->meshNode);

        m_textLines.push_back(textLineId);

        currentOffset = nextNewLine;
        ramses::LayoutUtils::StringBoundingBox boundingBoxLine =
            ramses::LayoutUtils::GetBoundingBoxForString(positionedGlyphs.begin(), positionedGlyphs.end());
        boundingBoxLine.offsetY += verticalOffset;

        bbxmin = std::min(bbxmin, boundingBoxLine.offsetX);
        bbymin = std::min(bbymin, boundingBoxLine.offsetY);
        bbxmax = std::max(bbxmax, static_cast<int32_t>(boundingBoxLine.offsetX + boundingBoxLine.width));
        bbymax = std::max(bbymax, static_cast<int32_t>(boundingBoxLine.offsetY + boundingBoxLine.height));

        verticalOffset -= lineHeight;

        if (!lastLine && '\n' == string[nextNewLine])
        {
            currentOffset++;
        }
    }
    if (bbxmax >= bbxmin && bbymax >= bbymin)
    {
        boundingBox.width   = bbxmax - bbxmin;
        boundingBox.height  = bbymax - bbymin;
        boundingBox.offsetX = bbxmin;
        boundingBox.offsetY = bbymin;
    }
    else
    {
        boundingBox.width   = 0;
        boundingBox.height  = 0;
        boundingBox.offsetX = 0;
        boundingBox.offsetY = 0;
    }
}

void TextBox::setPosition(int32_t x, int32_t y)
{
    m_translateNode.setTranslation(static_cast<float>(x - m_offsetX), static_cast<float>(y - m_offsetY), -0.5f);
}

int32_t TextBox::getOffsetX() const
{
    return m_offsetX;
}

int32_t TextBox::getOffsetY() const
{
    return m_offsetY;
}
