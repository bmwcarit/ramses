//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXT_SHADOW_DEMO_IMAGEBOX_H
#define RAMSES_TEXT_SHADOW_DEMO_IMAGEBOX_H

#include "ramses-client.h"

class ImageBox
{
public:
    ImageBox(ramses::TextureSampler& textureSampler,
             uint32_t                width,
             uint32_t                height,
             bool                    blend,
             ramses::RamsesClient&   client,
             ramses::Scene&          scene,
             ramses::RenderGroup*    renderGroup,
             int32_t                 renderOrder);

    ImageBox(ramses::TextureSampler& textureSampler,
             float                   r,
             float                   g,
             float                   b,
             uint32_t                width,
             uint32_t                height,
             bool                    blend,
             ramses::RamsesClient&   client,
             ramses::Scene&          scene,
             ramses::RenderGroup*    renderGroup,
             int32_t                 renderOrder);

    void setPosition(int32_t x, int32_t y);

private:
    void createGeometry(ramses::TextureSampler& textureSampler,
                        uint32_t                width,
                        uint32_t                height,
                        ramses::RamsesClient&   client,
                        ramses::Scene&          scene,
                        ramses::RenderGroup*    renderGroup,
                        ramses::Appearance&     appearance,
                        int32_t                 renderOrder);

    ramses::TranslateNode& m_translateNode;
};

#endif
