//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXT_TEXTLAYOUTDEMO_H
#define RAMSES_TEXT_TEXTLAYOUTDEMO_H

#include "ramses-client.h"
#include "ramses-text-api/FontRegistry.h"
#include "ramses-text-api/FontInstanceOffsets.h"
#include "ramses-text-api/TextCache.h"

/// Text sample application visualizing font metrics in RAMSES text API.
/** Creates a font, text style, text node and inserts a single text object into the node.
 *  Shows the string "ExampleBasicText" on the screen. */
class TextLayoutDemo
{
public:
    TextLayoutDemo(int argc, char* argv[]);
    ~TextLayoutDemo();

private:

    ramses::Effect& createTextEffect();
    ramses::Effect& createColorQuadEffect();
    ramses::Camera* createOrthographicCamera();

    void addColoredBox(ramses::Node& parent, float x, float y, float width, float height, float r, float g, float b, int32_t renderOrder);

    void layoutTextInABox(
        const std::u32string& string,
        const ramses::FontInstanceOffsets& fonts,
        float boxX, float boxY, float boxWidth, float boxHeight);

    ramses::RamsesFramework m_framework;
    ramses::RamsesClient& m_client;
    ramses::Scene& m_scene;

    ramses::FontRegistry    m_fontRegistry;
    ramses::TextCache       m_textCache;
    ramses::Effect&         m_textEffect;

    ramses::Effect&         m_colorQuadEffect;
    ramses::AttributeInput  m_colorQuadEffectPositionsInput;
    ramses::UniformInput    m_colorQuadEffectBoxSizeInput;
    ramses::UniformInput    m_colorQuadEffectColorInput;

    ramses::RenderPass&     m_renderPass;
    ramses::RenderGroup&    m_renderGroup;

    ramses::ArrayResource& m_quadIndices;
    ramses::ArrayResource& m_quadVertices;

    const uint32_t m_windowWidth;
    const uint32_t m_windowHeight;
};

#endif
