//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Base/TextureUploadingAdapter_Base.h"
#include "RendererAPI/IDevice.h"

namespace ramses_internal
{
    TextureUploadingAdapter_Base::TextureUploadingAdapter_Base(IDevice& device)
        : m_device(device)
    {
    }

    void TextureUploadingAdapter_Base::uploadTexture2D(DeviceResourceHandle textureHandle, UInt32 width, UInt32 height, ETextureFormat format, const UInt8* data, const TextureSwizzleArray& swizzle)
    {
        m_device.uploadStreamTexture2D(textureHandle, width, height, format, data, swizzle);
    }
}
