//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-utils.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/MipLevelData.h"

#include "EffectImpl.h"
#include "MeshNodeImpl.h"
#include "RamsesClientImpl.h"
#include "RamsesObjectTypeUtils.h"

#include "Utils/File.h"
#include "Utils/LogMacros.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "lodepng.h"

namespace ramses
{
    template <typename T>
    const T* RamsesUtils::TryConvert(const RamsesObject& obj)
    {
        if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(obj.getType(), TYPE_ID_OF_RAMSES_OBJECT<T>::ID))
        {
            return &RamsesObjectTypeUtils::ConvertTo<T>(obj);
        }

        return NULL;
    }

    template <typename T>
    T* RamsesUtils::TryConvert(RamsesObject& obj)
    {
        if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(obj.getType(), TYPE_ID_OF_RAMSES_OBJECT<T>::ID))
        {
            return &RamsesObjectTypeUtils::ConvertTo<T>(obj);
        }

        return NULL;
    }

    Texture2D* RamsesUtils::CreateTextureResourceFromPng(const char* pngFilePath, RamsesClient& ramsesClient, const char* name/* = 0*/)
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
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "RamsesUtils::CreateTextureResourceFromPng: Could not load PNG. File not found or invalid format: " << pngFilePath << " (error code " << ret << ")");
            return nullptr;
        }

        MipLevelData mipLevelData(static_cast<uint32_t>(data.size()), data.data());
        return ramsesClient.createTexture2D(width, height, ETextureFormat_RGBA8, 1, &mipLevelData, false, ResourceCacheFlag_DoNotCache, name);
    }

    Effect* RamsesUtils::CreateStandardTextEffect(RamsesClient& client, UniformInput& colorInput)
    {
        EffectDescription effectDesc;
        effectDesc.setVertexShader(
            "precision highp float;\n"
            "uniform highp mat4 mvpMatrix;\n"
            "attribute vec2 a_position; \n"
            "attribute vec2 a_texcoord; \n"
            "\n"
            "varying vec2 v_texcoord; \n"
            "\n"
            "void main()\n"
            "{\n"
            "  v_texcoord = a_texcoord; \n"
            "  gl_Position = mvpMatrix * vec4(a_position, 0.0, 1.0); \n"
            "}\n");
        effectDesc.setFragmentShader(
            "precision highp float;\n"
            "uniform sampler2D u_texture; \n"
            "uniform vec4 u_color; \n"
            "varying vec2 v_texcoord; \n"
            "\n"
            "void main(void)\n"
            "{\n"
            "  float a = texture2D(u_texture, v_texcoord).r; \n"
            "  gl_FragColor = vec4(u_color.x, u_color.y, u_color.z, a); \n"
            "}\n");

        effectDesc.setAttributeSemantic("a_position", EEffectAttributeSemantic_TextPositions);
        effectDesc.setAttributeSemantic("a_texcoord", EEffectAttributeSemantic_TextTextureCoordinates);
        effectDesc.setUniformSemantic("u_texture", EEffectUniformSemantic_TextTexture);
        effectDesc.setUniformSemantic("mvpMatrix", EEffectUniformSemantic_ModelViewProjectionMatrix);

        Effect* effect = client.impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, "");
        effect->findUniformInput("u_color", colorInput);
        return effect;
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
            mipMapCount = ramses_internal::max(Log2(originalWidth), Log2(originalHeight)) + 1u;
        }

        // prepare mip data generation
        ramses::MipLevelData* mipLevelData = new ramses::MipLevelData[mipMapCount];

        mipLevelData[0].m_size = originalSize;
        mipLevelData[0].m_data = originalData;

        uint32_t currentMipMapIndex = 1u;
        while (currentMipMapIndex < mipMapCount)
        {
            uint32_t nextWidth = ramses_internal::max(originalWidth >> 1, 1u);
            uint32_t nextHeight = ramses_internal::max(originalHeight >> 1, 1u);
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
            nextData = NULL;
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

    void RamsesUtils::DeleteGeneratedMipMaps(MipLevelData*& data, uint32_t numMipMaps)
    {
        for (uint32_t i = 0; i < numMipMaps; i++)
        {
            delete[] data[i].m_data;
        }

        delete[] data;
        data = NULL;
    }

    void RamsesUtils::DeleteGeneratedMipMaps(CubeMipLevelData*& data, uint32_t numMipMaps)
    {
        for (uint8_t level = 0u; level < numMipMaps; level++)
        {
            delete[] data[level].m_dataPX;
            delete[] data[level].m_dataNX;
            delete[] data[level].m_dataPY;
            delete[] data[level].m_dataNY;
            delete[] data[level].m_dataPZ;
            delete[] data[level].m_dataNZ;
        }
        delete[] data;
        data = NULL;
    }

    nodeId_t RamsesUtils::GetNodeId(const Node& node)
    {
        return nodeId_t(node.impl.getNodeHandle().asMemoryHandle());
    }
}

// include all RamsesObject to instantiate conversion templates
#include "ramses-client-api/AnimatedProperty.h"
#include "ramses-client-api/AnimatedSetter.h"
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/AnimationSystem.h"
#include "ramses-client-api/AnimationSystemRealTime.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/FloatArray.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/LocalCamera.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/RemoteCamera.h"
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
#include "ramses-client-api/SplineBezierFloat.h"
#include "ramses-client-api/SplineBezierInt32.h"
#include "ramses-client-api/SplineBezierVector2f.h"
#include "ramses-client-api/SplineBezierVector2i.h"
#include "ramses-client-api/SplineBezierVector3f.h"
#include "ramses-client-api/SplineBezierVector3i.h"
#include "ramses-client-api/SplineBezierVector4f.h"
#include "ramses-client-api/SplineBezierVector4i.h"
#include "ramses-client-api/SplineLinearFloat.h"
#include "ramses-client-api/SplineLinearInt32.h"
#include "ramses-client-api/SplineLinearVector2f.h"
#include "ramses-client-api/SplineLinearVector2i.h"
#include "ramses-client-api/SplineLinearVector3f.h"
#include "ramses-client-api/SplineLinearVector3i.h"
#include "ramses-client-api/SplineLinearVector4f.h"
#include "ramses-client-api/SplineLinearVector4i.h"
#include "ramses-client-api/SplineStepBool.h"
#include "ramses-client-api/SplineStepFloat.h"
#include "ramses-client-api/SplineStepInt32.h"
#include "ramses-client-api/SplineStepVector2f.h"
#include "ramses-client-api/SplineStepVector2i.h"
#include "ramses-client-api/SplineStepVector3f.h"
#include "ramses-client-api/SplineStepVector3i.h"
#include "ramses-client-api/SplineStepVector4f.h"
#include "ramses-client-api/SplineStepVector4i.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/VertexDataBuffer.h"
#include "ramses-client-api/IndexDataBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"

#define INSTANTIATE_CONVERT_TEMPLATE(_objType) \
    template RAMSES_API const ramses::_objType* ramses::RamsesUtils::TryConvert<ramses::_objType>(const ramses::RamsesObject& obj); \
    template RAMSES_API ramses::_objType* ramses::RamsesUtils::TryConvert<ramses::_objType>(ramses::RamsesObject& obj);

INSTANTIATE_CONVERT_TEMPLATE(ClientObject)
INSTANTIATE_CONVERT_TEMPLATE(RamsesObject)
INSTANTIATE_CONVERT_TEMPLATE(SceneObject)
INSTANTIATE_CONVERT_TEMPLATE(AnimationObject)
INSTANTIATE_CONVERT_TEMPLATE(RamsesClient)
INSTANTIATE_CONVERT_TEMPLATE(Scene)
INSTANTIATE_CONVERT_TEMPLATE(AnimationSystem)
INSTANTIATE_CONVERT_TEMPLATE(AnimationSystemRealTime)
INSTANTIATE_CONVERT_TEMPLATE(Node)
INSTANTIATE_CONVERT_TEMPLATE(MeshNode)
INSTANTIATE_CONVERT_TEMPLATE(Camera)
INSTANTIATE_CONVERT_TEMPLATE(RemoteCamera)
INSTANTIATE_CONVERT_TEMPLATE(LocalCamera)
INSTANTIATE_CONVERT_TEMPLATE(PerspectiveCamera)
INSTANTIATE_CONVERT_TEMPLATE(OrthographicCamera)
INSTANTIATE_CONVERT_TEMPLATE(Effect)
INSTANTIATE_CONVERT_TEMPLATE(AnimatedProperty)
INSTANTIATE_CONVERT_TEMPLATE(Animation)
INSTANTIATE_CONVERT_TEMPLATE(AnimationSequence)
INSTANTIATE_CONVERT_TEMPLATE(AnimatedSetter)
INSTANTIATE_CONVERT_TEMPLATE(Appearance)
INSTANTIATE_CONVERT_TEMPLATE(GeometryBinding)
INSTANTIATE_CONVERT_TEMPLATE(Spline)
INSTANTIATE_CONVERT_TEMPLATE(SplineStepBool)
INSTANTIATE_CONVERT_TEMPLATE(SplineStepFloat)
INSTANTIATE_CONVERT_TEMPLATE(SplineStepInt32)
INSTANTIATE_CONVERT_TEMPLATE(SplineStepVector2f)
INSTANTIATE_CONVERT_TEMPLATE(SplineStepVector3f)
INSTANTIATE_CONVERT_TEMPLATE(SplineStepVector4f)
INSTANTIATE_CONVERT_TEMPLATE(SplineStepVector2i)
INSTANTIATE_CONVERT_TEMPLATE(SplineStepVector3i)
INSTANTIATE_CONVERT_TEMPLATE(SplineStepVector4i)
INSTANTIATE_CONVERT_TEMPLATE(SplineLinearFloat)
INSTANTIATE_CONVERT_TEMPLATE(SplineLinearInt32)
INSTANTIATE_CONVERT_TEMPLATE(SplineLinearVector2f)
INSTANTIATE_CONVERT_TEMPLATE(SplineLinearVector3f)
INSTANTIATE_CONVERT_TEMPLATE(SplineLinearVector4f)
INSTANTIATE_CONVERT_TEMPLATE(SplineLinearVector2i)
INSTANTIATE_CONVERT_TEMPLATE(SplineLinearVector3i)
INSTANTIATE_CONVERT_TEMPLATE(SplineLinearVector4i)
INSTANTIATE_CONVERT_TEMPLATE(SplineBezierFloat)
INSTANTIATE_CONVERT_TEMPLATE(SplineBezierInt32)
INSTANTIATE_CONVERT_TEMPLATE(SplineBezierVector2f)
INSTANTIATE_CONVERT_TEMPLATE(SplineBezierVector3f)
INSTANTIATE_CONVERT_TEMPLATE(SplineBezierVector4f)
INSTANTIATE_CONVERT_TEMPLATE(SplineBezierVector2i)
INSTANTIATE_CONVERT_TEMPLATE(SplineBezierVector3i)
INSTANTIATE_CONVERT_TEMPLATE(SplineBezierVector4i)
INSTANTIATE_CONVERT_TEMPLATE(Resource)
INSTANTIATE_CONVERT_TEMPLATE(Texture2D)
INSTANTIATE_CONVERT_TEMPLATE(Texture3D)
INSTANTIATE_CONVERT_TEMPLATE(TextureCube)
INSTANTIATE_CONVERT_TEMPLATE(UInt16Array)
INSTANTIATE_CONVERT_TEMPLATE(UInt32Array)
INSTANTIATE_CONVERT_TEMPLATE(FloatArray)
INSTANTIATE_CONVERT_TEMPLATE(Vector2fArray)
INSTANTIATE_CONVERT_TEMPLATE(Vector3fArray)
INSTANTIATE_CONVERT_TEMPLATE(Vector4fArray)
INSTANTIATE_CONVERT_TEMPLATE(RenderGroup)
INSTANTIATE_CONVERT_TEMPLATE(RenderPass)
INSTANTIATE_CONVERT_TEMPLATE(BlitPass)
INSTANTIATE_CONVERT_TEMPLATE(TextureSampler)
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
INSTANTIATE_CONVERT_TEMPLATE(IndexDataBuffer)
INSTANTIATE_CONVERT_TEMPLATE(VertexDataBuffer)
INSTANTIATE_CONVERT_TEMPLATE(Texture2DBuffer)
INSTANTIATE_CONVERT_TEMPLATE(StreamTexture)
