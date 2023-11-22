//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ramses-utils.h"
#include "ramses/client/RamsesClient.h"
#include "ramses/client/Scene.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Node.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/MipLevelData.h"
#include "ramses/client/PickableObject.h"
#include "ramses/client/TextureSwizzle.h"
#include "ramses/client/DataObject.h"

#include "impl/EffectImpl.h"
#include "impl/MeshNodeImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/PickableObjectImpl.h"

#include "internal/Core/Math3d/ProjectionParams.h"
#include "internal/Core/Utils/File.h"
#include "internal/Core/Utils/LogMacros.h"
#include "impl/SceneDumper.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"
#include "internal/PlatformAbstraction/PlatformMath.h"
#include "lodepng.h"
#include <iostream>
#include <array>

namespace ramses
{
    namespace
    {
        bool IsPowerOfTwo(uint32_t val)
        {
            while (((val & 1u) == 0u) && val > 1u)
            {
                val >>= 1u;
            }

            return (val == 1u);
        }

        uint32_t Log2(uint32_t val)
        {
            uint32_t pow = 0;
            while (((val & 1u) == 0u) && val > 1u)
            {
                pow++;
                val >>= 1u;
            }

            return pow;
        }
    }

    Texture2D* RamsesUtils::CreateTextureResourceFromPng(const char* pngFilePath, Scene& scene, const TextureSwizzle& swizzle, std::string_view name/* = 0*/)
    {
        if (!pngFilePath)
        {
            LOG_ERROR(ramses::internal::CONTEXT_CLIENT, "RamsesUtils::CreateTextureResourceFromPng: file path is nullptr");
            return nullptr;
        }

        unsigned int width = 0;
        unsigned int height = 0;
        std::vector<unsigned char> data;

        const unsigned int ret = lodepng::decode(data, width, height, pngFilePath);
        if (ret != 0)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesUtils::CreateTextureResourceFromPng: Could not load PNG. File not found or invalid format: {} (error {}: {})",
                pngFilePath, ret, lodepng_error_text(ret));
            return nullptr;
        }

        auto dataBytes = reinterpret_cast<std::byte*>(data.data());
        const std::vector<MipLevelData> mipLevelData{ MipLevelData( dataBytes, dataBytes + data.size() ) };
        return scene.createTexture2D(ETextureFormat::RGBA8, width, height, mipLevelData, false, swizzle, name);
    }

    Texture2D* RamsesUtils::CreateTextureResourceFromPngBuffer(const std::vector<uint8_t>& pngData, Scene& scene, const TextureSwizzle& swizzle, std::string_view name)
    {
        unsigned int width = 0;
        unsigned int height = 0;
        std::vector<unsigned char> data;

        const unsigned int ret = lodepng::decode(data, width, height, pngData.data(), pngData.size());
        if (ret != 0)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesUtils::CreateTextureResourceFromPngBuffer: Could not load PNG. Invalid format (error {}: {})", ret, lodepng_error_text(ret));
            return nullptr;
        }

        auto dataBytes = reinterpret_cast<std::byte*>(data.data());
        const std::vector<MipLevelData> mipLevelData{ MipLevelData( dataBytes, dataBytes + data.size() ) };
        return scene.createTexture2D(ETextureFormat::RGBA8, width, height, mipLevelData, false, swizzle, name);
    }

    bool RamsesUtils::SaveImageBufferToPng(const std::string& filePath, const std::vector<uint8_t>& imageData, uint32_t width, uint32_t height)
    {
        if (width <= 0 || height <= 0)
        {
            LOG_ERROR(ramses::internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Given width and height cannot be 0");
            return false;
        }

        if (width * height > 268435455u)
        {
            LOG_ERROR(ramses::internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Width * height cannot exceed the size of 268435455");
            return false;
        }

        if (width * height * 4u != imageData.size())
        {
            LOG_ERROR(ramses::internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Width * height * 4 (rgba8) must exactly match the size of the image buffer");
            return false;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned int ret = lodepng::encode(filePath, reinterpret_cast<const unsigned char*>(imageData.data()), width, height);
        if (ret != 0)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Error while saving PNG file: {} (error {}: {})", filePath, ret, lodepng_error_text(ret));
            return false;
        }
        return true;
    }

    bool RamsesUtils::SaveImageBufferToPng(const std::string& filePath, std::vector<uint8_t>& imageData, uint32_t width, uint32_t height, bool flipImageBufferVertically)
    {
        if (width <= 0 || height <= 0)
        {
            LOG_ERROR(ramses::internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Given width and height cannot be 0");
            return false;
        }

        if (width * height > 268435455u)
        {
            LOG_ERROR(ramses::internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Width * height cannot exceed the size of 268435455");
            return false;
        }

        if (width * height * 4u != imageData.size())
        {
            LOG_ERROR(ramses::internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Width * height * 4 (rgba8) must exactly match the size of the image buffer");
            return false;
        }

        if (flipImageBufferVertically)
        {
            const size_t lineSize = width * 4u;
            std::vector<uint8_t> lineBufferV(lineSize);
            uint8_t* lineBuffer = lineBufferV.data();

            for (size_t i = 0u; i < height / 2u; i++)
            {
                uint8_t* endOfImageData = imageData.data() + height * lineSize;
                uint8_t* beginOfRowNumberI = imageData.data() + i * lineSize;
                uint8_t* beginOfLastRowMinusIrows = endOfImageData - ((i + 1) * lineSize);

                std::memcpy(lineBuffer, beginOfRowNumberI, lineSize); //1. fill line buffer
                std::memcpy(beginOfRowNumberI, beginOfLastRowMinusIrows, lineSize); //overwrite copied line
                std::memcpy(beginOfLastRowMinusIrows, lineBuffer, lineSize); //replace already copied part with buffer
            }
        }

        if (!SaveImageBufferToPng(filePath, imageData, width, height))
            return false;

        return true;
    }

    std::vector<MipLevelData>* RamsesUtils::GenerateMipMapsTexture2D(uint32_t originalWidth, uint32_t originalHeight, uint8_t bytesPerPixel, std::byte* data, size_t& mipMapCount)
    {
        if (!IsPowerOfTwo(originalWidth) || !IsPowerOfTwo(originalHeight))
        {
            // if no mip maps can be created only return original data
            mipMapCount = 1u;
        }
        else
        {
            mipMapCount = std::max(Log2(originalWidth), Log2(originalHeight)) + 1u;
        }

        // prepare mip data generation
        auto* mipLevelData = new std::vector<MipLevelData>(mipMapCount);

        // copy original data
        const uint32_t originalSize = originalWidth * originalHeight * bytesPerPixel;
        (*mipLevelData)[0].assign(data, data + originalSize);

        size_t currentMipMapIndex = 1u;
        while (currentMipMapIndex < mipMapCount)
        {
            const auto& originalData = (*mipLevelData)[currentMipMapIndex-1];
            uint32_t nextWidth = std::max(originalWidth >> 1, 1u);
            uint32_t nextHeight = std::max(originalHeight >> 1, 1u);
            uint32_t nextSize = nextWidth * nextHeight * bytesPerPixel;
            (*mipLevelData)[currentMipMapIndex].resize(nextSize);

            uint32_t originalRowSize = originalWidth * bytesPerPixel;
            uint32_t nextRowSize = nextWidth * bytesPerPixel;

            for (uint32_t row = 0u; row < nextHeight; row++)
            {
                for (uint32_t col = 0u; col < nextWidth; col++)
                {
                    const uint32_t nextIndex = (row * nextRowSize) + col * bytesPerPixel;
                    const uint32_t originalIndex = ((row * originalRowSize * 2u) + col * 2u * bytesPerPixel);
                    for (uint32_t i = 0u; i < bytesPerPixel; i++) // iterate through pixel components
                    {
                        // apply box filter
                        uint32_t tmp = 0u;
                        if (originalHeight > 1 && originalWidth > 1)
                        {
                            tmp = static_cast<uint32_t>(originalData[originalIndex + i]) +
                                static_cast<uint32_t>(originalData[originalIndex + i + bytesPerPixel]) +
                                static_cast<uint32_t>(originalData[originalIndex + originalRowSize + i]) +
                                static_cast<uint32_t>(originalData[originalIndex + originalRowSize + i + bytesPerPixel]);
                            tmp >>= 2; // divide by 4
                        }
                        else
                        {
                            if (originalHeight == 1)
                            {
                                tmp = static_cast<uint32_t>(originalData[originalIndex + i]) +
                                    static_cast<uint32_t>(originalData[originalIndex + i + bytesPerPixel]);
                                tmp >>= 1; // divide by 2
                            }
                            if (originalWidth == 1)
                            {
                                tmp = static_cast<uint32_t>(originalData[originalIndex + i]) +
                                    static_cast<uint32_t>(originalData[originalIndex + originalRowSize + i]);
                                tmp >>= 1; // divide by 2
                            }
                        }

                        (*mipLevelData)[currentMipMapIndex][nextIndex + i] = static_cast<std::byte>(tmp);
                    }
                }
            }

            originalWidth = nextWidth;
            originalHeight = nextHeight;
            currentMipMapIndex++;
        }

        return mipLevelData;
    }

    std::vector<CubeMipLevelData>* RamsesUtils::GenerateMipMapsTextureCube(uint32_t faceWidth, uint32_t faceHeight, uint8_t bytesPerPixel, std::byte* data, size_t& mipMapCount)
    {
        const uint32_t faceSize = faceWidth * faceHeight * bytesPerPixel;
        mipMapCount = 0u;
        std::array<std::vector<MipLevelData>*,6> faceMips{};
        faceMips[0] = GenerateMipMapsTexture2D(faceWidth, faceHeight, bytesPerPixel, &data[faceSize * 0], mipMapCount);
        faceMips[1] = GenerateMipMapsTexture2D(faceWidth, faceHeight, bytesPerPixel, &data[faceSize * 1], mipMapCount);
        faceMips[2] = GenerateMipMapsTexture2D(faceWidth, faceHeight, bytesPerPixel, &data[faceSize * 2], mipMapCount);
        faceMips[3] = GenerateMipMapsTexture2D(faceWidth, faceHeight, bytesPerPixel, &data[faceSize * 3], mipMapCount);
        faceMips[4] = GenerateMipMapsTexture2D(faceWidth, faceHeight, bytesPerPixel, &data[faceSize * 4], mipMapCount);
        faceMips[5] = GenerateMipMapsTexture2D(faceWidth, faceHeight, bytesPerPixel, &data[faceSize * 5], mipMapCount);

        auto cubeMipMaps = new std::vector<CubeMipLevelData>(mipMapCount);
        for (size_t level = 0; level < mipMapCount; level++)
        {
            (*cubeMipMaps)[level] = CubeMipLevelData{
                std::move((*faceMips[0])[level]),
                std::move((*faceMips[1])[level]),
                std::move((*faceMips[2])[level]),
                std::move((*faceMips[3])[level]),
                std::move((*faceMips[4])[level]),
                std::move((*faceMips[5])[level])};
        }

        for (auto*& ptr : faceMips)
        {
            delete ptr;
        }
        return cubeMipMaps;
    }

    void RamsesUtils::DeleteGeneratedMipMaps(std::vector<MipLevelData>*& data)
    {
        delete data;
        data = nullptr;
    }

    void RamsesUtils::DeleteGeneratedMipMaps(std::vector<CubeMipLevelData>*& data)
    {
        delete data;
        data = nullptr;
    }

    nodeId_t RamsesUtils::GetNodeId(const Node& node)
    {
        return nodeId_t(node.impl().getNodeHandle().asMemoryHandle());
    }

    bool RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(float fov, float aspectRatio, float nearPlane, float farPlane, DataObject& frustumPlanesData, DataObject& nearFarPlanesData)
    {
        const auto params = ramses::internal::ProjectionParams::Perspective(fov, aspectRatio, nearPlane, farPlane);
        if (!params.isValid())
        {
            LOG_ERROR(ramses::internal::CONTEXT_CLIENT, "RamsesUtils::SetPerspectiveCameraFrustumToDataObjects failed: invalid frustum planes");
            return false;
        }

        if (!frustumPlanesData.setValue(ramses::vec4f{ params.leftPlane, params.rightPlane, params.bottomPlane, params.topPlane }) ||
            !nearFarPlanesData.setValue(ramses::vec2f{ params.nearPlane, params.farPlane }))
            return false;

        return true;
    }

    void RamsesUtils::DumpUnrequiredSceneObjects(const Scene& scene)
    {
        LOG_INFO_F(ramses::internal::CONTEXT_CLIENT, [&](ramses::internal::StringOutputStream& output) {
            internal::SceneDumper sceneDumper{ scene.impl() };
            sceneDumper.dumpUnrequiredObjects(output);
            });
    }

    void RamsesUtils::DumpUnrequiredSceneObjectsToFile(const Scene& scene, std::ostream& out)
    {
        ramses::internal::StringOutputStream output;
        internal::SceneDumper sceneDumper{ scene.impl() };
        sceneDumper.dumpUnrequiredObjects(output);
        out << output.release();
    }
}

