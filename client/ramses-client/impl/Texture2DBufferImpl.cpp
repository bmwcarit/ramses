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
    Texture2DBufferImpl::Texture2DBufferImpl(SceneImpl& scene, std::string_view textureBufferName)
        : SceneObjectImpl(scene, ERamsesObjectType::Texture2DBuffer, textureBufferName)
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

        const auto& mipMaps = getIScene().getTextureBuffer(m_textureBufferHandle).mipMaps;
        if (mipLevel >= mipMaps.size())
        {
            return addErrorEntry("Texture2DBuffer::setData failed, mipLevel exceeds the number of allocated mips.");
        }

        const auto& mip = mipMaps[mipLevel];

        if (offsetX + width > mip.width || offsetY + height > mip.height)
        {
            return addErrorEntry("Texture2DBuffer::setData failed, updated subregion exceeds the size of the target mipLevel.");
        }

        getIScene().updateTextureBuffer(m_textureBufferHandle, mipLevel, offsetX, offsetY, width, height, data);

        return StatusOK;
    }

    uint32_t Texture2DBufferImpl::getMipLevelCount() const
    {
        return static_cast<uint32_t>(getIScene().getTextureBuffer(m_textureBufferHandle).mipMaps.size());
    }

    ETextureFormat Texture2DBufferImpl::getTexelFormat() const
    {
        return TextureUtils::GetTextureFormatFromInternal(getIScene().getTextureBuffer(m_textureBufferHandle).textureFormat);
    }

    status_t Texture2DBufferImpl::getMipLevelData(uint32_t mipLevel, char* buffer, uint32_t bufferSize) const
    {
        const auto& texBuffer = getIScene().getTextureBuffer(m_textureBufferHandle);
        if (mipLevel >= texBuffer.mipMaps.size())
        {
            return addErrorEntry("Texture2DBuffer::getMipLevelData failed, requested mipLevel does not exist in Texture2DBuffer.");
        }

        const auto& mipData = texBuffer.mipMaps[mipLevel].data;
        const uint32_t dataSizeToCopy = std::min<uint32_t>(bufferSize, static_cast<uint32_t>(mipData.size()));
        ramses_internal::PlatformMemory::Copy(buffer, mipData.data(), dataSizeToCopy);

        return StatusOK;
    }

    status_t Texture2DBufferImpl::getMipLevelSize(uint32_t mipLevel, uint32_t& widthOut, uint32_t& heightOut) const
    {
        const auto& mipMaps = getIScene().getTextureBuffer(m_textureBufferHandle).mipMaps;
        if (mipLevel >= mipMaps.size())
        {
            return addErrorEntry("Texture2DBuffer::getMipLevelSize failed, requested mipLevel does not exist in Texture2DBuffer.");
        }

        widthOut = mipMaps[mipLevel].width;
        heightOut = mipMaps[mipLevel].height;

        return StatusOK;
    }

    uint32_t Texture2DBufferImpl::getMipLevelDataSizeInBytes(uint32_t mipLevel) const
    {
        const auto& mipMaps = getIScene().getTextureBuffer(m_textureBufferHandle).mipMaps;
        if (mipLevel >= mipMaps.size())
            return 0u;

        const auto& mip = mipMaps[mipLevel];
        return mip.width * mip.height * GetTexelSizeFromFormat(getIScene().getTextureBuffer(m_textureBufferHandle).textureFormat);
    }

    status_t Texture2DBufferImpl::validate() const
    {
        status_t status = SceneObjectImpl::validate();

        const auto& iscene = getIScene();
        const auto& mips = iscene.getTextureBuffer(getTextureBufferHandle()).mipMaps;
        const bool isInitialized = (std::accumulate(mips.cbegin(), mips.cend(), 0, [](int32_t area, const auto& m) { return area + m.usedRegion.getArea(); }) > 0);
        const bool usedAsInput = std::any_of(iscene.getTextureSamplers().cbegin(), iscene.getTextureSamplers().cend(), [handle=getTextureBufferHandle()](const auto& ts) {
            return ts.second->contentType == ramses_internal::TextureSampler::ContentType::TextureBuffer && ts.second->contentHandle == handle;
        });

        if (usedAsInput && !isInitialized)
            return addValidationMessage(EValidationSeverity::Warning, "TextureBuffer is used in a sampler but there is no data set, this could lead to graphical glitches if actually rendered.");

        if (!usedAsInput)
            return addValidationMessage(EValidationSeverity::Warning, "TextureBuffer is not used anywhere, destroy it if not needed.");

        return status;
    }
}
