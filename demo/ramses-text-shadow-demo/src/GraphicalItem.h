//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXT_SHADOW_DEMO_GRAPHICALITEM_H
#define RAMSES_TEXT_SHADOW_DEMO_GRAPHICALITEM_H

#include "ramses-client.h"

class GraphicalItem
{
public:
    GraphicalItem(ramses::Scene& scene, ramses::RenderGroup* renderGroup = nullptr);

    void                  initOutputBuffer(uint32_t width, uint32_t height, int32_t renderOrder);
    ramses::RenderBuffer* getOutputBuffer();

protected:
    ramses::Scene&        m_scene;
    ramses::RenderGroup*  m_renderGroup  = nullptr;
    ramses::RenderBuffer* m_renderBuffer = nullptr;
};

#endif
