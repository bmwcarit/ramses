//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/DataFloat.h"
#include "ramses-client-api/DataMatrix44f.h"
#include "TestEffectCreator.h"
#include "ClientTestUtils.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"

namespace ramses
{
    class AAppearanceTest : public ::testing::Test
    {
    public:

        static void SetUpTestCase()
        {
            sharedTestState = new TestEffectCreator(false);
        }

        static void TearDownTestCase()
        {
            delete sharedTestState;
            sharedTestState = NULL;
        }

        void SetUp()
        {
            EXPECT_TRUE(sharedTestState != NULL);
            appearance = sharedTestState->appearance;
        }

    protected:
        struct TextureInputInfo
        {
            UniformInput input;
            TextureSampler* sampler;
            Texture2D* texture2D;
            TextureCube* textureCube;
        };

        void getTexture2DInputInfo(TextureInputInfo& info)
        {
            EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dInput", info.input));

            const uint8_t data[] = { 1, 2, 3 };
            const MipLevelData mipData(3u, data);
            Texture2D* texture = sharedTestState->getClient().createTexture2D(1u, 1u, ETextureFormat_RGB8, 1u, &mipData, false);
            EXPECT_TRUE(texture != NULL);
            info.texture2D = texture;
            info.textureCube = NULL;

            TextureSampler* sampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
            EXPECT_TRUE(sampler != NULL);
            info.sampler = sampler;
        }

        void getTextureCubeInputInfo(TextureInputInfo& info)
        {
            EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("textureCubeInput", info.input));

            const uint8_t data[] = { 1, 2, 3 };
            const CubeMipLevelData mipData(3u, data, data, data, data, data, data);
            TextureCube* texture = sharedTestState->getClient().createTextureCube(1u, ETextureFormat_RGB8, 1u, &mipData, false);
            EXPECT_TRUE(texture != NULL);
            info.textureCube = texture;
            info.texture2D = NULL;

            TextureSampler* sampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
            EXPECT_TRUE(sampler != NULL);
            info.sampler = sampler;
        }

        static TestEffectCreator* sharedTestState;
        Appearance* appearance;
    };

    TestEffectCreator* AAppearanceTest::sharedTestState = 0;

    class AAppearanceTestWithSemanticUniforms : public AAppearanceTest
    {
    public:
        static void SetUpTestCase()
        {
            sharedTestState = new TestEffectCreator(true);
        }
    };

    TEST_F(AAppearanceTest, getsTheSameEffectUsedToCreateIt)
    {
        const Effect& effect = appearance->getEffect();
        EXPECT_EQ(&effect, sharedTestState->effect);
    }

    TEST_F(AAppearanceTest, setGetBlendingFactors)
    {
        status_t stat = appearance->setBlendingFactors(EBlendFactor_One, EBlendFactor_SrcAlpha, EBlendFactor_OneMinusSrcAlpha, EBlendFactor_DstAlpha);
        EXPECT_EQ(StatusOK, stat);
        EBlendFactor srcColor = EBlendFactor_Zero;
        EBlendFactor destColor = EBlendFactor_Zero;
        EBlendFactor srcAlpha = EBlendFactor_Zero;
        EBlendFactor destAlpha = EBlendFactor_Zero;
        stat = appearance->getBlendingFactors(srcColor, destColor, srcAlpha, destAlpha);
        EXPECT_EQ(StatusOK, stat);
        EXPECT_EQ(EBlendFactor_One, srcColor);
        EXPECT_EQ(EBlendFactor_SrcAlpha, destColor);
        EXPECT_EQ(EBlendFactor_OneMinusSrcAlpha, srcAlpha);
        EXPECT_EQ(EBlendFactor_DstAlpha, destAlpha);
    }

    TEST_F(AAppearanceTest, setGetDepthWrite)
    {
        EDepthWrite depthWriteMode = EDepthWrite_Disabled;
        EXPECT_EQ(StatusOK, appearance->setDepthWrite(EDepthWrite_Enabled));
        EXPECT_EQ(StatusOK, appearance->getDepthWriteMode(depthWriteMode));
        EXPECT_EQ(EDepthWrite_Enabled, depthWriteMode);

        EXPECT_EQ(StatusOK, appearance->setDepthWrite(EDepthWrite_Disabled));
        EXPECT_EQ(StatusOK, appearance->getDepthWriteMode(depthWriteMode));
        EXPECT_EQ(EDepthWrite_Disabled, depthWriteMode);

        EXPECT_EQ(StatusOK, appearance->setDepthWrite(EDepthWrite_Enabled));
        EXPECT_EQ(StatusOK, appearance->getDepthWriteMode(depthWriteMode));
        EXPECT_EQ(EDepthWrite_Enabled, depthWriteMode);
    }

    TEST_F(AAppearanceTest, setGetDepthFunction)
    {
        EDepthFunc depthFunc = EDepthFunc_Disabled;
        EXPECT_EQ(StatusOK, appearance->getDepthFunction(depthFunc));
        EXPECT_EQ(EDepthFunc_LessEqual, depthFunc);

        EXPECT_EQ(StatusOK, appearance->setDepthFunction(EDepthFunc_GreaterEqual));
        EXPECT_EQ(StatusOK, appearance->getDepthFunction(depthFunc));
        EXPECT_EQ(EDepthFunc_GreaterEqual, depthFunc);
    }

    TEST_F(AAppearanceTest, setGetScissorTest)
    {
        EScissorTest mode = EScissorTest_Disabled;
        EXPECT_EQ(StatusOK, appearance->setScissorTest(EScissorTest_Enabled, 1, 2, 3u, 4u));
        EXPECT_EQ(StatusOK, appearance->getScissorTestState(mode));
        EXPECT_EQ(EScissorTest_Enabled, mode);

        int16_t x = 0;
        int16_t y = 0;
        uint16_t width = 0u;
        uint16_t height = 0;
        EXPECT_EQ(StatusOK, appearance->getScissorRegion(x, y, width, height));
        EXPECT_EQ(1, x);
        EXPECT_EQ(2, y);
        EXPECT_EQ(3u, width);
        EXPECT_EQ(4u, height);
    }

    TEST_F(AAppearanceTest, setGetStencilFunc)
    {
        status_t stat = appearance->setStencilFunction(EStencilFunc_Equal, 2u, 0xef);
        EXPECT_EQ(StatusOK, stat);
        EStencilFunc func = EStencilFunc_Disabled;
        uint8_t ref = 0;
        uint8_t mask = 0;
        stat = appearance->getStencilFunction(func, ref, mask);
        EXPECT_EQ(StatusOK, stat);
        EXPECT_EQ(EStencilFunc_Equal, func);
        EXPECT_EQ(2u, ref);
        EXPECT_EQ(0xef, mask);
    }

    TEST_F(AAppearanceTest, setGetStencilOperation)
    {
        status_t stat = appearance->setStencilOperation(EStencilOperation_Decrement, EStencilOperation_Increment, EStencilOperation_DecrementWrap);
        EXPECT_EQ(StatusOK, stat);
        EStencilOperation sfail = EStencilOperation_Zero;
        EStencilOperation dpfail = EStencilOperation_Zero;
        EStencilOperation dppass = EStencilOperation_Zero;
        stat = appearance->getStencilOperation(sfail, dpfail, dppass);
        EXPECT_EQ(StatusOK, stat);
        EXPECT_EQ(EStencilOperation_Decrement, sfail);
        EXPECT_EQ(EStencilOperation_Increment, dpfail);
        EXPECT_EQ(EStencilOperation_DecrementWrap, dppass);
    }

    TEST_F(AAppearanceTest, setGetBlendOperations)
    {
        EXPECT_EQ(StatusOK, appearance->setBlendingOperations(EBlendOperation_Subtract, EBlendOperation_Max));
        EBlendOperation opColor = EBlendOperation_Disabled;
        EBlendOperation opAlpha = EBlendOperation_Disabled;
        EXPECT_EQ(StatusOK, appearance->getBlendingOperations(opColor, opAlpha));
        EXPECT_EQ(EBlendOperation_Subtract, opColor);
        EXPECT_EQ(EBlendOperation_Max, opAlpha);
    }

    TEST_F(AAppearanceTest, setGetCullMode)
    {
        ECullMode mode = ECullMode_Disabled;
        EXPECT_EQ(StatusOK, appearance->setCullingMode(ECullMode_FrontFacing));
        EXPECT_EQ(StatusOK, appearance->getCullingMode(mode));
        EXPECT_EQ(ECullMode_FrontFacing, mode);
        EXPECT_EQ(StatusOK, appearance->setCullingMode(ECullMode_Disabled));
        EXPECT_EQ(StatusOK, appearance->getCullingMode(mode));
        EXPECT_EQ(ECullMode_Disabled, mode);
    }

    TEST_F(AAppearanceTest, setGetDrawMode)
    {
        EDrawMode mode = EDrawMode_Lines;
        EXPECT_EQ(StatusOK, appearance->setDrawMode(EDrawMode_Points));
        EXPECT_EQ(StatusOK, appearance->getDrawMode(mode));
        EXPECT_EQ(EDrawMode_Points, mode);
    }

    TEST_F(AAppearanceTest, setGetColorWriteMask)
    {
        bool writeR = false;
        bool writeG = false;
        bool writeB = false;
        bool writeA = false;
        EXPECT_EQ(StatusOK, appearance->setColorWriteMask(true, false, true, false));
        EXPECT_EQ(StatusOK, appearance->getColorWriteMask(writeR, writeG, writeB, writeA));
        EXPECT_TRUE(writeR);
        EXPECT_FALSE(writeG);
        EXPECT_TRUE(writeB);
        EXPECT_FALSE(writeA);
        EXPECT_EQ(StatusOK, appearance->setColorWriteMask(false, true, false, true));
        EXPECT_EQ(StatusOK, appearance->getColorWriteMask(writeR, writeG, writeB, writeA));
        EXPECT_FALSE(writeR);
        EXPECT_TRUE(writeG);
        EXPECT_FALSE(writeB);
        EXPECT_TRUE(writeA);
    }

    TEST_F(AAppearanceTest, reportsErrorWhenGetSetMismatchingInputTypeScalar)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("integerInput", inputObject));

        const float value = 42;
        float getValue = 0;

        EXPECT_NE(StatusOK, appearance->setInputValueFloat(inputObject, value));
        EXPECT_NE(StatusOK, appearance->getInputValueFloat(inputObject, getValue));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenGetSetMismatchingInputTypeArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("integerInput", inputObject));

        const float values[] = { 42, 43, 44, 45, 46, 47 };
        float getValues[6];

        EXPECT_NE(StatusOK, appearance->setInputValueVector2f(inputObject, 3u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueVector2f(inputObject, 3u, getValues));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenGetSetMismatchingInputTypeTexture)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("integerInput", inputObject));

        const uint8_t texData[] = { 1, 2, 3 };
        const MipLevelData mipData(3u, texData);

        Texture2D* texture = sharedTestState->getClient().createTexture2D(1u, 1u, ETextureFormat_RGB8, 1u, &mipData, false);
        ASSERT_TRUE(texture != NULL);
        TextureSampler* textureSampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(textureSampler != NULL);

        EXPECT_NE(StatusOK, appearance->setInputTexture(inputObject, *textureSampler));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*texture));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenSetInputTextureFromADifferentScene)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dInput", inputObject));

        const uint8_t texData[] = { 1, 2, 3 };
        const MipLevelData mipData(3u, texData);

        Texture2D* texture = sharedTestState->getClient().createTexture2D(1u, 1u, ETextureFormat_RGB8, 1u, &mipData, false);
        ASSERT_TRUE(texture != NULL);

        ramses::Scene& anotherScene = *sharedTestState->getClient().createScene(1u);
        TextureSampler* textureSampler = anotherScene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(textureSampler != NULL);

        EXPECT_NE(StatusOK, appearance->setInputTexture(inputObject, *textureSampler));

        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(anotherScene));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*texture));
    }

    TEST_F(AAppearanceTest, getsSamplerSetToUniformInput)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dInput", inputObject));

        const uint8_t texData[] = { 1, 2, 3 };
        const MipLevelData mipData(3u, texData);
        Texture2D* texture = sharedTestState->getClient().createTexture2D(1u, 1u, ETextureFormat_RGB8, 1u, &mipData, false);
        ASSERT_TRUE(texture != NULL);
        TextureSampler* textureSampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(textureSampler != NULL);

        EXPECT_EQ(StatusOK, appearance->setInputTexture(inputObject, *textureSampler));

        const TextureSampler* actualSampler = nullptr;
        EXPECT_EQ(StatusOK, appearance->getInputTexture(inputObject, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*texture));
    }

    TEST_F(AAppearanceTest, getsNullSamplerIfNoneSetToUniformInput)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dInput", inputObject));

        const TextureSampler* actualSampler = nullptr;
        EXPECT_EQ(StatusOK, appearance->getInputTexture(inputObject, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    TEST_F(AAppearanceTest, failsToGetSamplerSetToUniformIfInputHasWrongType)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("integerInput", inputObject));

        const TextureSampler* actualSampler = nullptr;
        EXPECT_NE(StatusOK, appearance->getInputTexture(inputObject, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    /// Int32
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeInt32)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("integerInput", inputObject));

        const int32_t value = 42;
        int32_t getValue = 0;

        EXPECT_EQ(StatusOK, appearance->setInputValueInt32(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->getInputValueInt32(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeInt32Array)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("integerInputArray", inputObject));

        const int32_t value = 42;
        int32_t values[] = { value, value * 2, value * 3 };
        int32_t getValues[3] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueInt32(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueInt32(inputObject, 3u, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));

        EXPECT_NE(StatusOK, appearance->setInputValueInt32(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValueInt32(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueInt32(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueInt32(inputObject, 11u, values));
    }

    /// Float
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeFloat)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));

        const float value = 42;
        float getValue = 0;

        EXPECT_EQ(StatusOK, appearance->setInputValueFloat(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->getInputValueFloat(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeFloatArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInputArray", inputObject));

        const float value = 42;
        const float values[] = { value, value * 2, value * 3 };
        float getValues[3] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueFloat(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueFloat(inputObject, 3u, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));

        EXPECT_NE(StatusOK, appearance->setInputValueFloat(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValueFloat(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueFloat(inputObject, 0u, getValues));
        EXPECT_NE(StatusOK, appearance->getInputValueFloat(inputObject, 11u, getValues));
    }

    /// Vector2i
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector2i)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec2iInput", inputObject));

        const ramses_internal::Vector2i value(42, 24);
        ramses_internal::Vector2i getValue;

        EXPECT_EQ(StatusOK, appearance->setInputValueVector2i(inputObject, value.x, value.y));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector2i(inputObject, getValue.x, getValue.y));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector2iArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec2iInputArray", inputObject));

        const int32_t values[] = { 42, 43, 44, 45, 46, 47 };
        int32_t getValues[6] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueVector2i(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector2i(inputObject, 3u, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));

        EXPECT_NE(StatusOK, appearance->setInputValueVector2i(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValueVector2i(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueVector2i(inputObject, 0u, getValues));
        EXPECT_NE(StatusOK, appearance->getInputValueVector2i(inputObject, 11u, getValues));
    }

    /// Vector3i
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector3i)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec3iInput", inputObject));

        const ramses_internal::Vector3i value(42, 24, 4422);
        ramses_internal::Vector3i getValue;

        EXPECT_EQ(StatusOK, appearance->setInputValueVector3i(inputObject, value.x, value.y, value.z));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector3i(inputObject, getValue.x, getValue.y, getValue.z));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector3iArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec3iInputArray", inputObject));

        const int32_t values[] = { 42, 43, 44, 45, 46, 47, 48, 49, 50 };
        int32_t getValues[9] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueVector3i(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector3i(inputObject, 3u, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));

        EXPECT_NE(StatusOK, appearance->setInputValueVector3i(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValueVector3i(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueVector3i(inputObject, 0u, getValues));
        EXPECT_NE(StatusOK, appearance->getInputValueVector3i(inputObject, 11u, getValues));
    }

    /// Vector4i
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector4i)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec4iInput", inputObject));

        const ramses_internal::Vector4i value(42, 24, 44, 22);
        ramses_internal::Vector4i getValue;

        EXPECT_EQ(StatusOK, appearance->setInputValueVector4i(inputObject, value.x, value.y, value.z, value.w));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector4i(inputObject, getValue.x, getValue.y, getValue.z, getValue.w));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector4iArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec4iInputArray", inputObject));

        const int32_t values[] =
        {
            42, 43, 44, 45,
            46, 47, 48, 49,
            50, 51, 52, 53
        };
        int32_t getValues[12] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueVector4i(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector4i(inputObject, 3u, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));

        EXPECT_NE(StatusOK, appearance->setInputValueVector4i(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValueVector4i(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueVector4i(inputObject, 0u, getValues));
        EXPECT_NE(StatusOK, appearance->getInputValueVector4i(inputObject, 11u, getValues));
    }

    /// Vector2
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector2f)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec2fInput", inputObject));

        const ramses_internal::Vector2 value(42, 24);
        ramses_internal::Vector2 getValue;

        EXPECT_EQ(StatusOK, appearance->setInputValueVector2f(inputObject, value.x, value.y));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector2f(inputObject, getValue.x, getValue.y));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector2fArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec2fInputArray", inputObject));

        const float values[] = { 42, 43, 44, 45, 46, 47 };
        float getValues[6] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueVector2f(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector2f(inputObject, 3u, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));

        EXPECT_NE(StatusOK, appearance->setInputValueVector2f(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValueVector2f(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueVector2f(inputObject, 0u, getValues));
        EXPECT_NE(StatusOK, appearance->getInputValueVector2f(inputObject, 11u, getValues));
    }

    /// Vector3
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector3f)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec3fInput", inputObject));

        const ramses_internal::Vector3 value(42, 24, 44);
        ramses_internal::Vector3 getValue;

        EXPECT_EQ(StatusOK, appearance->setInputValueVector3f(inputObject, value.x, value.y, value.z));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector3f(inputObject, getValue.x, getValue.y, getValue.z));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector3fArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec3fInputArray", inputObject));

        const float values[] =
        {
            42, 43, 44,
            45, 46, 47,
            48, 49, 50
        };
        float getValues[9] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueVector3f(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector3f(inputObject, 3u, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));

        EXPECT_NE(StatusOK, appearance->setInputValueVector3f(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValueVector3f(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueVector3f(inputObject, 0u, getValues));
        EXPECT_NE(StatusOK, appearance->getInputValueVector3f(inputObject, 11u, getValues));
    }

    /// Vector4
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector4f)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec4fInput", inputObject));

        const ramses_internal::Vector4 value(42, 24, 44, 55);
        ramses_internal::Vector4 getValue;

        EXPECT_EQ(StatusOK, appearance->setInputValueVector4f(inputObject, value.x, value.y, value.z, value.w));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector4f(inputObject, getValue.x, getValue.y, getValue.z, getValue.w));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector4fArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec4fInputArray", inputObject));

        const float values[] =
        {
            42, 43, 44, 45,
            46, 47, 48, 49,
            50, 51, 52, 53
        };
        float getValues[12] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueVector4f(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector4f(inputObject, 3u, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));

        EXPECT_NE(StatusOK, appearance->setInputValueVector4f(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValueVector4f(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueVector4f(inputObject, 0u, getValues));
        EXPECT_NE(StatusOK, appearance->getInputValueVector4f(inputObject, 11u, getValues));
    }

    /// Matrix22f
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix22f)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix22fInput", inputObject));

        const float values[4] =
        {
            42, 43, 44, 45
        };
        float getValues[4] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueMatrix22f(inputObject, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueMatrix22f(inputObject, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix22fArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix22fInputArray", inputObject));

        const float values[12] =
        {
            42, 43, 44, 45,
            46, 47, 48, 49,
            50, 51, 52, 53
        };
        float getValues[12] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueMatrix22f(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueMatrix22f(inputObject, 3u, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));

        EXPECT_NE(StatusOK, appearance->setInputValueMatrix22f(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValueMatrix22f(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueMatrix22f(inputObject, 0u, getValues));
        EXPECT_NE(StatusOK, appearance->getInputValueMatrix22f(inputObject, 11u, getValues));
    }

    /// Matrix33f
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix33f)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix33fInput", inputObject));

        const float values[9] =
        {
            42, 43, 44,
            45, 46, 47,
            48, 49, 50
        };
        float getValues[9] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueMatrix33f(inputObject, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueMatrix33f(inputObject, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix33fArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix33fInputArray", inputObject));

        const float values[27] =
        {
            42, 43, 44,
            46, 47, 48,
            50, 51, 52,
            54, 55, 56,
            46, 47, 48,
            42, 43, 44,
            54, 55, 56,
            46, 47, 48,
            50, 51, 52
        };
        float getValues[27] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueMatrix33f(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueMatrix33f(inputObject, 3u, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));

        EXPECT_NE(StatusOK, appearance->setInputValueMatrix33f(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValueMatrix33f(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueMatrix33f(inputObject, 0u, getValues));
        EXPECT_NE(StatusOK, appearance->getInputValueMatrix33f(inputObject, 11u, getValues));
    }

    /// Matrix44f
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix44f)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix44fInput", inputObject));

        const float values[16] =
        {
            42, 43, 44, 45,
            46, 47, 48, 49,
            50, 51, 52, 53,
            54, 55, 56, 57
        };
        float getValues[16];

        EXPECT_EQ(StatusOK, appearance->setInputValueMatrix44f(inputObject, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueMatrix44f(inputObject, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix44fArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix44fInputArray", inputObject));

        const float values[] =
        {
            42, 43, 44, 45,
            46, 47, 48, 49,
            50, 51, 52, 53,
            54, 55, 56, 57,
            46, 47, 48, 49,
            42, 43, 44, 45,
            54, 55, 56, 57,
            46, 47, 48, 49,
            50, 51, 52, 53,
            54, 55, 56, 57,
            42, 43, 44, 45,
            50, 51, 52, 53
        };
        float getValues[48] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValueMatrix44f(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValueMatrix44f(inputObject, 3u, getValues));
        EXPECT_EQ(0, ramses_internal::PlatformMemory::Compare(values, getValues, sizeof(values)));

        EXPECT_NE(StatusOK, appearance->setInputValueMatrix44f(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValueMatrix44f(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValueMatrix44f(inputObject, 0u, getValues));
        EXPECT_NE(StatusOK, appearance->getInputValueMatrix44f(inputObject, 11u, getValues));
    }

    /// Texture2D
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeTexture2D)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dInput", inputObject));

        const uint8_t texData[] = { 1, 2, 3 };
        const MipLevelData mipData(3u, texData);

        Texture2D* texture = sharedTestState->getClient().createTexture2D(1u, 1u, ETextureFormat_RGB8, 1u, &mipData, false);
        ASSERT_TRUE(texture != NULL);
        TextureSampler* textureSampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(textureSampler != NULL);

        EXPECT_EQ(StatusOK, appearance->setInputTexture(inputObject, *textureSampler));

        const TextureSampler* actualSampler = nullptr;
        EXPECT_EQ(StatusOK, appearance->getInputTexture(inputObject, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*texture));
    }

    /// TextureCube
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeTextureCube)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("textureCubeInput", inputObject));

        const uint8_t texData[] = { 1, 2, 3 };
        const CubeMipLevelData mipData(3u, texData, texData, texData, texData, texData, texData);

        TextureCube* texture = sharedTestState->getClient().createTextureCube(1u, ETextureFormat_RGB8, 1u, &mipData, false);
        ASSERT_TRUE(texture != NULL);
        TextureSampler* textureSampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, *texture);
        ASSERT_TRUE(textureSampler != NULL);

        EXPECT_EQ(StatusOK, appearance->setInputTexture(inputObject, *textureSampler));

        const TextureSampler* actualSampler = nullptr;
        EXPECT_EQ(StatusOK, appearance->getInputTexture(inputObject, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*texture));
    }

    /// Binding data objects
    TEST_F(AAppearanceTest, uniformInputIsNotBoundInitially)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));
        EXPECT_FALSE(appearance->isInputBound(inputObject));
    }

    TEST_F(AAppearanceTest, canBindDataObjectToUniformInput)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));

        DataFloat* dataObject = sharedTestState->getScene().createDataFloat();
        ASSERT_TRUE(dataObject != NULL);

        EXPECT_EQ(StatusOK, appearance->bindInput(inputObject, *dataObject));
        EXPECT_TRUE(appearance->isInputBound(inputObject));

        EXPECT_EQ(StatusOK, appearance->unbindInput(inputObject));
        EXPECT_FALSE(appearance->isInputBound(inputObject));
    }

    TEST_F(AAppearanceTest, failsToSetOrGetValueIfInputBound)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));
        EXPECT_EQ(StatusOK, appearance->setInputValueFloat(inputObject, 666.f));

        DataFloat* dataObject = sharedTestState->getScene().createDataFloat();
        ASSERT_TRUE(dataObject != NULL);
        dataObject->setValue(333.f);

        EXPECT_EQ(StatusOK, appearance->bindInput(inputObject, *dataObject));

        const float setValue = 0.111f;
        float value = 0.f;
        EXPECT_NE(StatusOK, appearance->setInputValueFloat(inputObject, setValue));
        EXPECT_EQ(StatusOK, dataObject->getValue(value));
        EXPECT_FLOAT_EQ(333.f, value); // failed setter does not modify data object
        value = 0.f;
        EXPECT_NE(StatusOK, appearance->getInputValueFloat(inputObject, value));
        EXPECT_FLOAT_EQ(0.f, value); // failed getter does not modify out parameter

        EXPECT_EQ(StatusOK, appearance->unbindInput(inputObject));

        EXPECT_EQ(StatusOK, appearance->getInputValueFloat(inputObject, value));
        EXPECT_EQ(666.f, value); // failed setter did not modify previously set value
    }

    TEST_F(AAppearanceTest, failsToBindDataObjectToArrayUniformInput)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInputArray", inputObject));

        DataFloat* dataObject = sharedTestState->getScene().createDataFloat();
        ASSERT_TRUE(dataObject != NULL);

        EXPECT_NE(StatusOK, appearance->bindInput(inputObject, *dataObject));
    }

    TEST_F(AAppearanceTest, failsToBindDataObjectFromADifferentScene)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));

        ramses::Scene& anotherScene = *sharedTestState->getClient().createScene(1u);
        DataFloat* dataObject = anotherScene.createDataFloat();
        ASSERT_TRUE(dataObject != NULL);

        EXPECT_NE(StatusOK, appearance->bindInput(inputObject, *dataObject));

        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(anotherScene));
    }

    TEST_F(AAppearanceTest, failsToBindDataObjectOfMismatchingType)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec4fInput", inputObject));

        DataFloat* dataObject = sharedTestState->getScene().createDataFloat();
        ASSERT_TRUE(dataObject != NULL);

        EXPECT_NE(StatusOK, appearance->bindInput(inputObject, *dataObject));
    }

    TEST_F(AAppearanceTestWithSemanticUniforms, failsToBindDataObjectIfInputHasSemantics)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix44fInput", inputObject));

        DataMatrix44f* dataObject = sharedTestState->getScene().createDataMatrix44f();
        ASSERT_TRUE(dataObject != NULL);

        EXPECT_NE(StatusOK, appearance->bindInput(inputObject, *dataObject));
    }

    TEST_F(AAppearanceTest, unbindingDataObjectFallsBackToPreviouslySetValue)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));
        EXPECT_EQ(StatusOK, appearance->setInputValueFloat(inputObject, 666.f));

        DataFloat* dataObject = sharedTestState->getScene().createDataFloat();
        ASSERT_TRUE(dataObject != NULL);

        EXPECT_EQ(StatusOK, appearance->bindInput(inputObject, *dataObject));
        dataObject->setValue(13.f);

        EXPECT_EQ(StatusOK, appearance->unbindInput(inputObject));
        float value = 0.f;
        EXPECT_EQ(StatusOK, appearance->getInputValueFloat(inputObject, value));
        EXPECT_FLOAT_EQ(666.f, value);
    }

    TEST_F(AAppearanceTest, failsToUnbindIfInputIsNotBound)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));

        EXPECT_NE(StatusOK, appearance->unbindInput(inputObject));
    }

    /// Validation
    TEST_F(AAppearanceTest, reportsErrorWhenValidatedWithInvalidTextureSampler)
    {
        TextureInputInfo texture2DInputInfo;
        getTexture2DInputInfo(texture2DInputInfo);

        TextureInputInfo textureCubeInputInfo;
        getTextureCubeInputInfo(textureCubeInputInfo);

        Appearance* newAppearance = sharedTestState->getScene().createAppearance(*sharedTestState->effect, "New Appearance");
        ASSERT_TRUE(NULL != newAppearance);

        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(texture2DInputInfo.input, *texture2DInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(textureCubeInputInfo.input, *textureCubeInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DInputInfo.sampler));
        EXPECT_NE(StatusOK, newAppearance->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*newAppearance));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*texture2DInputInfo.texture2D));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureCubeInputInfo.sampler));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*textureCubeInputInfo.textureCube));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenValidatedWithInvalidTexture)
    {
        TextureInputInfo texture2DInputInfo;
        getTexture2DInputInfo(texture2DInputInfo);

        TextureInputInfo textureCubeInputInfo;
        getTextureCubeInputInfo(textureCubeInputInfo);

        Appearance* newAppearance = sharedTestState->getScene().createAppearance(*sharedTestState->effect, "New Appearance");
        ASSERT_TRUE(NULL != newAppearance);
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(texture2DInputInfo.input, *texture2DInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(textureCubeInputInfo.input, *textureCubeInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*texture2DInputInfo.texture2D));
        EXPECT_NE(StatusOK, appearance->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*newAppearance));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DInputInfo.sampler));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureCubeInputInfo.sampler));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*textureCubeInputInfo.textureCube));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenValidatedWithBoundDataObjectThatWasDestroyed)
    {
        TextureInputInfo texture2DInputInfo;
        getTexture2DInputInfo(texture2DInputInfo);

        TextureInputInfo textureCubeInputInfo;
        getTextureCubeInputInfo(textureCubeInputInfo);

        Appearance* newAppearance = sharedTestState->getScene().createAppearance(*sharedTestState->effect, "New Appearance");
        ASSERT_TRUE(NULL != newAppearance);
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(texture2DInputInfo.input, *texture2DInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(textureCubeInputInfo.input, *textureCubeInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->validate());

        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));

        DataFloat* dataObject = sharedTestState->getScene().createDataFloat();
        ASSERT_TRUE(dataObject != NULL);

        EXPECT_EQ(StatusOK, newAppearance->bindInput(inputObject, *dataObject));
        EXPECT_EQ(StatusOK, newAppearance->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*dataObject));
        EXPECT_NE(StatusOK, newAppearance->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*newAppearance));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DInputInfo.sampler));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*texture2DInputInfo.texture2D));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureCubeInputInfo.sampler));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*textureCubeInputInfo.textureCube));
    }

    TEST_F(AAppearanceTest, failsWhenWrongBlendingOperationsSet)
    {
        status_t stat = appearance->setBlendingOperations(EBlendOperation_Subtract, EBlendOperation_ReverseSubtract);
        EXPECT_EQ(StatusOK, stat);

        stat = appearance->setBlendingOperations(EBlendOperation_Add, EBlendOperation_Disabled);
        EXPECT_NE(StatusOK, stat);

        stat = appearance->setBlendingOperations(EBlendOperation_Disabled, EBlendOperation_Add);
        EXPECT_NE(StatusOK, stat);

        EBlendOperation opColor = EBlendOperation_Disabled;
        EBlendOperation opAlpha = EBlendOperation_Disabled;
        EXPECT_EQ(StatusOK, appearance->getBlendingOperations(opColor, opAlpha));
        EXPECT_EQ(EBlendOperation_Subtract, opColor);
        EXPECT_EQ(EBlendOperation_ReverseSubtract, opAlpha);
    }
}
