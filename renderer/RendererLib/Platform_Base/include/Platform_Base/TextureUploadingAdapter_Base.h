//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTUREUPLOADINGADAPTER_BASE_H
#define RAMSES_TEXTUREUPLOADINGADAPTER_BASE_H

#include "RendererAPI/ITextureUploadingAdapter.h"

namespace ramses_internal
{
    class IDevice;

    class TextureUploadingAdapter_Base : public ITextureUploadingAdapter
    {
    public:
        TextureUploadingAdapter_Base(IDevice& device);
        virtual void uploadTexture2D(DeviceResourceHandle textureHandle, UInt32 width, UInt32 height, ETextureFormat format, const UInt8* data) override;

    protected:
        IDevice& m_device;
    };
}

#endif
