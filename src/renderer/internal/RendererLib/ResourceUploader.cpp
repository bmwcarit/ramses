//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/ResourceUploader.h"
#include "internal/SceneGraph/Resource/IResource.h"
#include "internal/SceneGraph/Resource/ArrayResource.h"
#include "internal/SceneGraph/Resource/TextureResource.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/RendererLib/PlatformInterface/IBinaryShaderCache.h"
#include "internal/RendererLib/PlatformInterface/IDevice.h"
#include "internal/RendererLib/PlatformInterface/IRenderBackend.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"
#include "internal/Core/Utils/TextureMathUtils.h"
#include "internal/Components/ManagedResource.h"
#include "internal/RendererLib/ResourceDescriptor.h"

namespace ramses::internal
{
    ResourceUploader::ResourceUploader(bool asyncEffectUploadEnabled, IBinaryShaderCache* binaryShaderCache)
        : m_asyncEffectUploadEnabled(asyncEffectUploadEnabled)
        , m_binaryShaderCache(binaryShaderCache)
    {
    }

    std::optional<DeviceResourceHandle> ResourceUploader::uploadResource(IRenderBackend& renderBackend, const ResourceDescriptor& rd, uint32_t& outVRAMSize)
    {
        ManagedResource res = rd.resource;
        const IResource& resourceObject = *res.get();
        IDevice& device = renderBackend.getDevice();
        outVRAMSize = resourceObject.getDecompressedDataSize();

        switch (resourceObject.getTypeID())
        {
        case EResourceType::VertexArray:
        {
            const auto* vertArray = resourceObject.convertTo<ArrayResource>();
            const DeviceResourceHandle deviceHandle = device.allocateVertexBuffer(vertArray->getDecompressedDataSize());
            device.uploadVertexBufferData(deviceHandle, vertArray->getResourceData().data(), vertArray->getDecompressedDataSize());
            return deviceHandle;
        }
        case EResourceType::IndexArray:
        {
            const auto* indexArray = resourceObject.convertTo<ArrayResource>();
            const DeviceResourceHandle deviceHandle = device.allocateIndexBuffer(indexArray->getElementType(), indexArray->getDecompressedDataSize());
            device.uploadIndexBufferData(deviceHandle, indexArray->getResourceData().data(), indexArray->getDecompressedDataSize());
            return deviceHandle;
        }
        case EResourceType::Texture2D:
        case EResourceType::Texture3D:
        case EResourceType::TextureCube:
            return UploadTexture(device, *resourceObject.convertTo<TextureResource>(), outVRAMSize);
        case EResourceType::Effect:
        {
            const auto* effectRes = resourceObject.convertTo<EffectResource>();
            const ResourceContentHash hash = effectRes->getHash();
            const auto binaryShaderDeviceHandle = queryBinaryShaderCache(renderBackend, *effectRes, hash);
            if(binaryShaderDeviceHandle.isValid())
                return binaryShaderDeviceHandle;

            if(m_asyncEffectUploadEnabled)
                return {};

            auto effectGpuRes = renderBackend.getDevice().uploadShader(*effectRes);
            return renderBackend.getDevice().registerShader(std::move(effectGpuRes));
        }
        default:
            assert(false && "Unexpected resource type");
            return DeviceResourceHandle::Invalid();
        }
    }

    void ResourceUploader::unloadResource(IRenderBackend& renderBackend, EResourceType type, ResourceContentHash /*hash*/, const DeviceResourceHandle handle)
    {
        switch (type)
        {
        case EResourceType::VertexArray:
            renderBackend.getDevice().deleteVertexBuffer(handle);
            break;
        case EResourceType::IndexArray:
            renderBackend.getDevice().deleteIndexBuffer(handle);
            break;
        case EResourceType::Texture2D:
        case EResourceType::Texture3D:
        case EResourceType::TextureCube:
            renderBackend.getDevice().deleteTexture(handle);
            break;
        case EResourceType::Effect:
            renderBackend.getDevice().deleteShader(handle);
            break;
        default:
            assert(false && "Unexpected resource type");
        }
    }

    DeviceResourceHandle ResourceUploader::UploadTexture(IDevice& device, const TextureResource& texture, uint32_t& vramSize)
    {
        const bool generateMipsFlag = texture.getGenerateMipChainFlag();
        const auto& mipDataSizes = texture.getMipDataSizes();
        const auto numProvidedMipLevels = static_cast<uint32_t>(mipDataSizes.size());
        assert(numProvidedMipLevels == 1u || !generateMipsFlag);
        const uint32_t numMipLevelsToAllocate = generateMipsFlag ? TextureMathUtils::GetMipLevelCount(texture.getWidth(), texture.getHeight(), texture.getDepth()) : numProvidedMipLevels;
        vramSize = EstimateGPUAllocatedSizeOfTexture(texture, numMipLevelsToAllocate);
        const uint32_t cubeFaces = 6u;

        // allocate texture
        DeviceResourceHandle textureDeviceHandle;
        switch (texture.getTypeID())
        {
        case EResourceType::Texture2D:
            textureDeviceHandle = device.allocateTexture2D(texture.getWidth(), texture.getHeight(), texture.getTextureFormat(), texture.getTextureSwizzle(), numMipLevelsToAllocate, vramSize);
            break;
        case EResourceType::Texture3D:
            textureDeviceHandle = device.allocateTexture3D(texture.getWidth(), texture.getHeight(), texture.getDepth(), texture.getTextureFormat(), numMipLevelsToAllocate, vramSize);
            break;
        case EResourceType::TextureCube:
            textureDeviceHandle = device.allocateTextureCube(texture.getWidth(), texture.getTextureFormat(), texture.getTextureSwizzle(), numMipLevelsToAllocate, vramSize);
            break;
        default:
            assert(false);
        }
        assert(textureDeviceHandle.isValid());

        // upload texture data
        const std::byte* pData = texture.getResourceData().data();
        switch (texture.getTypeID())
        {
        case EResourceType::Texture2D:
        case EResourceType::Texture3D:
            for (uint32_t mipLevel = 0u; mipLevel < static_cast<uint32_t>(mipDataSizes.size()); ++mipLevel)
            {
                const uint32_t width = TextureMathUtils::GetMipSize(mipLevel, texture.getWidth());
                const uint32_t height = TextureMathUtils::GetMipSize(mipLevel, texture.getHeight());
                const uint32_t depth = TextureMathUtils::GetMipSize(mipLevel, texture.getDepth());
                device.uploadTextureData(textureDeviceHandle, mipLevel, 0u, 0u, 0u, width, height, depth, pData, mipDataSizes[mipLevel], 0);
                pData += mipDataSizes[mipLevel];
            }
            break;
        case EResourceType::TextureCube:
            static_assert(EnumTraits::VerifyElementCountIfSupported<ETextureCubeFace>(cubeFaces));
            for (uint32_t faceId = 0; faceId < cubeFaces; ++faceId)
            {
                for (uint32_t mipLevel = 0u; mipLevel < static_cast<uint32_t>(mipDataSizes.size()); ++mipLevel)
                {
                    const uint32_t faceSize = TextureMathUtils::GetMipSize(mipLevel, texture.getWidth());
                    // texture faceID is encoded in Z offset
                    device.uploadTextureData(textureDeviceHandle, mipLevel, 0u, 0u, faceId, faceSize, faceSize, 1u, pData, mipDataSizes[mipLevel], 0);
                    pData += mipDataSizes[mipLevel];
                }
            }
            break;
        default:
            assert(false);
        }

        if (generateMipsFlag)
        {
            device.generateMipmaps(textureDeviceHandle);
        }

        return textureDeviceHandle;
    }

    DeviceResourceHandle ResourceUploader::queryBinaryShaderCache(IRenderBackend& renderBackend, const EffectResource& effect, ResourceContentHash hash)
    {
        LOG_TRACE(CONTEXT_RENDERER, "ResourceUploader::queryBinaryShaderCacheAndUploadEffect: effectid:" << effect.getHash());
        IDevice& device = renderBackend.getDevice();

        if (!m_binaryShaderCache)
            return {};

        std::call_once(m_binaryShaderCache->binaryShaderFormatsReported(), [this, &device]() {
                std::vector<BinaryShaderFormatID> supportedFormats;
                device.getSupportedBinaryProgramFormats(supportedFormats);
                m_binaryShaderCache->deviceSupportsBinaryShaderFormats(supportedFormats);
            });

        if (m_binaryShaderCache->hasBinaryShader(hash))
        {
            LOG_TRACE(CONTEXT_RENDERER, "ResourceUploader::queryBinaryShaderCacheAndUploadEffect: Cache has binary shader");
            const uint32_t binaryShaderSize = m_binaryShaderCache->getBinaryShaderSize(hash);
            const BinaryShaderFormatID binaryShaderFormat = m_binaryShaderCache->getBinaryShaderFormat(hash);

            std::vector<std::byte> buffer(binaryShaderSize);
            m_binaryShaderCache->getBinaryShaderData(hash, &buffer.front(), binaryShaderSize);

            const DeviceResourceHandle binaryShaderHandle = device.uploadBinaryShader(effect, &buffer.front(), binaryShaderSize, binaryShaderFormat);

            // Always tell if the upload succeeded or not. This allows the user to know that the cache was broken (for whatever reason)
            m_binaryShaderCache->binaryShaderUploaded(hash, binaryShaderHandle.isValid());

            if (binaryShaderHandle.isValid())
            {
                return binaryShaderHandle;
            }
        }

        LOG_TRACE(CONTEXT_RENDERER, "ResourceUploader::queryBinaryShaderCacheAndUploadEffect: Cache does not have binary shader");
        // If this point is reached, we either have no cache or the cache was broken.
        return {};
    }

    void ResourceUploader::storeShaderInBinaryShaderCache(IRenderBackend& renderBackend, DeviceResourceHandle deviceHandle, const ResourceContentHash& hash, SceneId sceneid)
    {
        assert(deviceHandle.isValid());
        if (m_binaryShaderCache && m_binaryShaderCache->shouldBinaryShaderBeCached(hash, sceneid))
        {
            IDevice& device = renderBackend.getDevice();

            std::vector<std::byte> binaryShader;
            BinaryShaderFormatID format;
            if (device.getBinaryShader(deviceHandle, binaryShader, format))
            {
                assert(!binaryShader.empty());
                m_binaryShaderCache->storeBinaryShader(hash, sceneid, &binaryShader.front(), static_cast<uint32_t>(binaryShader.size()), format);
            }
            else
                LOG_WARN_P(CONTEXT_RENDERER, "ResourceUploader::storeShaderInBinaryShaderCache: failed to retrieve binary shader from device, shader cannot be stored in cache (deviceHandle={} hash={} sceneId={})", deviceHandle, hash, sceneid);
        }
    }

    uint32_t ResourceUploader::EstimateGPUAllocatedSizeOfTexture(const TextureResource& texture, uint32_t numMipLevelsToAllocate)
    {
        if (IsFormatCompressed(texture.getTextureFormat()))
        {
            return texture.getDecompressedDataSize();
        }
        if (texture.getTypeID() == EResourceType::TextureCube)
        {
            return 6u * TextureMathUtils::GetTotalMemoryUsedByMipmappedTexture(GetTexelSizeFromFormat(texture.getTextureFormat()), texture.getWidth(), texture.getWidth(), 1u, numMipLevelsToAllocate);
        }
        return TextureMathUtils::GetTotalMemoryUsedByMipmappedTexture(GetTexelSizeFromFormat(texture.getTextureFormat()), texture.getWidth(), texture.getHeight(), texture.getDepth(), numMipLevelsToAllocate);
    }
}
