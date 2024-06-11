//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/renderer/DisplayConfig.h"
#include "ramses/framework/RamsesFramework.h"
#include "ramses/framework/RamsesFrameworkTypes.h"

#include "impl/DisplayConfigImpl.h"
#include "impl/RendererConfigImpl.h"
#include "RendererTestUtils.h"
#include "WindowEventHandlerMock.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/SceneGraph/Resource/ResourceTypes.h"
#include "internal/RendererLib/PlatformInterface/IRenderBackend.h"
#include "internal/RendererLib/PlatformBase/Device_Base.h"
#include "internal/RendererLib/PlatformInterface/IContext.h"
#include "internal/RendererLib/PlatformBase/DeviceResourceMapper.h"
#include "internal/Platform/PlatformFactory.h"
#include "internal/Platform/OpenGL/ShaderGPUResource_GL.h"
#include <memory>
#include <vector>

using namespace testing;

namespace ramses::internal
{
    class ADevice : public Test
    {
    public:
        void SetUp() override
        {
            if (displayConfig.getDeviceType() == EDeviceType::Vulkan)
                GTEST_SKIP() << "Shader upload tests do not support device type vulkan";

            // Need to "read" the error state in Device_GL once, in order to reset the error state to NO_ERROR
            // This is needed because all tests share the same device to save time
            std::ignore = testDevice->isDeviceStatusHealthy();
        }

        static EffectResource* CreateTestEffectResource()
        {
            const std::string vertexShader(
                "#version 100                            \n"
                "                                        \n"
                "attribute vec3 a_position;              \n"
                "varying vec3 v_position;                \n"
                "                                        \n"
                "void main()                             \n"
                "{                                       \n"
                "    gl_Position = vec4(a_position, 1.0);\n"
                "    v_position = a_position;            \n"
                "}                                       \n"
            );

            const std::string fragmentShader(
                "#version 100                                                     \n"
                "                                                                 \n"
                "varying highp vec3 v_position;                                   \n"
                "                                                                 \n"
                "uniform highp float u_float;                                     \n"
                "uniform highp vec2 u_vec2;                                       \n"
                "                                                                 \n"
                "void main(void)                                                  \n"
                "{                                                                \n"
                "    gl_FragColor = vec4(v_position + vec3(u_vec2, 1.0), u_float);\n"
                "}                                                                \n"
            );

            EffectInputInformationVector uniformInputs;
            uniformInputs.push_back(EffectInputInformation("u_float", 1, EDataType::Float, EFixedSemantics::Invalid));
            uniformInputs.push_back(EffectInputInformation("u_vec2", 1, EDataType::Vector2F, EFixedSemantics::Invalid));

            EffectInputInformationVector attributeInputs;
            attributeInputs.push_back(EffectInputInformation("a_position", 1, EDataType::Vector3F, EFixedSemantics::Invalid));

            return new EffectResource(vertexShader, fragmentShader, "", SPIRVShaders{}, {}, uniformInputs, attributeInputs, "test effect", EFeatureLevel_Latest);
        }

        static void SetUpTestSuite()
        {
            assert(nullptr == platform);

            ramses::RendererConfig rendererConfig = RendererTestUtils::CreateTestRendererConfig();
            displayConfig = RendererTestUtils::CreateTestDisplayConfig(0u);
            if (displayConfig.getDeviceType() == EDeviceType::Vulkan)
                return;

            PlatformFactory platformFactory;
            platform = platformFactory.createPlatform(rendererConfig.impl().getInternalRendererConfig(), displayConfig.impl().getInternalDisplayConfig());
            assert(nullptr != platform);

            eventHandler = std::make_unique<NiceMock<WindowEventHandlerMock>>();

            displayConfig.setWindowRectangle(0, 0, 16u, 16u);
            renderBackend = platform->createRenderBackend(displayConfig.impl().getInternalDisplayConfig(), *eventHandler);
            assert(nullptr != renderBackend);

            testDevice = &renderBackend->getDevice();
        }

        static void TearDownTestSuite()
        {
            if (platform)
            {
                testDevice = nullptr;

                platform->destroyRenderBackend();
                renderBackend = nullptr;

                platform.reset();
                eventHandler.reset();
            }
        }

    protected:
        static IDevice*                             testDevice;
        static IRenderBackend*                      renderBackend;

    private:
        static std::unique_ptr<IPlatform>           platform;
        static std::unique_ptr<NiceMock<WindowEventHandlerMock>> eventHandler;
        static ramses::DisplayConfig                displayConfig;
    };

    std::unique_ptr<IPlatform>          ADevice::platform                    ;
    IDevice*                            ADevice::testDevice         = nullptr;
    IRenderBackend*                     ADevice::renderBackend      = nullptr;
    std::unique_ptr<NiceMock<WindowEventHandlerMock>> ADevice::eventHandler = nullptr;
    ramses::DisplayConfig               ADevice::displayConfig;


    // Needed so that these tests can be blacklisted on drivers which don't support binary shaders
    class ADeviceSupportingBinaryShaders : public ADevice
    {
    };

    // Needed so that these tests can be blacklisted on drivers which don't support geometry shaders
    class ADeviceSupportingGeometryShaders : public ADevice
    {
    public:
        static EffectResource* CreateTestEffectResourceWithGeometryShader()
        {
            const std::string vertexShader(R"SHADER(
                #version 320 es

                in highp vec3 a_position;
                out highp vec3 v_position;

                void main()
                {
                    gl_Position = vec4(a_position, 1.0);
                    v_position = a_position;
                }
                )SHADER");

            const std::string fragmentShader(R"SHADER(
                #version 320 es

                in highp vec3 g_position;
                out highp vec4 color;

                uniform highp float u_float;
                uniform highp vec2 u_vec2;

                void main(void)
                {
                    color = vec4(g_position + vec3(u_vec2, 1.0), u_float);
                }
                )SHADER");

            const std::string geometryShader(R"SHADER(
                #version 320 es

                layout(points) in;
                layout(points, max_vertices = 1) out;
                uniform highp float u_geomFloat;

                in highp vec3 v_position[];
                out highp vec3 g_position;

                void main() {

                    gl_Position = vec4(u_geomFloat);
                    g_position = v_position[0];
                    EmitVertex();
                    EndPrimitive();
                }
                )SHADER");
            EffectInputInformationVector uniformInputs;
            uniformInputs.push_back(EffectInputInformation("u_float", 1, EDataType::Float, EFixedSemantics::Invalid));
            uniformInputs.push_back(EffectInputInformation("u_vec2", 1, EDataType::Vector2F, EFixedSemantics::Invalid));
            uniformInputs.push_back(EffectInputInformation("u_geomFloat", 1, EDataType::Float, EFixedSemantics::Invalid));

            EffectInputInformationVector attributeInputs;
            attributeInputs.push_back(EffectInputInformation("a_position", 1, EDataType::Vector3F, EFixedSemantics::Invalid));

            return new EffectResource(vertexShader, fragmentShader, geometryShader, SPIRVShaders{}, EDrawMode::Points, uniformInputs, attributeInputs, "test effect", EFeatureLevel_Latest);
        }
    };

    TEST_F(ADevice, CreatesShaderFromEffect)
    {
        ASSERT_TRUE(testDevice != nullptr);

        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResource());
        auto shaderGpuResource = testDevice->uploadShader(*testEffect);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        EXPECT_TRUE(handle.isValid());

        testDevice->deleteShader(handle);
    }

    TEST_F(ADevice, RefusesToUploadEffectWithInvalidVertexShader)
    {
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());
        const EffectResource invalidEffect(
            "--this is some invalid shader source code--",
            templateEffect->getFragmentShader(),
            "", {}, {},
            templateEffect->getUniformInputs(),
            templateEffect->getAttributeInputs(),
            "invalid effect", EFeatureLevel_Latest);
        const auto shaderGpuResource = testDevice->uploadShader(invalidEffect);
        EXPECT_EQ(nullptr, shaderGpuResource);
    }

    TEST_F(ADevice, RefusesToUploadEffectWithInvalidFragmentShader)
    {
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());
        const EffectResource invalidEffect(
            templateEffect->getVertexShader(),
            "--this is some invalid shader source code--",
            "", SPIRVShaders{}, {},
            templateEffect->getUniformInputs(),
            templateEffect->getAttributeInputs(),
            "invalid effect", EFeatureLevel_Latest);
        const auto shaderGpuResource = testDevice->uploadShader(invalidEffect);
        EXPECT_EQ(nullptr, shaderGpuResource);
    }

    TEST_F(ADevice, RefusesToUploadEffectWithUnlinkableShaders)
    {
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());
        const std::string fragmentShader(
            "#version 100                                                     \n"
            "                                                                 \n"
            "varying highp float v_position; /* interface mismatch */         \n"
            "                                                                 \n"
            "uniform highp float u_float;                                     \n"
            "uniform highp vec2 u_vec2;                                       \n"
            "                                                                 \n"
            "void main(void)                                                  \n"
            "{                                                                \n"
            "    gl_FragColor = vec4(vec3(v_position) + vec3(u_vec2, 1.0), u_float);\n"
            "}                                                                \n"
            );
        const EffectResource invalidEffect(
            templateEffect->getVertexShader(),
            fragmentShader,
            "", SPIRVShaders{}, {},
            templateEffect->getUniformInputs(),
            templateEffect->getAttributeInputs(),
            "invalid effect", EFeatureLevel_Latest);
        const auto shaderGpuResource = testDevice->uploadShader(invalidEffect);
        EXPECT_EQ(nullptr, shaderGpuResource);
    }

    TEST_F(ADevice, SetsConstantsForAllPossibleDataTypesOnShader)
    {
        ASSERT_TRUE(testDevice != nullptr);

        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());
        const std::string fragmentShader(R"(
            #version 100

            uniform bool u_bool[2];
            uniform highp int u_int[2];
            uniform highp float u_float[2];
            uniform highp vec2 u_vec2[2];
            uniform highp vec3 u_vec3[2];
            uniform highp vec4 u_vec4[2];
            uniform highp ivec2 u_ivec2[2];
            uniform highp ivec3 u_ivec3[2];
            uniform highp ivec4 u_ivec4[2];
            uniform highp mat2 u_mat2[2];
            uniform highp mat3 u_mat3[2];
            uniform highp mat4 u_mat4[2];

            void main(void)
            {
                gl_FragColor.r = u_float[1] * float(u_int[1]);
                gl_FragColor.g = u_mat2[1][0][0] * u_vec2[1].x * float(u_ivec2[1].x);
                gl_FragColor.b = u_mat3[1][0][0] * u_vec3[1].x * float(u_ivec3[1].x);
                if (u_bool[0])
                    gl_FragColor.g = u_mat4[1][0][0] * u_vec4[1].x * float(u_ivec4[1].x);
            }
            )");
        EffectInputInformationVector uniformInputs;
        uniformInputs.push_back(EffectInputInformation("u_bool", 2, EDataType::Bool, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_int", 2, EDataType::Int32, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_float", 2, EDataType::Float, EFixedSemantics::Invalid));

        uniformInputs.push_back(EffectInputInformation("u_vec2", 2, EDataType::Vector2F, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_vec3", 2, EDataType::Vector3F, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_vec4", 2, EDataType::Vector4F, EFixedSemantics::Invalid));

        uniformInputs.push_back(EffectInputInformation("u_ivec2", 2, EDataType::Vector2I, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_ivec3", 2, EDataType::Vector3I, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_ivec4", 2, EDataType::Vector4I, EFixedSemantics::Invalid));

        uniformInputs.push_back(EffectInputInformation("u_mat2", 2, EDataType::Matrix22F, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_mat3", 2, EDataType::Matrix33F, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_mat4", 2, EDataType::Matrix44F, EFixedSemantics::Invalid));

        const EffectResource testEffect(
            templateEffect->getVertexShader(),
            fragmentShader,
            "", SPIRVShaders{}, {},
            uniformInputs,
            templateEffect->getAttributeInputs(),
            "uniform test effect", EFeatureLevel_Latest);
        auto shaderGpuResource = testDevice->uploadShader(testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        ASSERT_TRUE(handle.isValid());

        testDevice->activateShader(handle);

        const std::array<bool, 2> boolValues{true, true};
        const std::array<int32_t, 2> intValues{ 5, 5 };
        const std::array<float, 2> floatValues{ 5.f, 5.f };
        const std::array<glm::vec2, 2> vec2Values{ glm::vec2{5.f}, glm::vec2{5.f} };
        const std::array<glm::vec3, 2> vec3Values{ glm::vec3{5.f}, glm::vec3{5.f} };
        const std::array<glm::vec4, 2> vec4Values{ glm::vec4{5.f}, glm::vec4{5.f} };
        const std::array<glm::ivec2, 2> vec2iValues{ glm::ivec2{5}, glm::ivec2{5} };
        const std::array<glm::ivec3, 2> vec3iValues{ glm::ivec3{5}, glm::ivec3{5} };
        const std::array<glm::ivec4, 2> vec4iValues{ glm::ivec4{5}, glm::ivec4{5} };
        const std::array<glm::mat2, 2> mat22Values{ glm::mat2{1.0f, 2.0f, 3.0f, 4.0f}, glm::mat2{1.0f, 2.0f, 3.0f, 4.0f} };
        const std::array<glm::mat3, 2> mat33Values{ glm::mat3{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f}, glm::mat3{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f} };
        const std::array<glm::mat4, 2> mat44Values{ glm::mat4{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f}, glm::mat4{} };

        EXPECT_TRUE(testDevice->isDeviceStatusHealthy()); // check that there were no errors (GL_NO_ERROR in case of OpenGL) - triggers an internal assert otherwise
        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_bool")), 2, boolValues.data()));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_int")), 2, intValues.data()));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_float")), 2, floatValues.data()));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());

        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_vec2")), 2, vec2Values.data()));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_vec3")), 2, vec3Values.data()));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_vec4")), 2, vec4Values.data()));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());

        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_ivec2")), 2, vec2iValues.data()));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_ivec3")), 2, vec3iValues.data()));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_ivec4")), 2, vec4iValues.data()));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());

        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_mat2")), 2, mat22Values.data()));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_mat3")), 2, mat33Values.data()));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_mat4")), 2, mat44Values.data()));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());

        testDevice->deleteShader(handle);
    }

    TEST_F(ADevice, IsHealthyAfterSettingNonExistingUniformOnShader)
    {
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());

        EffectInputInformationVector uniformInputs = templateEffect->getUniformInputs();
        uniformInputs.push_back(EffectInputInformation("u_NonExistingFloat", 1, EDataType::Float, EFixedSemantics::Invalid));

        const EffectResource testEffect(
            templateEffect->getVertexShader(),
            templateEffect->getFragmentShader(),
            "", SPIRVShaders{}, {},
            uniformInputs,
            templateEffect->getAttributeInputs(),
            "test effect", EFeatureLevel_Latest);

        auto shaderGpuResource = testDevice->uploadShader(testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        EXPECT_TRUE(handle.isValid());

        testDevice->activateShader(handle);

        const float floatValue = 5.0f;

        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        EXPECT_FALSE(testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_NonExistingFloat")), 1, &floatValue));

        // Currently, RAMSES silently ignores (== does not try to set) uniforms which are not declared in the shader
        // This test verifies that behavior (i.e. device does not crash or report error)
        // Should RAMSES become more strict and don't check uniforms existence, this EXPECTation should be false
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());

        testDevice->deleteShader(handle);
    }

    TEST_F(ADeviceSupportingBinaryShaders, DownloadsBinaryDataForUploadedShader)
    {
        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResource());
        auto shaderGpuResource = testDevice->uploadShader(*testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        EXPECT_TRUE(handle.isValid());

        std::vector<std::byte> binaryShader;
        BinaryShaderFormatID binaryShaderFormat;
        EXPECT_TRUE(testDevice->getBinaryShader(handle, binaryShader, binaryShaderFormat));
        EXPECT_FALSE(binaryShader.empty());
        EXPECT_TRUE(binaryShaderFormat != BinaryShaderFormatID::Invalid());

        testDevice->deleteShader(handle);
    }

    TEST_F(ADeviceSupportingBinaryShaders, UploadsShaderFromBinaryData)
    {
        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResource());
        auto shaderGpuResource = testDevice->uploadShader(*testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        EXPECT_TRUE(handle.isValid());

        std::vector<std::byte> binaryShader;
        BinaryShaderFormatID binaryShaderFormat;
        EXPECT_TRUE(testDevice->getBinaryShader(handle, binaryShader, binaryShaderFormat));
        EXPECT_FALSE(binaryShader.empty());
        EXPECT_TRUE(binaryShaderFormat != BinaryShaderFormatID::Invalid());

        const DeviceResourceHandle handleBinary = testDevice->uploadBinaryShader(*testEffect, &binaryShader.front(), static_cast<uint32_t>(binaryShader.size()), binaryShaderFormat);
        EXPECT_TRUE(handle != handleBinary);

        testDevice->deleteShader(handle);
        testDevice->deleteShader(handleBinary);
    }

    TEST_F(ADeviceSupportingBinaryShaders, RefusesToCreateShaderFromEffectWhenPassingInvalidBinaryData)
    {
        ResourceBlob data(0);
        BinaryShaderFormatID binaryShaderFormat;

        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResource());
        const DeviceResourceHandle handle = testDevice->uploadBinaryShader(*testEffect, data.data(), static_cast<uint32_t>(data.size()), binaryShaderFormat);

        EXPECT_FALSE(handle.isValid());
    }

    TEST_F(ADeviceSupportingBinaryShaders, Confidence_SetsDataOnBinaryShader)
    {
        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResource());
        auto shaderGpuResource = testDevice->uploadShader(*testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        ASSERT_TRUE(handle.isValid());

        std::vector<std::byte> binaryShader;
        BinaryShaderFormatID binaryShaderFormat;
        EXPECT_TRUE(testDevice->getBinaryShader(handle, binaryShader, binaryShaderFormat));
        EXPECT_FALSE(binaryShader.empty());
        EXPECT_TRUE(binaryShaderFormat != BinaryShaderFormatID::Invalid());

        const DeviceResourceHandle handleBinary = testDevice->uploadBinaryShader(*testEffect, &binaryShader.front(), static_cast<uint32_t>(binaryShader.size()), binaryShaderFormat);
        EXPECT_TRUE(handle != handleBinary);

        testDevice->activateShader(handleBinary);

        const float floatValue = 5.0f;
        const glm::vec2 vec2Value(5.0f);

        // This will trigger internal asserts if the shader is not correct/loaded properly
        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect->getUniformDataFieldHandleByName("u_float")), 1, &floatValue));
        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect->getUniformDataFieldHandleByName("u_vec2")), 1, &vec2Value));

        testDevice->deleteShader(handle);
        testDevice->deleteShader(handleBinary);
    }

    TEST_F(ADeviceSupportingGeometryShaders, CreatesShaderFromEffect)
    {
        ASSERT_TRUE(testDevice != nullptr);

        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResourceWithGeometryShader());
        auto shaderGpuResource = testDevice->uploadShader(*testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        EXPECT_TRUE(handle.isValid());

        testDevice->deleteShader(handle);
    }

    TEST_F(ADeviceSupportingGeometryShaders, RefusesToUploadEffectWithInvalidGeometryShader)
    {
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResourceWithGeometryShader());
        const EffectResource invalidEffect(
            templateEffect->getVertexShader(),
            templateEffect->getFragmentShader(),
            "--this is some invalid shader source code--",
            SPIRVShaders{},
            EDrawMode::Lines,
            templateEffect->getUniformInputs(),
            templateEffect->getAttributeInputs(),
            "invalid effect", EFeatureLevel_Latest);
        const auto shaderGpuResource = testDevice->uploadShader(invalidEffect);
        EXPECT_EQ(nullptr, shaderGpuResource);
    }

    TEST_F(ADeviceSupportingGeometryShaders, RefusesToUploadEffectWithUnlinkableShaders)
    {
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResourceWithGeometryShader());

        const std::string geometryShader(R"SHADER(
                #version 320 es

                layout(points) in;
                layout(points, max_vertices = 1) out;
                uniform highp float u_geomFloat;

                uniform highp vec4 u_float; //interface mismatch

                void main() {

                    gl_Position = vec4(u_geomFloat * u_float);
                    EmitVertex();
                    EndPrimitive();
                }
                )SHADER");
        const EffectResource invalidEffect(
            templateEffect->getVertexShader(),
            templateEffect->getFragmentShader(),
            geometryShader,
            SPIRVShaders{},
            EDrawMode::Points,
            templateEffect->getUniformInputs(),
            templateEffect->getAttributeInputs(),
            "invalid effect", EFeatureLevel_Latest);
        const auto shaderGpuResource = testDevice->uploadShader(invalidEffect);
        EXPECT_EQ(nullptr, shaderGpuResource);
    }

    TEST_F(ADeviceSupportingGeometryShaders, SetsConstantsOnShader)
    {
        ASSERT_TRUE(testDevice != nullptr);

        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResourceWithGeometryShader());
        auto shaderGpuResource = testDevice->uploadShader(*testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        EXPECT_TRUE(handle.isValid());

        testDevice->activateShader(handle);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy()); // check that there were no errors (GL_NO_ERROR in case of OpenGL) - triggers an internal assert otherwise
        float floatValue = 10.0;
        EXPECT_TRUE(testDevice->setConstant(DataFieldHandle(testEffect->getUniformDataFieldHandleByName("u_geomFloat")), 1, &floatValue));
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());

        testDevice->deleteShader(handle);
    }

    TEST_F(ADeviceSupportingGeometryShaders, PrintInvalidShaderToLogger)
    {
        std::vector<std::string> collectedErrors;

        ramses::RamsesFramework::SetLogHandler([&collectedErrors](auto level, auto /*unused*/, auto message) {
            if (level == ramses::ELogLevel::Error)
            {
                collectedErrors.push_back(std::string(message));
            }
        });

        const auto invalidShader{"--this is some invalid shader source code--"};
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());
        {
            const EffectResource invalidEffect(invalidShader,
                                               templateEffect->getFragmentShader(),
                                               templateEffect->getGeometryShader(),
                                               SPIRVShaders{},
                                               std::nullopt,
                                               templateEffect->getUniformInputs(),
                                               templateEffect->getAttributeInputs(),
                                               "invalid effect", EFeatureLevel_Latest);
            (void)testDevice->uploadShader(invalidEffect);

            EXPECT_THAT(collectedErrors.front(), HasSubstr(invalidShader));
        }

        std::vector<std::string> shader = {
            "#version 320 es",
            "",
            "layout(points) in;",
            "layout(points, max_vertices = 1) out;",
            "uniform highp float u_geomFloat;",
            "",
            "uniform highp vec4 u_float; //interface mismatch",
            "",
            "void main() {",
            "",
            "    gl_Position = vec4(u_geomFloat * u_float);",
            "    EmitVertex();",
            "    EndPrimitive() // missing ;",
            "}",
        };
        std::string invalidGeometryShader{};
        for (auto& line : shader)
        {
            invalidGeometryShader += line;
            invalidGeometryShader += '\n';
        }
        collectedErrors.clear();
        {
            const EffectResource invalidEffect(templateEffect->getVertexShader(),
                                               templateEffect->getFragmentShader(),
                                               invalidGeometryShader,
                                               SPIRVShaders{},
                                               EDrawMode::Points,
                                               templateEffect->getUniformInputs(),
                                               templateEffect->getAttributeInputs(),
                                               "invalid effect", EFeatureLevel_Latest);
            (void)testDevice->uploadShader(invalidEffect);

            for (size_t i = 0u; i < shader.size(); ++i)
            {
                EXPECT_THAT(collectedErrors[i], HasSubstr(shader[i]));
            }
        }
        ramses::RamsesFramework::SetLogHandler(nullptr);
    }

    TEST_F(ADevice, GetsAttributeLocationsCorrectly)
    {
        ASSERT_TRUE(testDevice != nullptr);

        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());
        const std::string customVertexShader(R"SHADER(
            #version 310 es

            layout(location=1) in vec2 a_texCoords;
            layout(location=0) in vec3 a_position;

            void main(void)
            {
                gl_Position = vec4(a_position.x + a_texCoords.x);
            }
            )SHADER");
        const std::string customFragmentShader(R"SHADER(
            #version 310 es
            out highp vec4 fragColor;
            void main(void)
            {
                fragColor = vec4(1.0);
            }
            )SHADER");
        EffectInputInformationVector attributeInputs;
        attributeInputs.push_back(EffectInputInformation("a_texCoords", 1, EDataType::Vector2F, EFixedSemantics::Invalid));
        attributeInputs.push_back(EffectInputInformation("a_position", 1, EDataType::Vector3F, EFixedSemantics::Invalid));

        const EffectResource testEffect(
            customVertexShader,
            customFragmentShader,
            "",
            SPIRVShaders{},
            {}, {},
            attributeInputs,
            "uniform test effect", EFeatureLevel_Latest);

        auto shaderGpuResource = testDevice->uploadShader(testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        ASSERT_TRUE(handle.isValid());

        testDevice->activateShader(handle);
        const auto& shaderResource = renderBackend->getContext().getResources().getResourceAs<ShaderGPUResource_GL>(handle);
        EXPECT_EQ(1, shaderResource.getAttributeLocation(DataFieldHandle{ 0u }).getValue()); // tex coords
        EXPECT_EQ(0, shaderResource.getAttributeLocation(DataFieldHandle{ 1u }).getValue()); // position

        testDevice->deleteShader(handle);
    }

    TEST_F(ADevice, GetsOpaqueUniformLocationsCorrectly)
    {
        ASSERT_TRUE(testDevice != nullptr);

        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());
        const std::string customVertexShader(R"SHADER(
            #version 310 es

            layout(location=11) uniform vec2 u_myVec2;
            layout(location=5) uniform sampler2D u_mySampler1;
            layout(location=23) uniform vec3 u_myVec3;
            layout(location=7) uniform sampler2D u_mySampler2;

            void main(void)
            {
                gl_Position = vec4(u_myVec2.x + u_myVec3.x) + texture(u_mySampler1, vec2(0.0))+ texture(u_mySampler2, vec2(0.0));
            }
            )SHADER");
        const std::string customFragmentShader(R"SHADER(
            #version 310 es
            out highp vec4 fragColor;
            void main(void)
            {
                fragColor = vec4(1.0);
            }
            )SHADER");
        EffectInputInformationVector uniformInputs;
        uniformInputs.push_back(EffectInputInformation("u_myVec2", 1, EDataType::Vector2F, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_mySampler1", 1, EDataType::TextureSampler2D, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_myVec3", 1, EDataType::Vector3F, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_mySampler2", 1, EDataType::TextureSampler2D, EFixedSemantics::Invalid));

        const EffectResource testEffect(
            customVertexShader,
            customFragmentShader,
            "", SPIRVShaders{}, {},
            uniformInputs,
            {},
            "uniform test effect", EFeatureLevel_Latest);

        auto shaderGpuResource = testDevice->uploadShader(testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        ASSERT_TRUE(handle.isValid());

        testDevice->activateShader(handle);
        const auto& shaderResource = renderBackend->getContext().getResources().getResourceAs<ShaderGPUResource_GL>(handle);
        EXPECT_EQ(11, shaderResource.getUniformLocation(DataFieldHandle{ 0u }).getValue());
        EXPECT_EQ(5, shaderResource.getUniformLocation(DataFieldHandle{ 1u }).getValue());
        EXPECT_EQ(23, shaderResource.getUniformLocation(DataFieldHandle{ 2u }).getValue());
        EXPECT_EQ(7, shaderResource.getUniformLocation(DataFieldHandle{ 3u }).getValue());

        testDevice->deleteShader(handle);
    }

    TEST_F(ADevice, GetsOpaqueUniformLocationsCorrectly_IfShaderHasUBO)
    {
        ASSERT_TRUE(testDevice != nullptr);

        const std::string customVertexShader(R"SHADER(
            #version 310 es

            layout(location=11) uniform vec2 u_myVec2;
            layout(location=5) uniform sampler2D u_mySampler1;
            layout(std140,binding=1) uniform MyUbo_t
            {
                vec2 u_myVec2;
            } myUbo;
            layout(location=23) uniform vec3 u_myVec3;
            layout(location=7) uniform sampler2D u_mySampler2;

            void main(void)
            {
                gl_Position = texture(u_mySampler1, u_myVec2)+ texture(u_mySampler2, myUbo.u_myVec2) * u_myVec3.x;
            }
            )SHADER");
        const std::string customFragmentShader(R"SHADER(
            #version 310 es
            out highp vec4 fragColor;
            void main(void)
            {
                fragColor = vec4(1.0);
            }
            )SHADER");
        EffectInputInformationVector uniformInputs;
        uniformInputs.push_back(EffectInputInformation("u_myVec2", 1, EDataType::Vector2F, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_mySampler1", 1, EDataType::TextureSampler2D, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("myUbo", 1, EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }));
        uniformInputs.push_back(EffectInputInformation("myUbo.u_myVec2", 1, EDataType::Vector2F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 8u }, UniformBufferFieldOffset{ 0u }));
        uniformInputs.push_back(EffectInputInformation("u_myVec3", 1, EDataType::Vector3F, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_mySampler2", 1, EDataType::TextureSampler2D, EFixedSemantics::Invalid));
        const EffectResource testEffect(
            customVertexShader,
            customFragmentShader,
            "", SPIRVShaders{}, {},
            uniformInputs,
            {},
            "uniform test effect", EFeatureLevel_Latest);

        auto shaderGpuResource = testDevice->uploadShader(testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        ASSERT_TRUE(handle.isValid());

        testDevice->activateShader(handle);
        const auto& shaderResource = renderBackend->getContext().getResources().getResourceAs<ShaderGPUResource_GL>(handle);
        EXPECT_EQ(11, shaderResource.getUniformLocation(DataFieldHandle{ 0u }).getValue());
        EXPECT_EQ(5, shaderResource.getUniformLocation(DataFieldHandle{ 1u }).getValue());
        EXPECT_FALSE(shaderResource.getUniformLocation(DataFieldHandle{ 2u }).isValid());
        EXPECT_EQ(23, shaderResource.getUniformLocation(DataFieldHandle{ 3u }).getValue());
        EXPECT_EQ(7, shaderResource.getUniformLocation(DataFieldHandle{ 4u }).getValue());

        testDevice->deleteShader(handle);
    }
}
