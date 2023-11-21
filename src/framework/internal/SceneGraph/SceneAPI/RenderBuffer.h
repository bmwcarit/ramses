//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/TextureEnums.h"

namespace ramses::internal
{
    struct RenderBuffer
    {
        uint32_t                width = 0u;
        uint32_t                height = 0u;
        EPixelStorageFormat     format = EPixelStorageFormat::Invalid;
        ERenderBufferAccessMode accessMode = ERenderBufferAccessMode::ReadWrite;
        uint32_t                sampleCount = 0u;
    };
}
