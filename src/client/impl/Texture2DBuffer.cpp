//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/Texture2DBuffer.h"

// Internal
#include "impl/Texture2DBufferImpl.h"

namespace ramses
{
    Texture2DBuffer::Texture2DBuffer(std::unique_ptr<internal::Texture2DBufferImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::Texture2DBufferImpl&>(SceneObject::m_impl) }
    {
    }

    bool Texture2DBuffer::updateData(size_t mipLevel, uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height, const void* data)
    {
        const bool status = m_impl.setData(static_cast<const std::byte*>(data), mipLevel, offsetX, offsetY, width, height);
        LOG_HL_CLIENT_API6(status, LOG_API_GENERIC_PTR_STRING(data), mipLevel, offsetX, offsetY, width, height);
        return status;
    }

    size_t Texture2DBuffer::getMipLevelCount() const
    {
        return m_impl.getMipLevelCount();
    }

    bool Texture2DBuffer::getMipLevelSize(size_t mipLevel, uint32_t& widthOut, uint32_t& heightOut) const
    {
        return m_impl.getMipLevelSize(mipLevel, widthOut, heightOut);
    }

    size_t Texture2DBuffer::getMipLevelDataSizeInBytes(size_t mipLevel) const
    {
        return m_impl.getMipLevelDataSizeInBytes(mipLevel);
    }

    ETextureFormat Texture2DBuffer::getTexelFormat() const
    {
        return m_impl.getTexelFormat();
    }

    bool Texture2DBuffer::getMipLevelData(size_t mipLevel, void* buffer, size_t bufferSize) const
    {
        return m_impl.getMipLevelData(mipLevel, static_cast<char *>(buffer), bufferSize);
    }

    internal::Texture2DBufferImpl& Texture2DBuffer::impl()
    {
        return m_impl;
    }

    const internal::Texture2DBufferImpl& Texture2DBuffer::impl() const
    {
        return m_impl;
    }
}
