//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ITEXTUREUPLOADINGADAPTER_H
#define RAMSES_ITEXTUREUPLOADINGADAPTER_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "SceneAPI/TextureEnums.h"
#include "RendererAPI/Types.h"
#include "Resource/TextureMetaInfo.h"

namespace ramses_internal
{
    class ITextureUploadingAdapter
    {
    public:
        virtual ~ITextureUploadingAdapter() {}
        virtual void uploadTexture2D(DeviceResourceHandle textureHandle, UInt32 width, UInt32 height, ETextureFormat format, const UInt8* data,  const TextureSwizzleArray& swizzle) = 0;
    };
}

#endif
