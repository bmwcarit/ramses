//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/Texture2DBufferImpl.h"
#include "impl/SerializationContext.h"
#include "impl/DataTypeUtils.h"
#include "impl/TextureUtils.h"
#include "impl/ErrorReporting.h"
#include "internal/SceneGraph/Scene/ClientScene.h"

namespace ramses::internal
{
    Texture2DBufferImpl::Texture2DBufferImpl(SceneImpl& scene, std::string_view textureBufferName)
        : SceneObjectImpl(scene, ERamsesObjectType::Texture2DBuffer, textureBufferName)
    {
    }

    Texture2DBufferImpl::~Texture2DBufferImpl() = default;

    void Texture2DBufferImpl::initializeFrameworkData(const ramses::internal::MipMapDimensions& mipDimensions, ETextureFormat textureFormat)
    {
        assert(!m_textureBufferHandle.isValid());
        m_textureBufferHandle = getIScene().allocateTextureBuffer(TextureUtils::GetTextureFormatInternal(textureFormat), mipDimensions, {});
    }

    void Texture2DBufferImpl::deinitializeFrameworkData()
    {
        assert(m_textureBufferHandle.isValid());
        getIScene().releaseTextureBuffer(m_textureBufferHandle);
        m_textureBufferHandle = ramses::internal::TextureBufferHandle::Invalid();
    }

    ramses::internal::TextureBufferHandle Texture2DBufferImpl::getTextureBufferHandle() const
    {
        return m_textureBufferHandle;
    }

    bool Texture2DBufferImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_textureBufferHandle;

        return true;
    }

    bool Texture2DBufferImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        inStream >> m_textureBufferHandle;

        return true;
    }

    bool Texture2DBufferImpl::setData(const std::byte* data, size_t mipLevel, uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
        {
            getErrorReporting().set("Texture2DBuffer::setData failed, width and height can not be zero.");
            return false;
        }

        const auto& mipMaps = getIScene().getTextureBuffer(m_textureBufferHandle).mipMaps;
        if (mipLevel >= mipMaps.size())
        {
            getErrorReporting().set("Texture2DBuffer::setData failed, mipLevel exceeds the number of allocated mips.");
            return false;
        }

        const auto& mip = mipMaps[mipLevel];

        if (offsetX + width > mip.width || offsetY + height > mip.height)
        {
            getErrorReporting().set("Texture2DBuffer::setData failed, updated subregion exceeds the size of the target mipLevel.");
            return false;
        }

        getIScene().updateTextureBuffer(m_textureBufferHandle, static_cast<uint32_t>(mipLevel), offsetX, offsetY, width, height, data);

        return true;
    }

    size_t Texture2DBufferImpl::getMipLevelCount() const
    {
        return getIScene().getTextureBuffer(m_textureBufferHandle).mipMaps.size();
    }

    ETextureFormat Texture2DBufferImpl::getTexelFormat() const
    {
        return TextureUtils::GetTextureFormatFromInternal(getIScene().getTextureBuffer(m_textureBufferHandle).textureFormat);
    }

    bool Texture2DBufferImpl::getMipLevelData(size_t mipLevel, char* buffer, size_t bufferSize) const
    {
        const auto& texBuffer = getIScene().getTextureBuffer(m_textureBufferHandle);
        if (mipLevel >= texBuffer.mipMaps.size())
        {
            getErrorReporting().set("Texture2DBuffer::getMipLevelData failed, requested mipLevel does not exist in Texture2DBuffer.");
            return false;
        }

        const auto& mipData = texBuffer.mipMaps[mipLevel].data;
        const size_t dataSizeToCopy = std::min<size_t>(bufferSize, mipData.size());
        ramses::internal::PlatformMemory::Copy(buffer, mipData.data(), dataSizeToCopy);

        return true;
    }

    bool Texture2DBufferImpl::getMipLevelSize(size_t mipLevel, uint32_t& widthOut, uint32_t& heightOut) const
    {
        const auto& mipMaps = getIScene().getTextureBuffer(m_textureBufferHandle).mipMaps;
        if (mipLevel >= mipMaps.size())
        {
            getErrorReporting().set("Texture2DBuffer::getMipLevelSize failed, requested mipLevel does not exist in Texture2DBuffer.");
            return false;
        }

        widthOut = mipMaps[mipLevel].width;
        heightOut = mipMaps[mipLevel].height;

        return true;
    }

    size_t Texture2DBufferImpl::getMipLevelDataSizeInBytes(size_t mipLevel) const
    {
        const auto& mipMaps = getIScene().getTextureBuffer(m_textureBufferHandle).mipMaps;
        if (mipLevel >= mipMaps.size())
            return 0u;

        const auto& mip = mipMaps[mipLevel];
        return mip.width * mip.height * GetTexelSizeFromFormat(getIScene().getTextureBuffer(m_textureBufferHandle).textureFormat);
    }

    void Texture2DBufferImpl::onValidate(ValidationReportImpl& report) const
    {
        SceneObjectImpl::onValidate(report);

        const auto& iscene = getIScene();
        const auto& mips = iscene.getTextureBuffer(getTextureBufferHandle()).mipMaps;
        const bool isInitialized = (std::accumulate(mips.cbegin(), mips.cend(), 0, [](int32_t area, const auto& m) { return area + m.usedRegion.getArea(); }) > 0);
        const bool usedAsInput = std::any_of(iscene.getTextureSamplers().cbegin(), iscene.getTextureSamplers().cend(), [handle=getTextureBufferHandle()](const auto& ts) {
            return ts.second->contentType == ramses::internal::TextureSampler::ContentType::TextureBuffer && ts.second->contentHandle == handle;
        });

        if (usedAsInput && !isInitialized)
            return report.add(EIssueType::Warning, "TextureBuffer is used in a sampler but there is no data set, this could lead to graphical glitches if actually rendered.", &getRamsesObject());

        if (!usedAsInput)
            return report.add(EIssueType::Warning, "TextureBuffer is not used anywhere, destroy it if not needed.", &getRamsesObject());
    }
}
