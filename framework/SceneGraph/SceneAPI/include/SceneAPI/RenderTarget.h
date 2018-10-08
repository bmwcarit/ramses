//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_RENDERTARGET_H
#define RAMSES_INTERNAL_RENDERTARGET_H

#include "SceneAPI/SceneTypes.h"

namespace ramses_internal
{
    struct RenderTarget
    {
        RenderBufferHandleVector renderBuffers;
    };
}

#endif
