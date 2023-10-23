//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/RendererLib/Types.h"
#include "internal/SceneGraph/Resource/TextureMetaInfo.h"

#include <cstdint>

namespace ramses::internal
{
    class ITextureUploadingAdapter
    {
    public:
        virtual ~ITextureUploadingAdapter() = default;
        virtual void uploadTexture2D(DeviceResourceHandle textureHandle, uint32_t width, uint32_t height, EPixelStorageFormat format, const std::byte* data,  const TextureSwizzleArray& swizzle) = 0;
    };
}
