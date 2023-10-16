//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureCube.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "TestEffectCreator.h"
#include "impl/EffectImpl.h"
#include "impl/DataObjectImpl.h"
#include "impl/TextureSamplerImpl.h"
#include "impl/AppearanceImpl.h"
#include "impl/AppearanceUtils.h"

namespace ramses::internal
{
    class AAppearanceTest : public ::testing::Test
    {
    public:

        static void SetUpTestSuite()
        {
            sharedTestState = std::make_unique<TestEffectCreator>(false);
        }

        static void TearDownTestSuite()
        {
            sharedTestState = nullptr;
        }

        void SetUp() override
        {
            EXPECT_TRUE(sharedTestState != nullptr);
            sharedTestState->recreateAppearence();
            appearance = sharedTestState->appearance;
        }

    protected:
        struct TextureInputInfo
        {
            std::optional<UniformInput> input;
            ramses::TextureSampler* sampler = nullptr;
            TextureSamplerMS* samplerMS = nullptr;
            TextureSamplerExternal* samplerExternal = nullptr;
            ramses::RenderBuffer* renderBuffer = nullptr;
            Texture2D* texture2D = nullptr;
            Texture3D* texture3D = nullptr;
            TextureCube* textureCube = nullptr;
        };

        static void GetTexture2DInputInfo(TextureInputInfo& info)
        {
            info.input = sharedTestState->effect->findUniformInput("texture2dInput");
            EXPECT_TRUE(info.input.has_value());

            const uint8_t data[] = { 1, 2, 3 };
            const MipLevelData mipData(3u, data);
            Texture2D* texture = sharedTestState->getScene().createTexture2D(ETextureFormat::RGB8, 1u, 1u, 1u, &mipData, false);
            EXPECT_TRUE(texture != nullptr);
            info.texture2D = texture;

            ramses::TextureSampler* sampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
            EXPECT_TRUE(sampler != nullptr);
            info.sampler = sampler;
        }

        static void GetTexture2DMSInputInfo(TextureInputInfo& info)
        {
            info.input = sharedTestState->effect->findUniformInput("texture2dMSInput");
            EXPECT_TRUE(info.input.has_value());

            ramses::RenderBuffer* renderBuffer = sharedTestState->getScene().createRenderBuffer(2u, 2u, ERenderBufferFormat::RGB8, ERenderBufferAccessMode::ReadWrite, 4u);
            EXPECT_TRUE(renderBuffer != nullptr);
            info.renderBuffer = renderBuffer;

            TextureSamplerMS* samplerMS = sharedTestState->getScene().createTextureSamplerMS(*renderBuffer, "renderBuffer");
            EXPECT_TRUE(samplerMS != nullptr);
            info.samplerMS = samplerMS;
        }

        static void GetTexture3DInputInfo(TextureInputInfo& info)
        {
            info.input = sharedTestState->effect->findUniformInput("texture3dInput");
            EXPECT_TRUE(info.input.has_value());

            const uint8_t data[] = { 1, 2, 3 };
            const MipLevelData mipData(3u, data);
            Texture3D* texture = sharedTestState->getScene().createTexture3D(ETextureFormat::RGB8, 1u, 1u, 1u, 1u, &mipData, false);
            EXPECT_TRUE(texture != nullptr);
            info.texture3D = texture;

            ramses::TextureSampler* sampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
            EXPECT_TRUE(sampler != nullptr);
            info.sampler = sampler;
        }

        static void GetTextureCubeInputInfo(TextureInputInfo& info)
        {
            info.input = sharedTestState->effect->findUniformInput("textureCubeInput");
            EXPECT_TRUE(info.input.has_value());

            const std::byte data[] = { std::byte{1}, std::byte{2}, std::byte{3} };
            const CubeMipLevelData mipData(3u, data, data, data, data, data, data);
            TextureCube* texture = sharedTestState->getScene().createTextureCube(ETextureFormat::RGB8, 1u, 1u, &mipData, false);
            EXPECT_TRUE(texture != nullptr);
            info.textureCube = texture;

            ramses::TextureSampler* sampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
            EXPECT_TRUE(sampler != nullptr);
            info.sampler = sampler;
        }

        static void GetTextureExternalInputInfo(TextureInputInfo& info)
        {
            info.input = sharedTestState->effect->findUniformInput("textureExternalInput");
            EXPECT_TRUE(info.input.has_value());

            TextureSamplerExternal* sampler = sharedTestState->getScene().createTextureSamplerExternal(ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear);
            EXPECT_TRUE(sampler != nullptr);
            info.samplerExternal = sampler;
        }

        static std::unique_ptr<TestEffectCreator> sharedTestState;
        Appearance*                               appearance{nullptr};
    };

    std::unique_ptr<TestEffectCreator> AAppearanceTest::sharedTestState;

    class AAppearanceTestWithSemanticUniforms : public AAppearanceTest
    {
    public:
        static void SetUpTestSuite()
        {
            sharedTestState = std::make_unique<TestEffectCreator>(true);
        }
    };

    TEST_F(AAppearanceTest, getsTheSameEffectUsedToCreateIt)
    {
        const Effect& effect = appearance->getEffect();
        EXPECT_EQ(&effect, sharedTestState->effect);

        const ramses::internal::DataLayout uniformLayout = sharedTestState->getInternalScene().getDataLayout(appearance->impl().getUniformDataLayout());
        const ramses::internal::ResourceContentHash& effectHashFromUniformLayout = uniformLayout.getEffectHash();
        EXPECT_EQ(appearance->getEffect().impl().getLowlevelResourceHash(), effectHashFromUniformLayout);
    }

    TEST_F(AAppearanceTest, setGetBlendingFactors)
    {
        bool stat = appearance->setBlendingFactors(EBlendFactor::One, EBlendFactor::SrcAlpha, EBlendFactor::OneMinusSrcAlpha, EBlendFactor::DstAlpha);
        EXPECT_TRUE(stat);
        EBlendFactor srcColor = EBlendFactor::Zero;
        EBlendFactor destColor = EBlendFactor::Zero;
        EBlendFactor srcAlpha = EBlendFactor::Zero;
        EBlendFactor destAlpha = EBlendFactor::Zero;
        stat = appearance->getBlendingFactors(srcColor, destColor, srcAlpha, destAlpha);
        EXPECT_TRUE(stat);
        EXPECT_EQ(EBlendFactor::One, srcColor);
        EXPECT_EQ(EBlendFactor::SrcAlpha, destColor);
        EXPECT_EQ(EBlendFactor::OneMinusSrcAlpha, srcAlpha);
        EXPECT_EQ(EBlendFactor::DstAlpha, destAlpha);
    }

    TEST_F(AAppearanceTest, setGetBlendingColor)
    {
        vec4f color{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
        //default values
        bool stat = appearance->getBlendingColor(color);
        EXPECT_TRUE(stat);
        EXPECT_EQ(color, vec4f(0.f, 0.f, 0.f, 0.f));

        const vec4f colorToSet{ 0.1f, 0.2f, 0.3f, 0.4f };
        stat = appearance->setBlendingColor(colorToSet);
        EXPECT_TRUE(stat);

        stat = appearance->getBlendingColor(color);
        EXPECT_TRUE(stat);
        EXPECT_EQ(colorToSet, color);
    }

    TEST_F(AAppearanceTest, setGetDepthWrite)
    {
        EDepthWrite depthWriteMode = EDepthWrite::Disabled;
        EXPECT_TRUE(appearance->setDepthWrite(EDepthWrite::Enabled));
        EXPECT_TRUE(appearance->getDepthWriteMode(depthWriteMode));
        EXPECT_EQ(EDepthWrite::Enabled, depthWriteMode);

        EXPECT_TRUE(appearance->setDepthWrite(EDepthWrite::Disabled));
        EXPECT_TRUE(appearance->getDepthWriteMode(depthWriteMode));
        EXPECT_EQ(EDepthWrite::Disabled, depthWriteMode);

        EXPECT_TRUE(appearance->setDepthWrite(EDepthWrite::Enabled));
        EXPECT_TRUE(appearance->getDepthWriteMode(depthWriteMode));
        EXPECT_EQ(EDepthWrite::Enabled, depthWriteMode);
    }

    TEST_F(AAppearanceTest, setGetDepthFunction)
    {
        EDepthFunc depthFunc = EDepthFunc::Disabled;
        EXPECT_TRUE(appearance->getDepthFunction(depthFunc));
        EXPECT_EQ(EDepthFunc::LessEqual, depthFunc);

        EXPECT_TRUE(appearance->setDepthFunction(EDepthFunc::GreaterEqual));
        EXPECT_TRUE(appearance->getDepthFunction(depthFunc));
        EXPECT_EQ(EDepthFunc::GreaterEqual, depthFunc);
    }

    TEST_F(AAppearanceTest, setGetScissorTest)
    {
        EScissorTest mode = EScissorTest::Disabled;
        EXPECT_TRUE(appearance->setScissorTest(EScissorTest::Enabled, 1, 2, 3u, 4u));
        EXPECT_TRUE(appearance->getScissorTestState(mode));
        EXPECT_EQ(EScissorTest::Enabled, mode);

        int16_t x = 0;
        int16_t y = 0;
        uint16_t width = 0u;
        uint16_t height = 0;
        EXPECT_TRUE(appearance->getScissorRegion(x, y, width, height));
        EXPECT_EQ(1, x);
        EXPECT_EQ(2, y);
        EXPECT_EQ(3u, width);
        EXPECT_EQ(4u, height);
    }

    TEST_F(AAppearanceTest, setGetStencilFunc)
    {
        bool stat = appearance->setStencilFunction(EStencilFunc::Equal, 2u, 0xef);
        EXPECT_TRUE(stat);
        EStencilFunc func = EStencilFunc::Disabled;
        uint8_t ref = 0;
        uint8_t mask = 0;
        stat = appearance->getStencilFunction(func, ref, mask);
        EXPECT_TRUE(stat);
        EXPECT_EQ(EStencilFunc::Equal, func);
        EXPECT_EQ(2u, ref);
        EXPECT_EQ(0xef, mask);
    }

    TEST_F(AAppearanceTest, setGetStencilOperation)
    {
        bool stat = appearance->setStencilOperation(EStencilOperation::Decrement, EStencilOperation::Increment, EStencilOperation::DecrementWrap);
        EXPECT_TRUE(stat);
        EStencilOperation sfail = EStencilOperation::Zero;
        EStencilOperation dpfail = EStencilOperation::Zero;
        EStencilOperation dppass = EStencilOperation::Zero;
        stat = appearance->getStencilOperation(sfail, dpfail, dppass);
        EXPECT_TRUE(stat);
        EXPECT_EQ(EStencilOperation::Decrement, sfail);
        EXPECT_EQ(EStencilOperation::Increment, dpfail);
        EXPECT_EQ(EStencilOperation::DecrementWrap, dppass);
    }

    TEST_F(AAppearanceTest, setGetBlendOperations)
    {
        EXPECT_TRUE(appearance->setBlendingOperations(EBlendOperation::Subtract, EBlendOperation::Max));
        EBlendOperation opColor = EBlendOperation::Disabled;
        EBlendOperation opAlpha = EBlendOperation::Disabled;
        EXPECT_TRUE(appearance->getBlendingOperations(opColor, opAlpha));
        EXPECT_EQ(EBlendOperation::Subtract, opColor);
        EXPECT_EQ(EBlendOperation::Max, opAlpha);
    }

    TEST_F(AAppearanceTest, setGetCullMode)
    {
        ECullMode mode = ECullMode::Disabled;
        EXPECT_TRUE(appearance->setCullingMode(ECullMode::FrontFacing));
        EXPECT_TRUE(appearance->getCullingMode(mode));
        EXPECT_EQ(ECullMode::FrontFacing, mode);
        EXPECT_TRUE(appearance->setCullingMode(ECullMode::Disabled));
        EXPECT_TRUE(appearance->getCullingMode(mode));
        EXPECT_EQ(ECullMode::Disabled, mode);
    }

    TEST_F(AAppearanceTest, hasDrawModeTrianglesByDefault)
    {
        EDrawMode mode;
        EXPECT_TRUE(appearance->getDrawMode(mode));
        EXPECT_EQ(EDrawMode::Triangles, mode);
    }

    TEST_F(AAppearanceTest, setGetDrawMode)
    {
        EDrawMode mode = EDrawMode::Lines;
        EXPECT_TRUE(appearance->setDrawMode(EDrawMode::Points));
        EXPECT_TRUE(appearance->getDrawMode(mode));
        EXPECT_EQ(EDrawMode::Points, mode);
    }

    TEST_F(AAppearanceTest, setGetColorWriteMask)
    {
        bool writeR = false;
        bool writeG = false;
        bool writeB = false;
        bool writeA = false;
        EXPECT_TRUE(appearance->setColorWriteMask(true, false, true, false));
        EXPECT_TRUE(appearance->getColorWriteMask(writeR, writeG, writeB, writeA));
        EXPECT_TRUE(writeR);
        EXPECT_FALSE(writeG);
        EXPECT_TRUE(writeB);
        EXPECT_FALSE(writeA);
        EXPECT_TRUE(appearance->setColorWriteMask(false, true, false, true));
        EXPECT_TRUE(appearance->getColorWriteMask(writeR, writeG, writeB, writeA));
        EXPECT_FALSE(writeR);
        EXPECT_TRUE(writeG);
        EXPECT_FALSE(writeB);
        EXPECT_TRUE(writeA);
    }

    TEST_F(AAppearanceTest, reportsErrorWhenGetSetMismatchingInputTypeScalar)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("integerInput");
        ASSERT_TRUE(optUniform.has_value());

        const float value = 42;
        float getValue = 0;

        EXPECT_FALSE(appearance->setInputValue(*optUniform, value));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, getValue));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenGetSetMismatchingInputTypeArray)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("integerInput");
        ASSERT_TRUE(optUniform.has_value());

        const float values[] = { 42, 43, 44, 45, 46, 47 };
        float getValues[6];

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 6u, values));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 6u, getValues));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenGetSetMismatchingInputTypeTexture)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("integerInput");
        ASSERT_TRUE(optUniform.has_value());

        const uint8_t texData[] = { 1, 2, 3 };
        const MipLevelData mipData(3u, texData);

        Texture2D* texture = sharedTestState->getScene().createTexture2D(ETextureFormat::RGB8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(texture != nullptr);
        ramses::TextureSampler* textureSampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_FALSE(appearance->setInputTexture(*optUniform, *textureSampler));

        EXPECT_TRUE(sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenSetInputTextureFromADifferentScene)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("texture2dInput");
        ASSERT_TRUE(optUniform.has_value());

        const uint8_t texData[] = { 1, 2, 3 };
        const MipLevelData mipData(3u, texData);

        ramses::Scene& anotherScene = *sharedTestState->getClient().createScene(sceneId_t(1u));
        Texture2D* texture = anotherScene.createTexture2D(ETextureFormat::RGB8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(texture != nullptr);
        ramses::TextureSampler* textureSampler = anotherScene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_FALSE(appearance->setInputTexture(*optUniform, *textureSampler));

        EXPECT_TRUE(anotherScene.destroy(*texture));
        EXPECT_TRUE(sharedTestState->getClient().destroy(anotherScene));
    }

    TEST_F(AAppearanceTest, getsSamplerSetToUniformInput)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("texture2dInput");
        ASSERT_TRUE(optUniform.has_value());

        const uint8_t texData[] = { 1, 2, 3 };
        const MipLevelData mipData(3u, texData);
        Texture2D* texture = sharedTestState->getScene().createTexture2D(ETextureFormat::RGB8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(texture != nullptr);
        ramses::TextureSampler* textureSampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_TRUE(appearance->setInputTexture(*optUniform, *textureSampler));

        const ramses::TextureSampler* actualSampler = nullptr;
        EXPECT_TRUE(appearance->getInputTexture(*optUniform, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_TRUE(sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture));
    }

    TEST_F(AAppearanceTest, getsNullSamplerIfNoneSetToUniformInput)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("texture2dInput");
        ASSERT_TRUE(optUniform.has_value());

        const ramses::TextureSampler* actualSampler = nullptr;
        EXPECT_TRUE(appearance->getInputTexture(*optUniform, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    TEST_F(AAppearanceTest, getsNullSamplerMSIfNoneSetToUniformInput)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("texture2dMSInput");
        ASSERT_TRUE(optUniform.has_value());

        const TextureSamplerMS* actualSampler = nullptr;
        EXPECT_TRUE(appearance->getInputTextureMS(*optUniform, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    TEST_F(AAppearanceTest, getsNullSamplerExternalIfNoneSetToUniformInput)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("textureExternalInput");
        ASSERT_TRUE(optUniform.has_value());

        const TextureSamplerExternal* actualSampler = nullptr;
        EXPECT_TRUE(appearance->getInputTextureExternal(*optUniform, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    TEST_F(AAppearanceTest, failsToGetSamplerSetToUniformIfInputHasWrongType)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("integerInput");
        ASSERT_TRUE(optUniform.has_value());

        const ramses::TextureSampler* actualSampler = nullptr;
        EXPECT_FALSE(appearance->getInputTexture(*optUniform, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    TEST_F(AAppearanceTest, failsToGetSamplerMSIfInputHasWrongType)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("texture2dInput");
        ASSERT_TRUE(optUniform.has_value());

        const TextureSamplerMS* actualSampler = nullptr;
        EXPECT_FALSE(appearance->getInputTextureMS(*optUniform, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    TEST_F(AAppearanceTest, failsToGetSamplerExternalIfInputHasWrongType)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("texture2dInput");
        ASSERT_TRUE(optUniform.has_value());

        const TextureSamplerExternal* actualSampler = nullptr;
        EXPECT_FALSE(appearance->getInputTextureExternal(*optUniform, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    /// Bool
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeBool)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("boolInput");
        ASSERT_TRUE(optUniform.has_value());

        bool        value   = true;
        bool&       valueR  = value;
        const bool& valueCR = value;
        auto        valueM  = value;
        EXPECT_TRUE(appearance->setInputValue(*optUniform, value));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueCR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, std::move(valueM)));

        bool getValue = false;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeBoolArray)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("boolInputArray");
        ASSERT_TRUE(optUniform.has_value());

        bool values[]     = {true, false, true};
        bool getValues[3] = {false};

        EXPECT_TRUE(appearance->setInputValue(*optUniform, 3u, values));
        EXPECT_TRUE(appearance->getInputValue(*optUniform, 3u, getValues));
        EXPECT_EQ(values, absl::MakeSpan(getValues));

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 0u, values));
        EXPECT_FALSE(appearance->setInputValue(*optUniform, 11u, values));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 0u, values));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 11u, values));
    }

    /// Int32
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeInt32)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("integerInput");
        ASSERT_TRUE(optUniform.has_value());

        int32_t value = 42;
        int32_t& valueR = value;
        const int32_t& valueCR = value;
        auto valueM = value;
        EXPECT_TRUE(appearance->setInputValue(*optUniform, value));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueCR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, std::move(valueM)));

        int32_t getValue = 0;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeInt32Array)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("integerInputArray");
        ASSERT_TRUE(optUniform.has_value());

        const int32_t value = 42;
        int32_t values[] = { value, value * 2, value * 3 };
        int32_t getValues[3] = { 0 };

        EXPECT_TRUE(appearance->setInputValue(*optUniform, 3u, values));
        EXPECT_TRUE(appearance->getInputValue(*optUniform, 3u, getValues));
        EXPECT_EQ(values, absl::MakeSpan(getValues));

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 0u, values));
        EXPECT_FALSE(appearance->setInputValue(*optUniform, 11u, values));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 0u, values));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 11u, values));
    }

    /// Float
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeFloat)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("floatInput");
        ASSERT_TRUE(optUniform.has_value());

        float value = 42;
        float& valueR = value;
        const float& valueCR = value;
        auto valueM = value;
        EXPECT_TRUE(appearance->setInputValue(*optUniform, value));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueCR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, std::move(valueM)));

        float getValue = 0;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeFloatArray)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("floatInputArray");
        ASSERT_TRUE(optUniform.has_value());

        const float value = 42;
        const float values[] = { value, value * 2, value * 3 };
        float getValues[3] = { 0 };

        EXPECT_TRUE(appearance->setInputValue(*optUniform, 3u, values));
        EXPECT_TRUE(appearance->getInputValue(*optUniform, 3u, getValues));
        EXPECT_EQ(values, absl::MakeSpan(getValues));

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 0u, values));
        EXPECT_FALSE(appearance->setInputValue(*optUniform, 11u, values));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 0u, getValues));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 11u, getValues));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector2i)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec2iInput");
        ASSERT_TRUE(optUniform.has_value());

        vec2i value{ 42, 24 };
        vec2i& valueR = value;
        const vec2i& valueCR = value;
        auto valueM = value;
        EXPECT_TRUE(appearance->setInputValue(*optUniform, value));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueCR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, std::move(valueM)));

        vec2i getValue;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector2iArray)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec2iInputArray");
        ASSERT_TRUE(optUniform.has_value());

        const std::vector<vec2i> values = { vec2i{42, 43}, vec2i{44, 45}, vec2i{46, 47} };
        std::vector<vec2i> getValues(values.size());

        EXPECT_TRUE(appearance->setInputValue(*optUniform, 3u, values.data()));
        EXPECT_TRUE(appearance->getInputValue(*optUniform, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 0u, values.data()));
        EXPECT_FALSE(appearance->setInputValue(*optUniform, 11u, values.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 0u, getValues.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 11u, getValues.data()));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector3i)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec3iInput");
        ASSERT_TRUE(optUniform.has_value());

        vec3i value{ 42, 24, 4422 };
        vec3i& valueR = value;
        const vec3i& valueCR = value;
        auto valueM = value;
        EXPECT_TRUE(appearance->setInputValue(*optUniform, value));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueCR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, std::move(valueM)));

        vec3i getValue;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector3iArray)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec3iInputArray");
        ASSERT_TRUE(optUniform.has_value());

        const std::vector<vec3i> values = { vec3i{42, 43, 444}, vec3i{44, 45, 555}, vec3i{46, 47, 666} };
        std::vector<vec3i> getValues(values.size());

        EXPECT_TRUE(appearance->setInputValue(*optUniform, 3u, values.data()));
        EXPECT_TRUE(appearance->getInputValue(*optUniform, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 0u, values.data()));
        EXPECT_FALSE(appearance->setInputValue(*optUniform, 11u, values.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 0u, getValues.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 11u, getValues.data()));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector4i)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec4iInput");
        ASSERT_TRUE(optUniform.has_value());

        vec4i value{ 42, 24, 44, 22 };
        vec4i& valueR = value;
        const vec4i& valueCR = value;
        auto valueM = value;
        EXPECT_TRUE(appearance->setInputValue(*optUniform, value));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueCR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, std::move(valueM)));

        vec4i getValue;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector4iArray)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec4iInputArray");
        ASSERT_TRUE(optUniform.has_value());

        const std::vector<vec4i> values = { vec4i{42, 43, 444, 555}, vec4i{44, 45, 666, 777}, vec4i{46, 47, 888, 999} };
        std::vector<vec4i> getValues(values.size());

        EXPECT_TRUE(appearance->setInputValue(*optUniform, 3u, values.data()));
        EXPECT_TRUE(appearance->getInputValue(*optUniform, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 0u, values.data()));
        EXPECT_FALSE(appearance->setInputValue(*optUniform, 11u, values.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 0u, getValues.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 11u, getValues.data()));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector2f)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec2fInput");
        ASSERT_TRUE(optUniform.has_value());

        vec2f value{ 42.f, 24.f };
        vec2f& valueR = value;
        const vec2f& valueCR = value;
        auto valueM = value;
        EXPECT_TRUE(appearance->setInputValue(*optUniform, value));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueCR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, std::move(valueM)));

        vec2f getValue;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector2fArray)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec2fInputArray");
        ASSERT_TRUE(optUniform.has_value());

        const std::vector<vec2f> values = { vec2f{42.f, 43.f}, vec2f{44.f, 45.f}, vec2f{46.f, 47.f} };
        std::vector<vec2f> getValues(values.size());

        EXPECT_TRUE(appearance->setInputValue(*optUniform, 3u, values.data()));
        EXPECT_TRUE(appearance->getInputValue(*optUniform, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 0u, values.data()));
        EXPECT_FALSE(appearance->setInputValue(*optUniform, 11u, values.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 0u, getValues.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 11u, getValues.data()));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector3f)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec3fInput");
        ASSERT_TRUE(optUniform.has_value());

        vec3f value{ 42.f, 24.f, 44.f };
        vec3f& valueR = value;
        const vec3f& valueCR = value;
        auto valueM = value;
        EXPECT_TRUE(appearance->setInputValue(*optUniform, value));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueCR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, std::move(valueM)));

        vec3f getValue;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector3fArray)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec3fInputArray");
        ASSERT_TRUE(optUniform.has_value());

        const std::vector<vec3f> values = { vec3f{42.f, 43.f, 444.f}, vec3f{44.f, 45.f, 666.f}, vec3f{46.f, 47.f, 888.f} };
        std::vector<vec3f> getValues(values.size());

        EXPECT_TRUE(appearance->setInputValue(*optUniform, 3u, values.data()));
        EXPECT_TRUE(appearance->getInputValue(*optUniform, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 0u, values.data()));
        EXPECT_FALSE(appearance->setInputValue(*optUniform, 11u, values.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 0u, getValues.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 11u, getValues.data()));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector4f)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec4fInput");
        ASSERT_TRUE(optUniform.has_value());

        vec4f value{ 42.f, 24.f, 44.f, 55.f };
        vec4f& valueR = value;
        const vec4f& valueCR = value;
        auto valueM = value;
        EXPECT_TRUE(appearance->setInputValue(*optUniform, value));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueCR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, std::move(valueM)));

        vec4f getValue;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector4fArray)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec4fInputArray");
        ASSERT_TRUE(optUniform.has_value());

        const std::vector<vec4f> values = { vec4f{42.f, 43.f, 444.f, 555.f}, vec4f{44.f, 45.f, 666.f, 777.f}, vec4f{46.f, 47.f, 888.f, 999.f} };
        std::vector<vec4f> getValues(values.size());

        EXPECT_TRUE(appearance->setInputValue(*optUniform, 3u, values.data()));
        EXPECT_TRUE(appearance->getInputValue(*optUniform, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 0u, values.data()));
        EXPECT_FALSE(appearance->setInputValue(*optUniform, 11u, values.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 0u, getValues.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 11u, getValues.data()));
    }

    /// matrix22f
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix22f)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("matrix22fInput");
        ASSERT_TRUE(optUniform.has_value());

        matrix22f value{ 42.f, 43.f, 44.f, 45.f };
        matrix22f& valueR = value;
        const matrix22f& valueCR = value;
        auto valueM = value;
        EXPECT_TRUE(appearance->setInputValue(*optUniform, value));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueCR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, std::move(valueM)));

        matrix22f getValue;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix22fArray)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("matrix22fInputArray");
        ASSERT_TRUE(optUniform.has_value());

        const std::vector<matrix22f> values =
        {
            matrix22f{42.f, 43.f, 44.f, 45.f},
            matrix22f{46.f, 47.f, 48.f, 49.f},
            matrix22f{50.f, 51.f, 52.f, 53.f}
        };
        std::vector<matrix22f> getValues(values.size());

        EXPECT_TRUE(appearance->setInputValue(*optUniform, 3u, values.data()));
        EXPECT_TRUE(appearance->getInputValue(*optUniform, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 0u, values.data()));
        EXPECT_FALSE(appearance->setInputValue(*optUniform, 11u, values.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 0u, getValues.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 11u, getValues.data()));
    }

    /// Matrix33f
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix33f)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("matrix33fInput");
        ASSERT_TRUE(optUniform.has_value());

        matrix33f value{ 42.f, 43.f, 44.f, 45.f, 46.f, 47.f, 48.f, 49.f, 50.f };
        matrix33f& valueR = value;
        const matrix33f& valueCR = value;
        auto valueM = value;
        EXPECT_TRUE(appearance->setInputValue(*optUniform, value));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueCR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, std::move(valueM)));

        matrix33f getValue;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix33fArray)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("matrix33fInputArray");
        ASSERT_TRUE(optUniform.has_value());

        const std::vector<matrix33f> values =
        {
            matrix33f{42.f, 43.f, 44.f, 45.f, 11.f, 22.f, 33.f, 44.f, 55.f},
            matrix33f{46.f, 47.f, 48.f, 49.f, 66.f, 77.f, 88.f, 99.f, 10.f},
            matrix33f{50.f, 51.f, 52.f, 53.f, 20.f, 30.f, 40.f, 50.f, 60.f}
        };
        std::vector<matrix33f> getValues(values.size());

        EXPECT_TRUE(appearance->setInputValue(*optUniform, 3u, values.data()));
        EXPECT_TRUE(appearance->getInputValue(*optUniform, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 0u, values.data()));
        EXPECT_FALSE(appearance->setInputValue(*optUniform, 11u, values.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 0u, getValues.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 11u, getValues.data()));
    }

    /// Matrix44f
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix44f)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("matrix44fInput");
        ASSERT_TRUE(optUniform.has_value());

        matrix44f value{ 42.f, 43.f, 44.f, 45.f, 46.f, 47.f, 48.f, 49.f, 50.f, 51.f, 52.f, 53.f, 54.f, 55.f, 56.f, 57.f };
        matrix44f& valueR = value;
        const matrix44f& valueCR = value;
        auto valueM = value;
        EXPECT_TRUE(appearance->setInputValue(*optUniform, value));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, valueCR));
        EXPECT_TRUE(appearance->setInputValue(*optUniform, std::move(valueM)));

        matrix44f getValue;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix44fArray)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("matrix44fInputArray");
        ASSERT_TRUE(optUniform.has_value());

        const std::vector<matrix44f> values =
        {
            matrix44f{42.f, 43.f, 44.f, 45.f, 11.f, 22.f, 33.f, 44.f, 55.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f},
            matrix44f{46.f, 47.f, 48.f, 49.f, 66.f, 77.f, 88.f, 99.f, 10.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f},
            matrix44f{50.f, 51.f, 52.f, 53.f, 20.f, 30.f, 40.f, 50.f, 60.f, 15.f, 16.f, 17.f, 18.f, 19.f, 20.f, 21.f}
        };
        std::vector<matrix44f> getValues(values.size());

        EXPECT_TRUE(appearance->setInputValue(*optUniform, 3u, values.data()));
        EXPECT_TRUE(appearance->getInputValue(*optUniform, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_FALSE(appearance->setInputValue(*optUniform, 0u, values.data()));
        EXPECT_FALSE(appearance->setInputValue(*optUniform, 11u, values.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 0u, getValues.data()));
        EXPECT_FALSE(appearance->getInputValue(*optUniform, 11u, getValues.data()));
    }

    /// Texture2D
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeTexture2D)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("texture2dInput");
        ASSERT_TRUE(optUniform.has_value());

        const uint8_t texData[] = { 1, 2, 3 };
        const MipLevelData mipData(3u, texData);

        Texture2D* texture = sharedTestState->getScene().createTexture2D(ETextureFormat::RGB8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(texture != nullptr);
        ramses::TextureSampler* textureSampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_TRUE(appearance->setInputTexture(*optUniform, *textureSampler));

        const ramses::TextureSampler* actualSampler = nullptr;
        EXPECT_TRUE(appearance->getInputTexture(*optUniform, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_TRUE(sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture));
    }

    /// Texture2DMS
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeTexture2DMS)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("texture2dMSInput");
        ASSERT_TRUE(optUniform.has_value());

        ramses::RenderBuffer* renderBuffer = sharedTestState->getScene().createRenderBuffer(2u, 2u, ERenderBufferFormat::RGB8, ERenderBufferAccessMode::ReadWrite, 4u);
        TextureSamplerMS* textureSampler = sharedTestState->getScene().createTextureSamplerMS(*renderBuffer, "renderBuffer");
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_EQ(textureSampler->impl().getTextureDataType(), ramses::internal::EDataType::TextureSampler2DMS);
        EXPECT_TRUE(appearance->setInputTexture(*optUniform, *textureSampler));

        const TextureSamplerMS* actualSampler = nullptr;
        EXPECT_TRUE(appearance->getInputTextureMS(*optUniform, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_TRUE(sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*renderBuffer));
    }

    /// TextureCube
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeTextureCube)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("textureCubeInput");
        ASSERT_TRUE(optUniform.has_value());

        const std::byte texData[] = { std::byte{1}, std::byte{2}, std::byte{3} };
        const CubeMipLevelData mipData(3u, texData, texData, texData, texData, texData, texData);

        TextureCube* texture = sharedTestState->getScene().createTextureCube(ETextureFormat::RGB8, 1u, 1u, &mipData, false);
        ASSERT_TRUE(texture != nullptr);
        ramses::TextureSampler* textureSampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_TRUE(appearance->setInputTexture(*optUniform, *textureSampler));

        const ramses::TextureSampler* actualSampler = nullptr;
        EXPECT_TRUE(appearance->getInputTexture(*optUniform, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_TRUE(sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture));
    }

    /// TextureExternal
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeTextureExternal)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("textureExternalInput");
        ASSERT_TRUE(optUniform.has_value());

        TextureSamplerExternal* textureSampler = sharedTestState->getScene().createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear);
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_EQ(textureSampler->impl().getTextureDataType(), ramses::internal::EDataType::TextureSamplerExternal);
        EXPECT_TRUE(appearance->setInputTexture(*optUniform, *textureSampler));

        const TextureSamplerExternal* actualSampler = nullptr;
        EXPECT_TRUE(appearance->getInputTextureExternal(*optUniform, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_TRUE(sharedTestState->getScene().destroy(*textureSampler));
    }

    /// Binding data objects
    TEST_F(AAppearanceTest, uniformInputIsNotBoundInitially)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("floatInput");
        ASSERT_TRUE(optUniform.has_value());
        EXPECT_FALSE(appearance->isInputBound(*optUniform));
    }

    TEST_F(AAppearanceTest, canBindDataObjectToUniformInput)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("floatInput");
        ASSERT_TRUE(optUniform.has_value());

        auto dataObject = sharedTestState->getScene().createDataObject(ramses::EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_TRUE(appearance->bindInput(*optUniform, *dataObject));
        EXPECT_TRUE(appearance->isInputBound(*optUniform));
        EXPECT_EQ(dataObject->impl().getDataReference(), appearance->getDataObjectBoundToInput(*optUniform)->impl().getDataReference());

        EXPECT_TRUE(appearance->unbindInput(*optUniform));
        EXPECT_FALSE(appearance->isInputBound(*optUniform));
        EXPECT_EQ(nullptr, appearance->getDataObjectBoundToInput(*optUniform));
    }

    TEST_F(AAppearanceTest, failsToSetOrGetValueIfInputBound)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("floatInput");
        ASSERT_TRUE(optUniform.has_value());
        EXPECT_TRUE(appearance->setInputValue(*optUniform, 666.f));

        auto dataObject = sharedTestState->getScene().createDataObject(ramses::EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);
        dataObject->setValue(333.f);

        EXPECT_TRUE(appearance->bindInput(*optUniform, *dataObject));

        const float setValue = 0.111f;
        float value = 0.f;
        EXPECT_FALSE(appearance->setInputValue(*optUniform, setValue));
        EXPECT_TRUE(dataObject->getValue(value));
        EXPECT_FLOAT_EQ(333.f, value); // failed setter does not modify data object
        value = 0.f;
        EXPECT_FALSE(appearance->getInputValue(*optUniform, value));
        EXPECT_FLOAT_EQ(0.f, value); // failed getter does not modify out parameter

        EXPECT_TRUE(appearance->unbindInput(*optUniform));

        EXPECT_TRUE(appearance->getInputValue(*optUniform, value));
        EXPECT_EQ(666.f, value); // failed setter did not modify previously set value
    }

    TEST_F(AAppearanceTest, failsToBindDataObjectToArrayUniformInput)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("floatInputArray");
        ASSERT_TRUE(optUniform.has_value());

        auto dataObject = sharedTestState->getScene().createDataObject(ramses::EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_FALSE(appearance->bindInput(*optUniform, *dataObject));
    }

    TEST_F(AAppearanceTest, failsToBindDataObjectFromADifferentScene)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("floatInput");
        ASSERT_TRUE(optUniform.has_value());

        ramses::Scene& anotherScene = *sharedTestState->getClient().createScene(sceneId_t(1u));
        auto dataObject = anotherScene.createDataObject(ramses::EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_FALSE(appearance->bindInput(*optUniform, *dataObject));

        EXPECT_TRUE(sharedTestState->getClient().destroy(anotherScene));
    }

    TEST_F(AAppearanceTest, failsToBindDataObjectOfMismatchingType)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("vec4fInput");
        ASSERT_TRUE(optUniform.has_value());

        auto dataObject = sharedTestState->getScene().createDataObject(ramses::EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_FALSE(appearance->bindInput(*optUniform, *dataObject));
        EXPECT_FALSE(appearance->isInputBound(*optUniform));
    }

    TEST_F(AAppearanceTestWithSemanticUniforms, failsToBindDataObjectIfInputHasSemantics)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("matrix44fInput");
        ASSERT_TRUE(optUniform.has_value());

        auto dataObject = sharedTestState->getScene().createDataObject(ramses::EDataType::Matrix44F);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_FALSE(appearance->bindInput(*optUniform, *dataObject));
    }

    TEST_F(AAppearanceTest, unbindingDataObjectFallsBackToPreviouslySetValue)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("floatInput");
        ASSERT_TRUE(optUniform.has_value());
        EXPECT_TRUE(appearance->setInputValue(*optUniform, 666.f));

        auto dataObject = sharedTestState->getScene().createDataObject(ramses::EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_TRUE(appearance->bindInput(*optUniform, *dataObject));
        dataObject->setValue(13.f);

        EXPECT_TRUE(appearance->unbindInput(*optUniform));
        float value = 0.f;
        EXPECT_TRUE(appearance->getInputValue(*optUniform, value));
        EXPECT_FLOAT_EQ(666.f, value);
    }

    TEST_F(AAppearanceTest, failsToUnbindIfInputIsNotBound)
    {
        const auto optUniform = sharedTestState->effect->findUniformInput("floatInput");
        ASSERT_TRUE(optUniform.has_value());

        EXPECT_FALSE(appearance->unbindInput(*optUniform));
    }

    /// Validation
    TEST_F(AAppearanceTest, reportsErrorWhenValidatedWithInvalidTextureSampler)
    {
        TextureInputInfo texture2DInputInfo;
        GetTexture2DInputInfo(texture2DInputInfo);

        TextureInputInfo texture2DMSInputInfo;
        GetTexture2DMSInputInfo(texture2DMSInputInfo);

        TextureInputInfo texture3DInputInfo;
        GetTexture3DInputInfo(texture3DInputInfo);

        TextureInputInfo textureCubeInputInfo;
        GetTextureCubeInputInfo(textureCubeInputInfo);

        TextureInputInfo textureExternalInfo;
        GetTextureExternalInputInfo(textureExternalInfo);

        Appearance* newAppearance = sharedTestState->getScene().createAppearance(*sharedTestState->effect, "New Appearance");
        ASSERT_TRUE(nullptr != newAppearance);

        EXPECT_TRUE(newAppearance->setInputTexture(*texture2DInputInfo.input, *texture2DInputInfo.sampler));
        EXPECT_TRUE(newAppearance->setInputTexture(*texture2DMSInputInfo.input, *texture2DMSInputInfo.samplerMS));
        EXPECT_TRUE(newAppearance->setInputTexture(*texture3DInputInfo.input, *texture3DInputInfo.sampler));
        EXPECT_TRUE(newAppearance->setInputTexture(*textureCubeInputInfo.input, *textureCubeInputInfo.sampler));
        EXPECT_TRUE(newAppearance->setInputTexture(*textureExternalInfo.input, *textureExternalInfo.samplerExternal));
        ValidationReport report;
        newAppearance->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture2DInputInfo.sampler));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture2DMSInputInfo.samplerMS));
        report.clear();
        newAppearance->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(sharedTestState->getScene().destroy(*newAppearance));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture2DInputInfo.texture2D));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture2DMSInputInfo.renderBuffer));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*textureCubeInputInfo.sampler));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*textureCubeInputInfo.textureCube));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenValidatedWithInvalidTexture)
    {
        TextureInputInfo texture2DInputInfo;
        GetTexture2DInputInfo(texture2DInputInfo);

        TextureInputInfo texture2DMSInputInfo;
        GetTexture2DMSInputInfo(texture2DMSInputInfo);

        TextureInputInfo texture3DInputInfo;
        GetTexture3DInputInfo(texture3DInputInfo);

        TextureInputInfo textureCubeInputInfo;
        GetTextureCubeInputInfo(textureCubeInputInfo);

        TextureInputInfo textureExternalInfo;
        GetTextureExternalInputInfo(textureExternalInfo);

        Appearance* newAppearance = sharedTestState->getScene().createAppearance(*sharedTestState->effect, "New Appearance");
        ASSERT_TRUE(nullptr != newAppearance);
        EXPECT_TRUE(newAppearance->setInputTexture(*texture2DInputInfo.input, *texture2DInputInfo.sampler));
        EXPECT_TRUE(newAppearance->setInputTexture(*texture2DMSInputInfo.input, *texture2DMSInputInfo.samplerMS));
        EXPECT_TRUE(newAppearance->setInputTexture(*texture3DInputInfo.input, *texture3DInputInfo.sampler));
        EXPECT_TRUE(newAppearance->setInputTexture(*textureCubeInputInfo.input, *textureCubeInputInfo.sampler));
        EXPECT_TRUE(newAppearance->setInputTexture(*textureExternalInfo.input, *textureExternalInfo.samplerExternal));
        ValidationReport report;
        newAppearance->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture2DInputInfo.texture2D));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture2DMSInputInfo.renderBuffer));
        report.clear();
        newAppearance->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(sharedTestState->getScene().destroy(*newAppearance));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture2DInputInfo.sampler));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture2DMSInputInfo.samplerMS));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*textureCubeInputInfo.sampler));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*textureCubeInputInfo.textureCube));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenValidatedWithBoundDataObjectThatWasDestroyed)
    {
        TextureInputInfo texture2DInputInfo;
        GetTexture2DInputInfo(texture2DInputInfo);

        TextureInputInfo texture2DMSInputInfo;
        GetTexture2DMSInputInfo(texture2DMSInputInfo);

        TextureInputInfo texture3DInputInfo;
        GetTexture3DInputInfo(texture3DInputInfo);

        TextureInputInfo textureCubeInputInfo;
        GetTextureCubeInputInfo(textureCubeInputInfo);

        TextureInputInfo textureExternalInfo;
        GetTextureExternalInputInfo(textureExternalInfo);

        Appearance* newAppearance = sharedTestState->getScene().createAppearance(*sharedTestState->effect, "New Appearance");
        ASSERT_TRUE(nullptr != newAppearance);
        EXPECT_TRUE(newAppearance->setInputTexture(*texture2DInputInfo.input, *texture2DInputInfo.sampler));
        EXPECT_TRUE(newAppearance->setInputTexture(*texture2DMSInputInfo.input, *texture2DMSInputInfo.samplerMS));
        EXPECT_TRUE(newAppearance->setInputTexture(*texture3DInputInfo.input, *texture3DInputInfo.sampler));
        EXPECT_TRUE(newAppearance->setInputTexture(*textureCubeInputInfo.input, *textureCubeInputInfo.sampler));
        EXPECT_TRUE(newAppearance->setInputTexture(*textureExternalInfo.input, *textureExternalInfo.samplerExternal));
        ValidationReport report;
        newAppearance->validate(report);
        EXPECT_FALSE(report.hasIssue());

        const auto optUniform = sharedTestState->effect->findUniformInput("floatInput");
        ASSERT_TRUE(optUniform.has_value());

        auto dataObject = sharedTestState->getScene().createDataObject(ramses::EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_TRUE(newAppearance->bindInput(*optUniform, *dataObject));
        report.clear();
        newAppearance->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(sharedTestState->getScene().destroy(*dataObject));
        report.clear();
        newAppearance->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(sharedTestState->getScene().destroy(*newAppearance));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture2DInputInfo.sampler));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture2DInputInfo.texture2D));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture2DMSInputInfo.samplerMS));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*texture2DMSInputInfo.renderBuffer));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*textureCubeInputInfo.sampler));
        EXPECT_TRUE(sharedTestState->getScene().destroy(*textureCubeInputInfo.textureCube));
    }

    TEST_F(AAppearanceTest, failsWhenWrongBlendingOperationsSet)
    {
        bool stat = appearance->setBlendingOperations(EBlendOperation::Subtract, EBlendOperation::ReverseSubtract);
        EXPECT_TRUE(stat);

        stat = appearance->setBlendingOperations(EBlendOperation::Add, EBlendOperation::Disabled);
        EXPECT_FALSE(stat);

        stat = appearance->setBlendingOperations(EBlendOperation::Disabled, EBlendOperation::Add);
        EXPECT_FALSE(stat);

        EBlendOperation opColor = EBlendOperation::Disabled;
        EBlendOperation opAlpha = EBlendOperation::Disabled;
        EXPECT_TRUE(appearance->getBlendingOperations(opColor, opAlpha));
        EXPECT_EQ(EBlendOperation::Subtract, opColor);
        EXPECT_EQ(EBlendOperation::ReverseSubtract, opAlpha);
    }

    TEST_F(AAppearanceTest, defaultBlendingFactors)
    {
        EBlendFactor srcColor  = EBlendFactor::Zero;
        EBlendFactor destColor = EBlendFactor::Zero;
        EBlendFactor srcAlpha  = EBlendFactor::Zero;
        EBlendFactor destAlpha = EBlendFactor::Zero;
        bool stat = appearance->getBlendingFactors(srcColor, destColor, srcAlpha, destAlpha);
        EXPECT_TRUE(stat);
        EXPECT_EQ(EBlendFactor::SrcAlpha, srcColor);
        EXPECT_EQ(EBlendFactor::OneMinusSrcAlpha, destColor);
        EXPECT_EQ(EBlendFactor::One, srcAlpha);
        EXPECT_EQ(EBlendFactor::One, destAlpha);
    }

    TEST_F(AAppearanceTest, defaultDepthWriteMode)
    {
        EDepthWrite depthWriteMode = EDepthWrite::Disabled;
        EXPECT_TRUE(appearance->getDepthWriteMode(depthWriteMode));
        EXPECT_EQ(EDepthWrite::Enabled, depthWriteMode);
    }

    TEST_F(AAppearanceTest, defaultScissorRegion)
    {
        int16_t  x      = std::numeric_limits<int16_t>::max();
        int16_t  y      = std::numeric_limits<int16_t>::max();
        uint16_t width  = std::numeric_limits<uint16_t>::max();
        uint16_t height = std::numeric_limits<uint16_t>::max();
        EXPECT_TRUE(appearance->getScissorRegion(x, y, width, height));
        EXPECT_EQ(0, x);
        EXPECT_EQ(0, y);
        EXPECT_EQ(0u, width);
        EXPECT_EQ(0u, height);
    }

    TEST_F(AAppearanceTest, defaultStencilFunc)
    {
        EStencilFunc func = EStencilFunc::Always;
        uint8_t      ref  = std::numeric_limits<uint8_t>::max();
        uint8_t      mask = std::numeric_limits<uint8_t>::max();
        bool     stat = appearance->getStencilFunction(func, ref, mask);
        EXPECT_TRUE(stat);
        EXPECT_EQ(EStencilFunc::Disabled, func);
        EXPECT_EQ(0u, ref);
        EXPECT_EQ(0xFF, mask);
    }

    TEST_F(AAppearanceTest, defaultBlendingOperations)
    {
        EBlendOperation opColor = EBlendOperation::Add;
        EBlendOperation opAlpha = EBlendOperation::Subtract;
        EXPECT_TRUE(appearance->getBlendingOperations(opColor, opAlpha));
        EXPECT_EQ(EBlendOperation::Disabled, opColor);
        EXPECT_EQ(EBlendOperation::Disabled, opAlpha);
    }

    TEST_F(AAppearanceTest, defaultCullMode)
    {
        ECullMode mode = ECullMode::Disabled;
        EXPECT_TRUE(appearance->getCullingMode(mode));
        EXPECT_EQ(ECullMode::BackFacing, mode);
    }

    TEST_F(AAppearanceTest, defaultColorWriteMask)
    {
        bool writeR = false;
        bool writeG = false;
        bool writeB = false;
        bool writeA = false;
        EXPECT_TRUE(appearance->getColorWriteMask(writeR, writeG, writeB, writeA));
        EXPECT_TRUE(writeR);
        EXPECT_TRUE(writeG);
        EXPECT_TRUE(writeB);
        EXPECT_TRUE(writeA);
    }

    TEST_F(AAppearanceTest, defaultGetInputValue)
    {
        {
            auto intOut             = std::numeric_limits<int32_t>::max();
            auto uniformItegerInput = sharedTestState->effect->findUniformInput("integerInput");
            EXPECT_TRUE(appearance->getInputValue(*uniformItegerInput, intOut));
            EXPECT_EQ(intOut, 0);
        }
        {
            auto floatOut          = std::numeric_limits<float>::max();
            auto uniformFloatInput = sharedTestState->effect->findUniformInput("floatInput");
            EXPECT_TRUE(appearance->getInputValue(*uniformFloatInput, floatOut));
            EXPECT_FLOAT_EQ(floatOut, 0.0f);
        }
        {
            vec2i vec2iOut(42);
            auto  uniformVec2iInput = sharedTestState->effect->findUniformInput("vec2iInput");
            EXPECT_TRUE(appearance->getInputValue(*uniformVec2iInput, vec2iOut));
            EXPECT_EQ(vec2iOut.x, 0);
            EXPECT_EQ(vec2iOut.y, 0);
        }
        {
            vec3i vec3iOut(42);
            auto  uniformVec3iInput = sharedTestState->effect->findUniformInput("vec3iInput");
            EXPECT_TRUE(appearance->getInputValue(*uniformVec3iInput, vec3iOut));
            EXPECT_EQ(vec3iOut, vec3i{0});
        }
        {
            vec4i vec4iOut(42);
            auto  uniformVec4iInput = sharedTestState->effect->findUniformInput("vec4iInput");
            EXPECT_TRUE(appearance->getInputValue(*uniformVec4iInput, vec4iOut));
            EXPECT_EQ(vec4iOut, vec4i{0});
        }
        {
            vec2f vec2fOut(42.f);
            auto  uniformVec2fInput = sharedTestState->effect->findUniformInput("vec2fInput");
            EXPECT_TRUE(appearance->getInputValue(*uniformVec2fInput, vec2fOut));
            EXPECT_EQ(vec2fOut, vec2f{0.f});
        }
        {
            vec3f vec3fOut(42.f);
            auto  uniformVec3fInput = sharedTestState->effect->findUniformInput("vec3fInput");
            EXPECT_TRUE(appearance->getInputValue(*uniformVec3fInput, vec3fOut));
            EXPECT_EQ(vec3fOut, vec3f{0.f});
        }
        {
            vec4f vec4fOut(42.f);
            auto  uniformVec4fInput = sharedTestState->effect->findUniformInput("vec4fInput");
            EXPECT_TRUE(appearance->getInputValue(*uniformVec4fInput, vec4fOut));
            EXPECT_EQ(vec4fOut, vec4f{0.f});
        }
        {
            auto      uniformMatrix22fInput = sharedTestState->effect->findUniformInput("matrix22fInput");
            matrix22f matrix22fOut          = {42, 43, 44, 45};
            EXPECT_TRUE(appearance->getInputValue(*uniformMatrix22fInput, matrix22fOut));
            for (const auto& i : matrix22fOut)
                EXPECT_FLOAT_EQ(i, 0.0f);
        }
        {
            auto      uniformMatrix33fInput = sharedTestState->effect->findUniformInput("matrix33fInput");
            matrix33f matrix33fOut          = {42, 43, 44, 45, 46, 47, 48, 49, 50};
            EXPECT_TRUE(appearance->getInputValue(*uniformMatrix33fInput, matrix33fOut));
            for (const auto& i : matrix33fOut)
                EXPECT_FLOAT_EQ(i, 0.0f);
        }
        {
            auto      uniformMatrix44fInput = sharedTestState->effect->findUniformInput("matrix44fInput");
            matrix44f matrix44fOut          = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57};
            EXPECT_TRUE(appearance->getInputValue(*uniformMatrix44fInput, matrix44fOut));
            for (const auto& i : matrix44fOut)
                EXPECT_FLOAT_EQ(i, 0.0f);
        }

        {
            auto    integerInputArrayInput = sharedTestState->effect->findUniformInput("integerInputArray");
            int32_t integerInputArray[]    = {42, 42, 42};
            EXPECT_TRUE(appearance->getInputValue(*integerInputArrayInput, 3u, integerInputArray));
            for (const auto& i : integerInputArray)
                EXPECT_EQ(i, 0);
        }
        {
            auto  floatInputArrayInput = sharedTestState->effect->findUniformInput("floatInputArray");
            float floatInputArray[]    = {42, 42, 42};
            EXPECT_TRUE(appearance->getInputValue(*floatInputArrayInput, 3u, floatInputArray));
            for (const auto& i : floatInputArray)
                EXPECT_FLOAT_EQ(i, 0.0f);
        }
        {
            auto  vec2iInputArrayInput = sharedTestState->effect->findUniformInput("vec2iInputArray");
            vec2i vec2iInputArray[]    = {vec2i{42, 43}, vec2i{44, 45}, vec2i{46, 47}};
            EXPECT_TRUE(appearance->getInputValue(*vec2iInputArrayInput, 3u, vec2iInputArray));
            for (const auto& i : vec2iInputArray)
                EXPECT_EQ(i, vec2i{0});
        }
        {
            auto  vec3iInputArrayInput = sharedTestState->effect->findUniformInput("vec3iInputArray");
            vec3i vec3iInputArray[]    = {vec3i{42, 43, 44}, vec3i{45, 46, 47}, vec3i{48, 49, 50}};
            EXPECT_TRUE(appearance->getInputValue(*vec3iInputArrayInput, 3u, vec3iInputArray));
            for (const auto& i : vec3iInputArray)
                EXPECT_EQ(i, vec3i{0});
        }
        {
            auto  vec4iInputArrayInput = sharedTestState->effect->findUniformInput("vec4iInputArray");
            vec4i vec4iInputArray[]    = {vec4i{42, 43, 44, 45}, vec4i{46, 47, 48, 49}, vec4i{50, 51, 52, 53}};
            EXPECT_TRUE(appearance->getInputValue(*vec4iInputArrayInput, 3u, vec4iInputArray));
            for (const auto& i : vec4iInputArray)
                EXPECT_EQ(i, vec4i{0});
        }
        {
            auto  vec2fInputArrayInput = sharedTestState->effect->findUniformInput("vec2fInputArray");
            vec2f vec2fInputArray[]    = {vec2f{42, 43}, vec2f{44, 45}, vec2f{46, 47}};
            EXPECT_TRUE(appearance->getInputValue(*vec2fInputArrayInput, 3u, vec2fInputArray));
            for (const auto& i : vec2fInputArray)
                EXPECT_EQ(i, vec2f{0});
        }
        {
            auto  vec3fInputArrayInput = sharedTestState->effect->findUniformInput("vec3fInputArray");
            vec3f vec3fInputArray[]    = {vec3f{42, 43, 44}, vec3f{45, 46, 47}, vec3f{48, 49, 50}};
            EXPECT_TRUE(appearance->getInputValue(*vec3fInputArrayInput, 3u, vec3fInputArray));
            for (const auto& i : vec3fInputArray)
                EXPECT_EQ(i, vec3f{0});
        }
        {
            auto  vec4fInputArrayInput = sharedTestState->effect->findUniformInput("vec4fInputArray");
            vec4f vec4fInputArray[]    = {vec4f{42, 43, 44, 45}, vec4f{46, 47, 48, 49}, vec4f{50, 51, 52, 53}};
            EXPECT_TRUE(appearance->getInputValue(*vec4fInputArrayInput, 3u, vec4fInputArray));
            for (const auto& i : vec4fInputArray)
                EXPECT_EQ(i, vec4f{0});
        }
        {
            auto      matrix22fInputArrayInput = sharedTestState->effect->findUniformInput("matrix22fInputArray");
            matrix22f matrix22fInputArray[3]   = {{42, 43, 44, 45}, {46, 47, 48, 49}, {50, 51, 52, 53}};
            EXPECT_TRUE(appearance->getInputValue(*matrix22fInputArrayInput, 3u, matrix22fInputArray));
            for (const auto& i : matrix22fInputArray)
                EXPECT_EQ(i, matrix22f{0.0f});
        }
        {
            auto      matrix33fInputArrayInput = sharedTestState->effect->findUniformInput("matrix33fInputArray");
            matrix33f matrix33fInputArray[3]   = {{42, 43, 44, 46, 47, 48, 50, 51, 52}, {54, 55, 56, 46, 47, 48, 42, 43, 44}, {54, 55, 56, 46, 47, 48, 50, 51, 52}};
            EXPECT_TRUE(appearance->getInputValue(*matrix33fInputArrayInput, 3u, matrix33fInputArray));
            for (const auto& i : matrix33fInputArray)
                EXPECT_EQ(i, matrix33f{0.0f});
        }
        {
            auto      matrix44fInputArrayInput = sharedTestState->effect->findUniformInput("matrix44fInputArray");
            matrix44f matrix44fInputArray[3]   = {{42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57},
                                                {46, 47, 48, 49, 42, 43, 44, 45, 54, 55, 56, 57, 46, 47, 48, 49},
                                                {50, 51, 52, 53, 54, 55, 56, 57, 42, 43, 44, 45, 50, 51, 52, 53}};
            EXPECT_TRUE(appearance->getInputValue(*matrix44fInputArrayInput, 3u, matrix44fInputArray));
            for (const auto& i : matrix44fInputArray)
                EXPECT_EQ(i, matrix44f{0.0f});
        }
    }

    class AnAppearanceWithGeometryShader : public AAppearanceTest
    {
    public:

        static void SetUpTestSuite()
        {
            sharedTestState = std::make_unique<TestEffectCreator>(false, true);
        }
    };

    TEST_F(AnAppearanceWithGeometryShader, HasInitialDrawModeOfGeometryShadersRequirement)
    {
        EDrawMode mode;
        EXPECT_TRUE(appearance->getDrawMode(mode));
        EXPECT_EQ(EDrawMode::Lines, mode);
    }

    TEST_F(AnAppearanceWithGeometryShader, RefusesToChangeDrawingMode_WhenIncompatibleToGeometryShader)
    {
        // Shader uses lines, can't change to incompatible types
        EXPECT_FALSE(appearance->setDrawMode(EDrawMode::Points));
        EXPECT_FALSE(appearance->setDrawMode(EDrawMode::Triangles));
        EXPECT_FALSE(appearance->setDrawMode(EDrawMode::TriangleFan));
        EDrawMode mode;
        EXPECT_TRUE(appearance->getDrawMode(mode));
        EXPECT_EQ(EDrawMode::Lines, mode);
    }

    TEST_F(AnAppearanceWithGeometryShader, AllowsChangingDrawMode_IfNewModeIsStillCompatible)
    {
        // Shader uses lines, change to line strip is ok - still produces lines for the geometry stage
        EXPECT_TRUE(appearance->setDrawMode(EDrawMode::LineStrip));
        EDrawMode mode;
        EXPECT_TRUE(appearance->getDrawMode(mode));
        EXPECT_EQ(EDrawMode::LineStrip, mode);
    }

    TEST(AnAppearanceUtils, ChecksValidGeometryShaderModes)
    {
        // valid combinations of appearance draw mode (first) and GS input type (second)
        const std::initializer_list<std::pair<EDrawMode, EDrawMode>> validDrawModeToGSModeCombinations = {
            { EDrawMode::Points, EDrawMode::Points },
            { EDrawMode::Lines, EDrawMode::Lines },
            { EDrawMode::LineStrip, EDrawMode::Lines },
            { EDrawMode::LineLoop, EDrawMode::Lines },
            { EDrawMode::Triangles, EDrawMode::Triangles },
            { EDrawMode::TriangleStrip, EDrawMode::Triangles },
            { EDrawMode::TriangleFan, EDrawMode::Triangles }
        };

        for (int drawModeVal = 0; drawModeVal <= static_cast<int>(EDrawMode::LineStrip); ++drawModeVal)
        {
            for (int gsInputTypeVal = 0; gsInputTypeVal <= static_cast<int>(EDrawMode::LineStrip); ++gsInputTypeVal)
            {
                const auto drawMode = static_cast<EDrawMode>(drawModeVal);
                const auto gsInputType = static_cast<EDrawMode>(gsInputTypeVal);

                // gs input mode restricted to only these types
                if (gsInputType != EDrawMode::Points && gsInputType != EDrawMode::Lines && gsInputType != EDrawMode::Triangles)
                    continue;

                const auto it = std::find_if(validDrawModeToGSModeCombinations.begin(), validDrawModeToGSModeCombinations.end(), [&](const auto& modePair) {
                    return modePair.first == drawMode && modePair.second == gsInputType;
                    });
                const bool isValid = (it != validDrawModeToGSModeCombinations.end());

                EXPECT_EQ(isValid, AppearanceUtils::GeometryShaderCompatibleWithDrawMode(gsInputType, drawMode));
            }
        }
    }
}
