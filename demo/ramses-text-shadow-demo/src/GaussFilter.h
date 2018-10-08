//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXT_SHADOW_DEMO_GAUSSFILTER_H
#define RAMSES_TEXT_SHADOW_DEMO_GAUSSFILTER_H

#include "GraphicalItem.h"
#include "ramses-client.h"

class GaussFilter : public GraphicalItem
{
public:
    enum EDirection
    {
        EDirection_Horizontal = 0,
        EDirection_Vertical
    };
    GaussFilter(ramses::RenderBuffer& inputBuffer,
                EDirection            direction,
                ramses::RamsesClient& client,
                ramses::Scene&        scene,
                int32_t               renderOrder);
    void setVariance(float variance);

private:
    const uint32_t      m_maxKernelSize = 32;
    ramses::Effect*     m_effect        = nullptr;
    ramses::Appearance* m_appearance    = nullptr;
};

#endif
