//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Texture2DBufferImpl.h"
#include "SerializationContext.h"
#include "DataTypeUtils.h"
#include "Scene/ClientScene.h"
#include "TextureUtils.h"

namespace ramses
{
    Texture2DBufferImpl::Texture2DBufferImpl(SceneImpl& scene, const char* textureBufferName)
        : SceneObjectImpl(scene, ERamsesObjectType_Texture2DBuffer, textureBufferName)
    {
    }

    Texture2DBufferImpl::~Texture2DBufferImpl()
    {
    }

    void Texture2DBufferImpl::initializeFrameworkData(const ramses_internal::MipMapDimensions& mipDimensions, ETextureFormat textureFormat)
    {
        assert(!m_textureBufferHandle.isValid());
        m_textureBufferHandle = getIScene().allocateTextureBuffer(TextureUtils::GetTextureFormatInternal(textureFormat), mipDimensions);
    }

    void Texture2DBufferImpl::deinitializeFrameworkData()
    {
        assert(m_textureBufferHandle.isValid());
        getIScene().releaseTextureBuffer(m_textureBufferHandle);
        m_textureBufferHandle = ramses_internal::TextureBufferHandle::Invalid();
    }

    ramses_internal::TextureBufferHandle Texture2DBufferImpl::getTextureBufferHandle() const
    {
        return m_textureBufferHandle;
    }

    status_t Texture2DBufferImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_textureBufferHandle;

        return StatusOK;
    }

    status_t Texture2DBufferImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_textureBufferHandle;

        return StatusOK;
    }

    status_t Texture2DBufferImpl::setData(const ramses_internal::Byte* data, uint32_t mipLevel, uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
        {
            return addErrorEntry("Texture2DBuffer::setData failed, width and height can not be zero.");
        }

        const ramses_internal::MipMapDimensions& mipMapDimensions = getIScene().getTextureBuffer(m_textureBufferHandle).mipMapDimensions;
        if (mipLevel >= mipMapDimensions.size())
        {
            return addErrorEntry("Texture2DBuffer::setData failed, mipLevel exceeds the number of allocated mips.");
        }

        if (offsetX + width > mipMapDimensions[mipLevel].width || offsetY + height > mipMapDimensions[mipLevel].height)
        {
            return addErrorEntry("Texture2DBuffer::setData failed, updated subregion exceeds the size of the target mipLevel.");
        }

        getIScene().updateTextureBuffer(m_textureBufferHandle, mipLevel, offsetX, offsetY, width, height, data);

        return StatusOK;
    }

    uint32_t Texture2DBufferImpl::getMipLevelCount() const
    {
        return static_cast<uint32_t>(getIScene().getTextureBuffer(m_textureBufferHandle).mipMapDimensions.size());
    }

    ETextureFormat Texture2DBufferImpl::getTexelFormat() const
    {
        return TextureUtils::GetTextureFormatFromInternal(getIScene().getTextureBuffer(m_textureBufferHandle).textureFormat);
    }

    status_t Texture2DBufferImpl::getMipLevelData(uint32_t mipLevel, char* buffer, uint32_t bufferSize) const
    {
        const auto& texBuffer = getIScene().getTextureBuffer(m_textureBufferHandle);
        if (mipLevel >= texBuffer.mipMapDimensions.size())
        {
            return addErrorEntry("Texture2DBuffer::getMipLevelData failed, requested mipLevel does not exist in Texture2DBuffer.");
        }

        const auto& mipData = texBuffer.mipMapData[mipLevel];
        const uint32_t dataSizeToCopy = std::min<uint32_t>(bufferSize, static_cast<uint32_t>(mipData.size()));
        ramses_internal::PlatformMemory::Copy(buffer, mipData.data(), dataSizeToCopy);

        return StatusOK;
    }

    status_t Texture2DBufferImpl::getMipLevelSize(uint32_t mipLevel, uint32_t& widthOut, uint32_t& heightOut) const
    {
        const auto& mipDimensions = getIScene().getTextureBuffer(m_textureBufferHandle).mipMapDimensions;
        if (mipLevel >= mipDimensions.size())
        {
            return addErrorEntry("Texture2DBuffer::getMipLevelSize failed, requested mipLevel does not exist in Texture2DBuffer.");
        }

        widthOut = mipDimensions[mipLevel].width;
        heightOut = mipDimensions[mipLevel].height;

        return StatusOK;
    }

    uint32_t Texture2DBufferImpl::getMipLevelDataSizeInBytes(uint32_t mipLevel) const
    {
        const auto& mipDimensions = getIScene().getTextureBuffer(m_textureBufferHandle).mipMapDimensions;
        if (mipLevel >= mipDimensions.size())
            return 0u;

        const auto& mipDimension = mipDimensions[mipLevel];
        return mipDimension.width * mipDimension.height * GetTexelSizeFromFormat(getIScene().getTextureBuffer(m_textureBufferHandle).textureFormat);
    }
}
