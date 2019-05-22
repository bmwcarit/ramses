//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TextBoxWithShadow.h"
#include <algorithm>

TextBoxWithShadow::TextBoxWithShadow(std::u32string         string,
                                     ramses::TextCache&     textCache,
                                     ramses::FontInstanceId fontInstance,
                                     int32_t                lineHeight,
                                     ramses::RamsesClient&  client,
                                     ramses::Scene&         scene,
                                     ramses::RenderGroup*   renderGroup,
                                     int32_t                renderOrder)
    : GraphicalItem(scene, renderGroup)
{
    m_textBox = new TextBox(string, textCache, fontInstance, lineHeight, client, scene, nullptr, -3);
    ramses::RenderBuffer& textRenderBuffer = *m_textBox->getOutputBuffer();

    m_gaussFilterH = new GaussFilter(textRenderBuffer, GaussFilter::EDirection_Horizontal, client, scene, -2);
    m_gaussFilterV = new GaussFilter(*m_gaussFilterH->getOutputBuffer(), GaussFilter::EDirection_Vertical, client, scene, -1);

    ramses::RenderBuffer& blurredTextRenderBuffer = *m_gaussFilterV->getOutputBuffer();

    ramses::TextureSampler* textureSamplerText = scene.createTextureSampler(ramses::ETextureAddressMode_Clamp,
                                                                            ramses::ETextureAddressMode_Clamp,
                                                                            ramses::ETextureSamplingMethod_Nearest,
                                                                            ramses::ETextureSamplingMethod_Nearest,
                                                                            textRenderBuffer);

    ramses::TextureSampler* textureSamplerShadow = scene.createTextureSampler(ramses::ETextureAddressMode_Clamp,
                                                                              ramses::ETextureAddressMode_Clamp,
                                                                              ramses::ETextureSamplingMethod_Nearest,
                                                                              ramses::ETextureSamplingMethod_Nearest,
                                                                              blurredTextRenderBuffer);

    m_imageBoxShadow = new ImageBox(*textureSamplerShadow,
                                    0.0f,
                                    0.0f,
                                    0.0f,
                                    blurredTextRenderBuffer.getWidth(),
                                    blurredTextRenderBuffer.getHeight(),
                                    true,
                                    client,
                                    m_scene,
                                    m_renderGroup,
                                    renderOrder);

    m_imageBoxText = new ImageBox(*textureSamplerText,
                                  textRenderBuffer.getWidth(),
                                  textRenderBuffer.getHeight(),
                                  true,
                                  client,
                                  m_scene,
                                  m_renderGroup,
                                  renderOrder + 1);


    m_textToShadowOffsetX = -(static_cast<int32_t>(blurredTextRenderBuffer.getWidth()) -
                              static_cast<int32_t>(textRenderBuffer.getWidth())) /
                            2;
    m_textToShadowOffsetY = -(static_cast<int32_t>(blurredTextRenderBuffer.getHeight()) -
                              static_cast<int32_t>(textRenderBuffer.getHeight())) /
                            2;
    setPosition(0, 0, 0, 0);
}

void TextBoxWithShadow::setPosition(int32_t x, int32_t y, int32_t shadowDisplacementX, int32_t shadowDisplacementY)
{
    const int32_t textPosX = x + m_textBox->getOffsetX();
    const int32_t textPosY = y + m_textBox->getOffsetY();
    m_imageBoxText->setPosition(textPosX, textPosY);
    m_imageBoxShadow->setPosition(textPosX + m_textToShadowOffsetX + shadowDisplacementX,
                                  textPosY + m_textToShadowOffsetY + shadowDisplacementY);
}

void TextBoxWithShadow::setShadowSharpness(float variance)
{
    if (m_gaussFilterH)
    {
        m_gaussFilterH->setVariance(variance);
    }
    if (m_gaussFilterV)
    {
        m_gaussFilterV->setVariance(variance);
    }
}
