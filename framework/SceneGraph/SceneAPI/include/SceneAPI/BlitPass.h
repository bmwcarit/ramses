//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_BLITPASS_H
#define RAMSES_INTERNAL_BLITPASS_H

#include "SceneAPI/SceneTypes.h"
#include "SceneAPI/PixelRectangle.h"

namespace ramses_internal
{
    struct BlitPass
    {
        bool               isEnabled = true;
        int32_t              renderOrder = 0;
        RenderBufferHandle sourceRenderBuffer;
        RenderBufferHandle destinationRenderBuffer;
        PixelRectangle     sourceRegion;
        PixelRectangle     destinationRegion;
    };
}

#endif


