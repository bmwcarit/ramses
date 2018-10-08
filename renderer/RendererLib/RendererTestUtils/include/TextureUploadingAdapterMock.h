//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTUREUPLOADINGADAPTERMOCK_H
#define RAMSES_TEXTUREUPLOADINGADAPTERMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/ITextureUploadingAdapter.h"


namespace ramses_internal
{
    class TextureUploadingAdapterMock : public ITextureUploadingAdapter
    {
    public:
        TextureUploadingAdapterMock();
        ~TextureUploadingAdapterMock();

        MOCK_METHOD5(uploadTexture2D, void(DeviceResourceHandle, UInt32, UInt32, ETextureFormat, const UInt8*));
    };

    class TextureUploadingAdapterMockWithDestructor : public TextureUploadingAdapterMock
    {
    public:
        TextureUploadingAdapterMockWithDestructor();
        ~TextureUploadingAdapterMockWithDestructor();

        MOCK_METHOD0(Die, void());
    };
}

#endif
