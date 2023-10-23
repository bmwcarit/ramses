//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/ITextureUploadingAdapter.h"
#include "internal/SceneGraph/Resource/TextureMetaInfo.h"

namespace ramses::internal
{
    class IDevice;

    class TextureUploadingAdapter_Base : public ITextureUploadingAdapter
    {
    public:
        explicit TextureUploadingAdapter_Base(IDevice& device);
        void uploadTexture2D(DeviceResourceHandle textureHandle, uint32_t width, uint32_t height, EPixelStorageFormat format, const std::byte* data,  const TextureSwizzleArray& swizzle) override;

    protected:
        IDevice& m_device;
    };
}
