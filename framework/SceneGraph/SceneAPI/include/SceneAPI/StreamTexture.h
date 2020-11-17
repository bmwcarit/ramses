//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_STREAMTEXTURE_H
#define RAMSES_INTERNAL_STREAMTEXTURE_H

#include "SceneAPI/ResourceContentHash.h"
#include "SceneAPI/WaylandIviSurfaceId.h"

namespace ramses_internal
{
    struct StreamTexture
    {
        ResourceContentHash fallbackTexture;
        bool forceFallbackTexture = false;
        WaylandIviSurfaceId source;
    };
}

#endif
