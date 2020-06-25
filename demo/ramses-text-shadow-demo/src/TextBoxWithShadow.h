//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXT_SHADOW_DEMO_TEXTBOXWITHSHADOW_H
#define RAMSES_TEXT_SHADOW_DEMO_TEXTBOXWITHSHADOW_H

#include "GraphicalItem.h"
#include "GaussFilter.h"
#include "ImageBox.h"
#include "TextBox.h"

class TextBoxWithShadow : public GraphicalItem
{
public:
    TextBoxWithShadow(const std::u32string&     string,
                      ramses::TextCache&        textCache,
                      ramses::FontInstanceId    fontInstance,
                      int32_t                   lineHeight,
                      ramses::RamsesClient&     client,
                      ramses::Scene&            scene,
                      ramses::RenderGroup*      renderGroup,
                      int32_t                   renderOrder);

    void setPosition(int32_t x, int32_t y, int32_t shadowDisplacementX, int32_t shadowDisplacementY);
    void setShadowSharpness(float variance);

private:
    ImageBox* m_imageBoxText   = nullptr;
    ImageBox* m_imageBoxShadow = nullptr;

    int32_t m_textToShadowOffsetX = 0;
    int32_t m_textToShadowOffsetY = 0;

    TextBox*     m_textBox      = nullptr;
    GaussFilter* m_gaussFilterH = nullptr;
    GaussFilter* m_gaussFilterV = nullptr;
};

#endif
