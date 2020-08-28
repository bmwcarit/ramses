//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/Texture2DBuffer.h"

// Internal
#include "Texture2DBufferImpl.h"

namespace ramses
{
    Texture2DBuffer::Texture2DBuffer(Texture2DBufferImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    Texture2DBuffer::~Texture2DBuffer()
    {
    }

    status_t Texture2DBuffer::updateData(uint32_t mipLevel, uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height, const void* data)
    {
        const status_t status = impl.setData(reinterpret_cast<const ramses_internal::Byte*>(data), mipLevel, offsetX, offsetY, width, height);
        LOG_HL_CLIENT_API6(status, LOG_API_GENERIC_PTR_STRING(data), mipLevel, offsetX, offsetY, width, height);
        return status;
    }

    uint32_t Texture2DBuffer::getMipLevelCount() const
    {
        return impl.getMipLevelCount();
    }

    status_t Texture2DBuffer::getMipLevelSize(uint32_t mipLevel, uint32_t& widthOut, uint32_t& heightOut) const
    {
        return impl.getMipLevelSize(mipLevel, widthOut, heightOut);
    }

    uint32_t Texture2DBuffer::getMipLevelDataSizeInBytes(uint32_t mipLevel) const
    {
        return impl.getMipLevelDataSizeInBytes(mipLevel);
    }

    ETextureFormat Texture2DBuffer::getTexelFormat() const
    {
        return impl.getTexelFormat();
    }

    status_t Texture2DBuffer::getMipLevelData(uint32_t mipLevel, void* buffer, uint32_t bufferSize) const
    {
        return impl.getMipLevelData(mipLevel, static_cast<char *>(buffer), bufferSize);
    }

}
