//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StreamTextureImpl.h"
#include "ramses-client-api/StreamTexture.h"

namespace ramses
{
    status_t StreamTexture::forceFallbackImage(bool forceFallbackImage)
    {
        const status_t status = impl.forceFallbackImage(forceFallbackImage);
        LOG_HL_CLIENT_API1(status, forceFallbackImage)
        return status;
    }

    bool StreamTexture::getForceFallbackImage() const
    {
        return impl.getForceFallbackImage();
    }

    StreamTexture::StreamTexture(StreamTextureImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    StreamTexture::~StreamTexture()
    {
    }

    streamSource_t StreamTexture::getStreamSourceId() const
    {
        return impl.getStreamSource();
    }
}

