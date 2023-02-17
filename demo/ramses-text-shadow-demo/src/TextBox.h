//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXT_SHADOW_DEMO_TEXTBOX_H
#define RAMSES_TEXT_SHADOW_DEMO_TEXTBOX_H

#include "GraphicalItem.h"

#include "ramses-text-api/FontInstanceId.h"
#include "ramses-text-api/TextCache.h"
#include "ramses-text-api/LayoutUtils.h"

#include <vector>

class TextBox : public GraphicalItem
{
public:
    TextBox(const std::u32string&   string,
            ramses::TextCache&      textCache,
            ramses::FontInstanceId  fontInstance,
            int32_t                 lineHeight,
            ramses::Scene&          scene,
            ramses::RenderGroup*    renderGroup,
            int32_t                 renderOrder);

    void    setPosition(int32_t x, int32_t y);
    [[nodiscard]] int32_t getOffsetX() const;
    [[nodiscard]] int32_t getOffsetY() const;

private:
    void createTextNodes(const std::u32string&      string,
                         ramses::TextCache&         textCache,
                         ramses::FontInstanceId     fontInstance,
                         int32_t                    lineHeight,
                         ramses::Scene&             scene,
                         ramses::Node&              node,
                         ramses::LayoutUtils::StringBoundingBox& boundingBox);

    ramses::Node& m_translateNode;

    int32_t m_offsetX = 0;
    int32_t m_offsetY = 0;

    std::vector<ramses::TextLineId> m_textLines;
};

#endif
