//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-utils.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/MipLevelData.h"
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/TextureSwizzle.h"
#include "ramses-client-api/DataVector2f.h"
#include "ramses-client-api/DataVector4f.h"

#include "EffectImpl.h"
#include "MeshNodeImpl.h"
#include "RamsesClientImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "PickableObjectImpl.h"

#include "Math3d/ProjectionParams.h"
#include "Utils/File.h"
#include "Utils/LogMacros.h"
#include "SceneDumper.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "lodepng.h"
#include <iostream>

namespace ramses
{
    template <typename T>
    const T* RamsesUtils::TryConvert(const RamsesObject& obj)
    {
        if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(obj.getType(), TYPE_ID_OF_RAMSES_OBJECT<T>::ID))
        {
            return &RamsesObjectTypeUtils::ConvertTo<T>(obj);
        }

        return nullptr;
    }

    template <typename T>
    T* RamsesUtils::TryConvert(RamsesObject& obj)
    {
        if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(obj.getType(), TYPE_ID_OF_RAMSES_OBJECT<T>::ID))
        {
            return &RamsesObjectTypeUtils::ConvertTo<T>(obj);
        }

        return nullptr;
    }

    Texture2D* RamsesUtils::CreateTextureResourceFromPng(const char* pngFilePath, Scene& scene, const TextureSwizzle& swizzle, const char* name/* = 0*/)
    {
        if (!pngFilePath)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesUtils::CreateTextureResourceFromPng: file path is nullptr");
            return nullptr;
        }

        unsigned int width = 0;
        unsigned int height = 0;
        std::vector<unsigned char> data;

        const unsigned int ret = lodepng::decode(data, width, height, pngFilePath);
        if (ret != 0)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesUtils::CreateTextureResourceFromPng: Could not load PNG. File not found or invalid format: " <<
                      pngFilePath << " (error " << ret << ": " << lodepng_error_text(ret) << ")");
            return nullptr;
        }

        MipLevelData mipLevelData(static_cast<uint32_t>(data.size()), data.data());
        return scene.createTexture2D(ETextureFormat::RGBA8, width, height, 1, &mipLevelData, false, swizzle, ResourceCacheFlag_DoNotCache, name);
    }

    Texture2D* RamsesUtils::CreateTextureResourceFromPngBuffer(const std::vector<unsigned char>& pngData, Scene& scene, const TextureSwizzle& swizzle, const char* name)
    {
        unsigned int width = 0;
        unsigned int height = 0;
        std::vector<unsigned char> data;

        const unsigned int ret = lodepng::decode(data, width, height, pngData);
        if (ret != 0)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesUtils::CreateTextureResourceFromPngBuffer: Could not load PNG. Invalid format (error " << ret << ": " << lodepng_error_text(ret) << ")");
            return nullptr;
        }

        MipLevelData mipLevelData(static_cast<uint32_t>(data.size()), data.data());
        return scene.createTexture2D(ETextureFormat::RGBA8, width, height, 1, &mipLevelData, false, swizzle, ResourceCacheFlag_DoNotCache, name);
    }

    bool RamsesUtils::SaveImageBufferToPng(const std::string& filePath, const std::vector<uint8_t>& imageData, uint32_t width, uint32_t height)
    {
        if (width <= 0 || height <= 0)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Given width and height cannot be 0");
            return false;
        }

        if (width * height > 268435455u)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Width * height cannot exceed the size of 268435455");
            return false;
        }

        if (width * height * 4u != imageData.size())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Width * height * 4 (rgba8) must exactly match the size of the image buffer");
            return false;
        }

        const unsigned int ret = lodepng::encode(filePath, imageData, width, height);
        if (ret != 0)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Error while saving PNG file: " << filePath << " (error " << ret << ": " << lodepng_error_text(ret) << ")");
            return false;
        }
        return true;
    }

    bool RamsesUtils::SaveImageBufferToPng(const std::string& filePath, std::vector<uint8_t>& imageData, uint32_t width, uint32_t height, bool flipImageBufferVertically)
    {
        if (width <= 0 || height <= 0)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Given width and height cannot be 0");
            return false;
        }

        if (width * height > 268435455u)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Width * height cannot exceed the size of 268435455");
            return false;
        }

        if (width * height * 4u != imageData.size())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesUtils::SaveImageBufferToPng: Width * height * 4 (rgba8) must exactly match the size of the image buffer");
            return false;
        }

        if (flipImageBufferVertically)
        {
            const size_t lineSize = width * 4u;
            std::vector<unsigned char> lineBufferV(lineSize);
            unsigned char* lineBuffer = lineBufferV.data();

            for (size_t i = 0u; i < height / 2u; i++)
            {
                unsigned char* endOfImageData = imageData.data() + height * lineSize;
                unsigned char* beginOfRowNumberI = imageData.data() + i * lineSize;
                unsigned char* beginOfLastRowMinusIrows = endOfImageData - ((i + 1) * lineSize);

                std::memcpy(lineBuffer, beginOfRowNumberI, lineSize); //1. fill line buffer
                std::memcpy(beginOfRowNumberI, beginOfLastRowMinusIrows, lineSize); //overwrite copied line
                std::memcpy(beginOfLastRowMinusIrows, lineBuffer, lineSize); //replace already copied part with buffer
            }
        }

        if (!SaveImageBufferToPng(filePath, imageData, width, height))
            return false;

        return true;
    }

    bool IsPowerOfTwo(uint32_t val)
    {
        while (((val & 1) == 0) && val > 1)
        {
            val >>= 1;
        }

        return (val == 1);
    }

    uint32_t Log2(uint32_t val)
    {
        uint32_t pow = 0;
        while (((val & 1) == 0) && val > 1)
        {
            pow++;
            val >>= 1;
        }

        return pow;
    }

    MipLevelData* RamsesUtils::GenerateMipMapsTexture2D(uint32_t originalWidth, uint32_t originalHeight, uint8_t bytesPerPixel, uint8_t* data, uint32_t& mipMapCount)
    {
        // copy original data
        const uint32_t originalSize = originalWidth * originalHeight * bytesPerPixel;
        uint8_t* originalData = new uint8_t[originalSize];
        ramses_internal::PlatformMemory::Copy(originalData, data, originalSize);

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
        ramses::MipLevelData* mipLevelData = new ramses::MipLevelData[mipMapCount];

        mipLevelData[0].m_size = originalSize;
        mipLevelData[0].m_data = originalData;

        uint32_t currentMipMapIndex = 1u;
        while (currentMipMapIndex < mipMapCount)
        {
            uint32_t nextWidth = std::max(originalWidth >> 1, 1u);
            uint32_t nextHeight = std::max(originalHeight >> 1, 1u);
            uint32_t nextSize = nextWidth * nextHeight * bytesPerPixel;
            uint8_t* nextData = new uint8_t[nextSize];

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

                        nextData[nextIndex + i] = static_cast<uint8_t>(tmp);
                    }
                }
            }

            mipLevelData[currentMipMapIndex].m_size = nextSize;
            mipLevelData[currentMipMapIndex].m_data = nextData;

            originalData = nextData;
            nextData = nullptr;
            originalWidth = nextWidth;
            originalHeight = nextHeight;
            currentMipMapIndex++;
        }

        return mipLevelData;
    }

    CubeMipLevelData* RamsesUtils::GenerateMipMapsTextureCube(uint32_t faceWidth, uint32_t faceHeight, uint8_t bytesPerPixel, uint8_t* data, uint32_t& mipMapCount)
    {
        const uint32_t faceSize = faceWidth * faceHeight * bytesPerPixel;
        mipMapCount = 0u;
        MipLevelData* faceMips[6];
        faceMips[0] = GenerateMipMapsTexture2D(faceWidth, faceHeight, bytesPerPixel, &data[faceSize * 0], mipMapCount);
        faceMips[1] = GenerateMipMapsTexture2D(faceWidth, faceHeight, bytesPerPixel, &data[faceSize * 1], mipMapCount);
        faceMips[2] = GenerateMipMapsTexture2D(faceWidth, faceHeight, bytesPerPixel, &data[faceSize * 2], mipMapCount);
        faceMips[3] = GenerateMipMapsTexture2D(faceWidth, faceHeight, bytesPerPixel, &data[faceSize * 3], mipMapCount);
        faceMips[4] = GenerateMipMapsTexture2D(faceWidth, faceHeight, bytesPerPixel, &data[faceSize * 4], mipMapCount);
        faceMips[5] = GenerateMipMapsTexture2D(faceWidth, faceHeight, bytesPerPixel, &data[faceSize * 5], mipMapCount);

        CubeMipLevelData* cubeMipMaps = new CubeMipLevelData[mipMapCount];
        for (uint32_t level = 0; level < mipMapCount; level++)
        {
            new (&cubeMipMaps[level]) CubeMipLevelData(
                faceMips[0][level].m_size,
                faceMips[0][level].m_data,
                faceMips[1][level].m_data,
                faceMips[2][level].m_data,
                faceMips[3][level].m_data,
                faceMips[4][level].m_data,
                faceMips[5][level].m_data);
        }

        for (uint8_t level = 0; level < 6u; level++)
        {
            delete[] faceMips[level];
        }
        return cubeMipMaps;
    }

    void RamsesUtils::DeleteGeneratedMipMaps(MipLevelData*& data, uint32_t mipMapCount)
    {
        for (uint32_t i = 0; i < mipMapCount; i++)
        {
            delete[] data[i].m_data;
        }

        delete[] data;
        data = nullptr;
    }

    void RamsesUtils::DeleteGeneratedMipMaps(CubeMipLevelData*& data, uint32_t mipMapCount)
    {
        for (size_t level = 0u; level < mipMapCount; level++)
        {
            delete[] data[level].m_dataPX;
            delete[] data[level].m_dataNX;
            delete[] data[level].m_dataPY;
            delete[] data[level].m_dataNY;
            delete[] data[level].m_dataPZ;
            delete[] data[level].m_dataNZ;
        }
        delete[] data;
        data = nullptr;
    }

    nodeId_t RamsesUtils::GetNodeId(const Node& node)
    {
        return nodeId_t(node.impl.getNodeHandle().asMemoryHandle());
    }

    bool RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(float fov, float aspectRatio, float nearPlane, float farPlane, DataVector4f& frustumPlanesData, DataVector2f& nearFarPlanesData)
    {
        const auto params = ramses_internal::ProjectionParams::Perspective(fov, aspectRatio, nearPlane, farPlane);
        if (!params.isValid())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesUtils::SetPerspectiveCameraFrustumToDataObjects failed: invalid frustum planes");
            return false;
        }

        if (frustumPlanesData.setValue(params.leftPlane, params.rightPlane, params.bottomPlane, params.topPlane) != StatusOK ||
            nearFarPlanesData.setValue(params.nearPlane, params.farPlane) != StatusOK)
            return false;

        return true;
    }

    void RamsesUtils::DumpUnrequiredSceneObjects(const Scene& scene)
    {
        LOG_INFO_F(ramses_internal::CONTEXT_CLIENT, [&](ramses_internal::StringOutputStream& output) {
            SceneDumper sceneDumper{ scene.impl };
            sceneDumper.dumpUnrequiredObjects(output);
            });
    }

    void RamsesUtils::DumpUnrequiredSceneObjectsToFile(const Scene& scene, std::ostream& out)
    {
        ramses_internal::StringOutputStream output;
        SceneDumper sceneDumper{ scene.impl };
        sceneDumper.dumpUnrequiredObjects(output);
        out << output.release();
    }
}

// include all RamsesObject to instantiate conversion templates
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/DataFloat.h"
#include "ramses-client-api/DataInt32.h"
#include "ramses-client-api/DataVector2f.h"
#include "ramses-client-api/DataVector2i.h"
#include "ramses-client-api/DataVector3f.h"
#include "ramses-client-api/DataVector3i.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-client-api/DataVector4i.h"
#include "ramses-client-api/DataMatrix22f.h"
#include "ramses-client-api/DataMatrix33f.h"
#include "ramses-client-api/DataMatrix44f.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/SceneReference.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/TextureSamplerMS.h"
#include "ramses-client-api/TextureSamplerExternal.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/ArrayResource.h"

#define INSTANTIATE_CONVERT_TEMPLATE(_objType) \
    template RAMSES_API const ramses::_objType* ramses::RamsesUtils::TryConvert<ramses::_objType>(const ramses::RamsesObject& obj); \
    template RAMSES_API ramses::_objType* ramses::RamsesUtils::TryConvert<ramses::_objType>(ramses::RamsesObject& obj);

INSTANTIATE_CONVERT_TEMPLATE(ClientObject)
INSTANTIATE_CONVERT_TEMPLATE(RamsesObject)
INSTANTIATE_CONVERT_TEMPLATE(SceneObject)
INSTANTIATE_CONVERT_TEMPLATE(RamsesClient)
INSTANTIATE_CONVERT_TEMPLATE(Scene)
INSTANTIATE_CONVERT_TEMPLATE(Node)
INSTANTIATE_CONVERT_TEMPLATE(MeshNode)
INSTANTIATE_CONVERT_TEMPLATE(Camera)
INSTANTIATE_CONVERT_TEMPLATE(PerspectiveCamera)
INSTANTIATE_CONVERT_TEMPLATE(OrthographicCamera)
INSTANTIATE_CONVERT_TEMPLATE(Effect)
INSTANTIATE_CONVERT_TEMPLATE(Appearance)
INSTANTIATE_CONVERT_TEMPLATE(GeometryBinding)
INSTANTIATE_CONVERT_TEMPLATE(PickableObject)
INSTANTIATE_CONVERT_TEMPLATE(Resource)
INSTANTIATE_CONVERT_TEMPLATE(Texture2D)
INSTANTIATE_CONVERT_TEMPLATE(Texture3D)
INSTANTIATE_CONVERT_TEMPLATE(TextureCube)
INSTANTIATE_CONVERT_TEMPLATE(ArrayResource)
INSTANTIATE_CONVERT_TEMPLATE(RenderGroup)
INSTANTIATE_CONVERT_TEMPLATE(RenderPass)
INSTANTIATE_CONVERT_TEMPLATE(BlitPass)
INSTANTIATE_CONVERT_TEMPLATE(TextureSampler)
INSTANTIATE_CONVERT_TEMPLATE(TextureSamplerMS)
INSTANTIATE_CONVERT_TEMPLATE(TextureSamplerExternal)
INSTANTIATE_CONVERT_TEMPLATE(RenderBuffer)
INSTANTIATE_CONVERT_TEMPLATE(RenderTarget)
INSTANTIATE_CONVERT_TEMPLATE(DataObject)
INSTANTIATE_CONVERT_TEMPLATE(DataFloat)
INSTANTIATE_CONVERT_TEMPLATE(DataVector2f)
INSTANTIATE_CONVERT_TEMPLATE(DataVector3f)
INSTANTIATE_CONVERT_TEMPLATE(DataVector4f)
INSTANTIATE_CONVERT_TEMPLATE(DataMatrix22f)
INSTANTIATE_CONVERT_TEMPLATE(DataMatrix33f)
INSTANTIATE_CONVERT_TEMPLATE(DataMatrix44f)
INSTANTIATE_CONVERT_TEMPLATE(DataInt32)
INSTANTIATE_CONVERT_TEMPLATE(DataVector2i)
INSTANTIATE_CONVERT_TEMPLATE(DataVector3i)
INSTANTIATE_CONVERT_TEMPLATE(DataVector4i)
INSTANTIATE_CONVERT_TEMPLATE(ArrayBuffer)
INSTANTIATE_CONVERT_TEMPLATE(Texture2DBuffer)
INSTANTIATE_CONVERT_TEMPLATE(SceneReference)
