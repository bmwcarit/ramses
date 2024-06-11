//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <array>

#include "internal/Components/SceneFileHandle.h"
#include "internal/PlatformAbstraction/PlatformError.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTargetDescription.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/PickableObject.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Effect.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/ramses-utils.h"

#include "impl/CameraNodeImpl.h"
#include "impl/AppearanceImpl.h"
#include "impl/DataObjectImpl.h"
#include "impl/GeometryImpl.h"
#include "impl/EffectImpl.h"
#include "impl/SceneImpl.h"
#include "impl/Texture2DImpl.h"
#include "impl/TextureSamplerImpl.h"
#include "impl/RenderGroupImpl.h"
#include "impl/RenderPassImpl.h"
#include "impl/BlitPassImpl.h"
#include "impl/PickableObjectImpl.h"
#include "impl/RenderBufferImpl.h"
#include "impl/RenderTargetImpl.h"
#include "impl/MeshNodeImpl.h"
#include "impl/ArrayBufferImpl.h"
#include "impl/Texture2DBufferImpl.h"

#include "internal/Core/Utils/File.h"
#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "internal/SceneGraph/SceneAPI/BlitPass.h"
#include "internal/SceneGraph/SceneAPI/RenderGroupUtils.h"
#include "internal/SceneGraph/SceneAPI/RenderPass.h"
#include "internal/SceneGraph/Scene/ESceneActionId.h"
#include "internal/SceneGraph/Scene/ResourceChanges.h"
#include "internal/SceneGraph/Scene/SceneActionApplier.h"

#include "ScenePersistationTest.h"
#include "TestEffects.h"
#include "FileDescriptorHelper.h"
#include "UnsafeTestMemoryHelpers.h"
#include "RamsesObjectTestTypes.h"
#include "FeatureLevelTestValues.h"
#include "ramses/framework/EFeatureLevel.h"

#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>

namespace ramses::internal
{
    using namespace testing;

    class ASceneLoadedFromFile : public SceneLoadedFromFile, public ::testing::TestWithParam<ramses::EFeatureLevel>
    {
    public:
        ASceneLoadedFromFile()
            : SceneLoadedFromFile{ GetParam() }
        {
        }
    };
    RAMSES_INSTANTIATE_FEATURELEVEL_TEST_SUITE(ASceneLoadedFromFile);

    TEST_P(ASceneLoadedFromFile, canWriteAndReadSceneFromFile)
    {
        EXPECT_TRUE(m_scene.saveToFile("someTemporaryFile.ram", {}));
        EXPECT_NE(nullptr, m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", {}));
    }

    TEST_P(ASceneLoadedFromFile, logsExporterMetadata)
    {
        ramses::SaveFileConfig config;
        config.setMetadataString("foo-bar-baz");
        config.setExporterVersion(1, 2, 3, 7);
        EXPECT_TRUE(m_scene.saveToFile("someTemporaryFile.ram", config));

        std::vector<std::string> logs;
        ramses::RamsesFramework::SetLogHandler([&logs](auto level, auto /*unused*/, auto message) {
            if (level == ramses::ELogLevel::Info)
            {
                logs.push_back(std::string(message));
            }
        });
        EXPECT_NE(nullptr, m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", {}));
        ramses::RamsesFramework::SetLogHandler(nullptr);
        EXPECT_THAT(logs, Contains("R.main: Metadata: 'foo-bar-baz'"));
        EXPECT_THAT(logs, Contains("R.main: Exporter version: 1.2.3 (file format version 7)"));
    }

    TEST_P(ASceneLoadedFromFile, loadedSceneHasReferenceToClientAndFrameworkAndMatchingFeaturelevel)
    {
        EXPECT_TRUE(m_scene.saveToFile("someTemporaryFile.ram", {}));
        auto loadedScene = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", {});
        ASSERT_NE(nullptr, loadedScene);
        EXPECT_EQ(&m_clientForLoading, &loadedScene->getRamsesClient());
        EXPECT_EQ(&m_frameworkForLoader, &loadedScene->getRamsesClient().getRamsesFramework());
        EXPECT_EQ(GetParam(), loadedScene->getRamsesClient().getRamsesFramework().getFeatureLevel());
    }

    TEST_P(ASceneLoadedFromFile, canWriteAndReadSceneFromMemoryWithExplicitDeleter)
    {
        EXPECT_TRUE(m_scene.saveToFile("someTemporaryFile.ram", {}));

        ramses::internal::File f("someTemporaryFile.ram");
        size_t fileSize = 0;
        EXPECT_TRUE(f.getSizeInBytes(fileSize));

        std::unique_ptr<std::byte[], void(*)(const std::byte*)> data(new std::byte[fileSize], [](const auto* ptr) { delete[] ptr; });
        size_t numBytesRead = 0;
        EXPECT_TRUE(f.open(ramses::internal::File::Mode::ReadOnlyBinary));
        EXPECT_EQ(ramses::internal::EStatus::Ok, f.read(data.get(), fileSize, numBytesRead));
        EXPECT_NE(nullptr, m_clientForLoading.loadSceneFromMemory(std::move(data), fileSize, {}));
    }

    TEST_P(ASceneLoadedFromFile, canWriteAndReadSceneFromMemoryWithImplicitDeleter)
    {
        EXPECT_TRUE(m_scene.saveToFile("someTemporaryFile.ram", {}));

        ramses::internal::File f("someTemporaryFile.ram");
        size_t fileSize = 0;
        EXPECT_TRUE(f.getSizeInBytes(fileSize));

        std::unique_ptr<std::byte[]> data(new std::byte[fileSize]);
        size_t numBytesRead = 0;
        EXPECT_TRUE(f.open(ramses::internal::File::Mode::ReadOnlyBinary));
        EXPECT_EQ(ramses::internal::EStatus::Ok, f.read(data.get(), fileSize, numBytesRead));
        EXPECT_NE(nullptr, RamsesUtils::LoadSceneFromMemory(m_clientForLoading, std::move(data), fileSize, {}));
    }

    TEST_P(ASceneLoadedFromFile, canWriteAndReadSceneFromFileDescriptor)
    {
        EXPECT_TRUE(m_scene.saveToFile("someTemporaryFile.ram", {}));

        size_t fileSize = 0;
        {
            // write to a file with some offset
            ramses::internal::File inFile("someTemporaryFile.ram");
            EXPECT_TRUE(inFile.getSizeInBytes(fileSize));
            std::vector<unsigned char> data(fileSize);
            size_t numBytesRead = 0;
            EXPECT_TRUE(inFile.open(ramses::internal::File::Mode::ReadOnlyBinary));
            EXPECT_EQ(ramses::internal::EStatus::Ok, inFile.read(data.data(), fileSize, numBytesRead));

            ramses::internal::File outFile("someTemporaryFileWithOffset.ram");
            EXPECT_TRUE(outFile.open(ramses::internal::File::Mode::WriteOverWriteOldBinary));

            uint32_t zeroData = 0;
            EXPECT_TRUE(outFile.write(&zeroData, sizeof(zeroData)));
            EXPECT_TRUE(outFile.write(data.data(), data.size()));
            EXPECT_TRUE(outFile.write(&zeroData, sizeof(zeroData)));
        }

        const int fd = ramses::internal::FileDescriptorHelper::OpenFileDescriptorBinary ("someTemporaryFileWithOffset.ram");
        auto scene = m_clientForLoading.loadSceneFromFileDescriptor(fd, 4, fileSize, {});
        ASSERT_NE(nullptr, scene);
        EXPECT_EQ(123u, scene->getSceneId().getValue());
    }

    TEST_P(ASceneLoadedFromFile, canReadSceneFromFileDescriptorCustomSceneId)
    {
        const char* filename = "someTemporaryFile.ram";
        EXPECT_TRUE(m_scene.saveToFile(filename, {}));
        size_t fileSize = 0;
        {
            ramses::internal::File inFile(filename);
            EXPECT_TRUE(inFile.getSizeInBytes(fileSize));
        }
        {
            SceneConfig config;
            config.setSceneId(sceneId_t(335));
            const int fdA = ramses::internal::FileDescriptorHelper::OpenFileDescriptorBinary(filename);
            auto sceneA = m_clientForLoading.loadSceneFromFileDescriptor(fdA, 0, fileSize, config);
            ASSERT_TRUE(sceneA);
            EXPECT_EQ(335u, sceneA->getSceneId().getValue());
        }
        {
            SceneConfig config;
            config.setSceneId(sceneId_t(339));
            const int fdB = ramses::internal::FileDescriptorHelper::OpenFileDescriptorBinary (filename);
            auto sceneB = m_clientForLoading.loadSceneFromFileDescriptor(fdB, 0, fileSize, config);
            ASSERT_TRUE(sceneB);
            EXPECT_EQ(339u, sceneB->getSceneId().getValue());
        }
    }

    TEST_P(ASceneLoadedFromFile, errorsReadingSceneFromFileDescriptorCustomSceneId)
    {
        const char* filename = "someTemporaryFile.ram";
        EXPECT_TRUE(m_scene.saveToFile(filename, {}));
        size_t fileSize = 0;
        {
            ramses::internal::File inFile(filename);
            EXPECT_TRUE(inFile.getSizeInBytes(fileSize));
        }

        const int fd  = ramses::internal::FileDescriptorHelper::OpenFileDescriptorBinary(filename);

        static_cast<void>(m_clientForLoading.createScene(SceneConfig(m_scene.getSceneId())));
        SceneConfig config;
        config.setSceneId(m_scene.getSceneId());
        const auto duplicateSceneId = m_clientForLoading.loadSceneFromFileDescriptor(fd, 0, fileSize, config);
        config.setSceneId(ramses::sceneId_t(340));
        const auto invalidFileDescriptor = m_clientForLoading.loadSceneFromFileDescriptor(0, 0, fileSize, config);
        const auto invalidFileSize = m_clientForLoading.loadSceneFromFileDescriptor(fd, 0, 0, config);

        EXPECT_TRUE(duplicateSceneId == nullptr);
        EXPECT_TRUE(invalidFileDescriptor == nullptr);
        EXPECT_TRUE(invalidFileSize == nullptr);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAScene)
    {
        EXPECT_TRUE(m_scene.saveToFile("someTemporaryFile.ram", {}));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", {});
        ASSERT_TRUE(nullptr != m_sceneLoaded);

        ObjectTypeHistogram origSceneNumbers;
        ObjectTypeHistogram loadedSceneNumbers;
        FillObjectTypeHistogramFromScene(origSceneNumbers, m_scene);
        FillObjectTypeHistogramFromScene(loadedSceneNumbers, *m_sceneLoaded);
        EXPECT_PRED_FORMAT2(AssertHistogramEqual, origSceneNumbers, loadedSceneNumbers);

        ramses::internal::SceneSizeInformation origSceneSizeInfo = m_scene.impl().getIScene().getSceneSizeInformation();
        ramses::internal::SceneSizeInformation loadedSceneSizeInfo = m_sceneLoaded->impl().getIScene().getSceneSizeInformation();
        EXPECT_EQ(origSceneSizeInfo, loadedSceneSizeInfo);
    }

    TEST_P(ASceneLoadedFromFile, validationReportSameBeforeSavingAndAfterLoadingFromFile)
    {
        TestEffects::CreateTestEffectWithAllStages(this->m_scene, "eff");
        ValidationReport report1;
        m_scene.validate(report1);
        EXPECT_FALSE(report1.hasIssue());

        doWriteReadCycle();

        ValidationReport report2;
        m_sceneLoaded->validate(report2);
        EXPECT_FALSE(report2.hasIssue());
        EXPECT_EQ(report1.impl().toString(), report2.impl().toString());
    }

    TEST_P(ASceneLoadedFromFile, validationReportWithWarningsSameBeforeSavingAndAfterLoadingFromFile)
    {
        TestEffects::CreateTestEffectWithAllStagesWithWarnings(this->m_scene, "eff");
        ValidationReport report1;
        m_scene.validate(report1);
        EXPECT_TRUE(report1.hasIssue());

        doWriteReadCycle();

        ValidationReport report2;
        m_sceneLoaded->validate(report2);
        EXPECT_TRUE(report2.hasIssue());
        EXPECT_EQ(report1.impl().toString(), report2.impl().toString());
    }

    TEST_P(ASceneLoadedFromFile, validationReportWithWarningsSameBeforeSavingAndAfterLoadingFromFileWithCompression)
    {
        TestEffects::CreateTestEffectWithAllStagesWithWarnings(this->m_scene, "eff");
        ValidationReport report1;
        m_scene.validate(report1);
        EXPECT_TRUE(report1.hasIssue());

        doWriteReadCycle(true, true, true);

        ValidationReport report2;
        m_sceneLoaded->validate(report2);
        EXPECT_TRUE(report2.hasIssue());
        EXPECT_EQ(report1.impl().toString(), report2.impl().toString());
    }

    TEST_P(ASceneLoadedFromFile, loadsLogic)
    {
        auto* logic = this->m_scene.createLogicEngine("my logic");
        auto* timer = logic->createTimerNode("dummy");

        doWriteReadCycle();

        auto* loadedLogic = this->getObjectForTesting<LogicEngine>("my logic");

        EXPECT_EQ(logic->getSceneObjectId(), loadedLogic->getSceneObjectId());
        EXPECT_NE(nullptr, loadedLogic->findObject(timer->getSceneObjectId()));
        EXPECT_EQ(1u, loadedLogic->getCollection<TimerNode>().size());

        EXPECT_TRUE(m_sceneLoaded->destroy(*loadedLogic));
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteASceneWithOpenGLCompatibility)
    {
        doWriteReadCycle();

        EXPECT_EQ(ERenderBackendCompatibility::OpenGL, m_sceneLoaded->impl().getIScene().getRenderBackendCompatibility());
        EXPECT_EQ(EVulkanAPIVersion::Invalid, m_sceneLoaded->impl().getIScene().getVulkanAPIVersion());
        EXPECT_EQ(ESPIRVVersion::Invalid, m_sceneLoaded->impl().getIScene().getSPIRVVersion());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteASceneWithVulkanAndOpenGLCompatibility)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        auto* scene = client.createScene(SceneConfig{ sceneId_t{456u}, EScenePublicationMode::LocalOnly, ERenderBackendCompatibility::VulkanAndOpenGL });
        EXPECT_TRUE(scene->saveToFile("someTemporaryFile.ram", {}));

        CheckSceneFile("someTemporaryFile.ram", scene);
        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram");
        ASSERT_TRUE(nullptr != m_sceneLoaded);

        EXPECT_EQ(ERenderBackendCompatibility::VulkanAndOpenGL, m_sceneLoaded->impl().getIScene().getRenderBackendCompatibility());
        EXPECT_EQ(TargetVulkanApiVersion, m_sceneLoaded->impl().getIScene().getVulkanAPIVersion());
        EXPECT_EQ(TargetSPIRVVersion, m_sceneLoaded->impl().getIScene().getSPIRVVersion());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAPerspectiveCamera)
    {
        auto* camera = this->m_scene.createPerspectiveCamera("my cam");
        camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        camera->setViewport(1, -2, 3, 4);

        doWriteReadCycle();

        auto* loadedCamera = this->getObjectForTesting<PerspectiveCamera>("my cam");
        EXPECT_EQ(camera->impl().getCameraHandle(), loadedCamera->impl().getCameraHandle());
        EXPECT_EQ(0.1f, loadedCamera->getLeftPlane());
        EXPECT_EQ(0.2f, loadedCamera->getRightPlane());
        EXPECT_EQ(0.3f, loadedCamera->getBottomPlane());
        EXPECT_EQ(0.4f, loadedCamera->getTopPlane());
        EXPECT_EQ(0.5f, loadedCamera->getNearPlane());
        EXPECT_EQ(0.6f, loadedCamera->getFarPlane());

        EXPECT_EQ(1, loadedCamera->getViewportX());
        EXPECT_EQ(-2, loadedCamera->getViewportY());
        EXPECT_EQ(3u, loadedCamera->getViewportWidth());
        EXPECT_EQ(4u, loadedCamera->getViewportHeight());

        ValidationReport report;
        loadedCamera->validate(report);
        EXPECT_FALSE(report.hasIssue());

        m_sceneLoaded->destroy(*loadedCamera);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAnOrthographicCamera)
    {
        OrthographicCamera* camera = this->m_scene.createOrthographicCamera("my cam");
        camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        camera->setViewport(1, -2, 3, 4);

        doWriteReadCycle();

        auto* loadedCamera = this->getObjectForTesting<OrthographicCamera>("my cam");

        EXPECT_EQ(camera->impl().getCameraHandle(), loadedCamera->impl().getCameraHandle());

        EXPECT_FLOAT_EQ(0.1f, loadedCamera->getLeftPlane());
        EXPECT_FLOAT_EQ(0.2f, loadedCamera->getRightPlane());
        EXPECT_FLOAT_EQ(0.3f, loadedCamera->getBottomPlane());
        EXPECT_FLOAT_EQ(0.4f, loadedCamera->getTopPlane());
        EXPECT_FLOAT_EQ(0.5f, loadedCamera->getNearPlane());
        EXPECT_FLOAT_EQ(0.6f, loadedCamera->getFarPlane());

        EXPECT_EQ(1, loadedCamera->getViewportX());
        EXPECT_EQ(-2, loadedCamera->getViewportY());
        EXPECT_EQ(3u, loadedCamera->getViewportWidth());
        EXPECT_EQ(4u, loadedCamera->getViewportHeight());

        ValidationReport report;
        loadedCamera->validate(report);
        EXPECT_FALSE(report.hasIssue());

        m_sceneLoaded->destroy(*loadedCamera);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAnEffect)
    {
        TestEffects::CreateTestEffectWithAllStages(this->m_scene, "eff");

        doWriteReadCycle();

        auto loadedEffect = this->getObjectForTesting<Effect>("eff");
        ASSERT_TRUE(loadedEffect);

        // check uniforms
        EXPECT_EQ(4u, loadedEffect->getUniformInputCount());
        std::optional<UniformInput> optUniform = loadedEffect->getUniformInput(0u);
        ASSERT_TRUE(optUniform.has_value());
        EXPECT_STREQ("vs_uniform", optUniform->getName());
        EXPECT_EQ(ramses::EDataType::Float, optUniform->getDataType());
        EXPECT_EQ(1u, optUniform->getElementCount());

        optUniform = loadedEffect->getUniformInput(1u);
        ASSERT_TRUE(optUniform.has_value());
        EXPECT_STREQ("colorRG", optUniform->getName());
        EXPECT_EQ(ramses::EDataType::Vector2F, optUniform->getDataType());
        EXPECT_EQ(1u, optUniform->getElementCount());

        optUniform = loadedEffect->getUniformInput(2u);
        ASSERT_TRUE(optUniform.has_value());
        EXPECT_STREQ("colorBA", optUniform->getName());
        EXPECT_EQ(ramses::EDataType::Float, optUniform->getDataType());
        EXPECT_EQ(2u, optUniform->getElementCount());

        optUniform = loadedEffect->getUniformInput(3u);
        ASSERT_TRUE(optUniform.has_value());
        EXPECT_STREQ("gs_uniform", optUniform->getName());
        EXPECT_EQ(ramses::EDataType::Float, optUniform->getDataType());
        EXPECT_EQ(1u, optUniform->getElementCount());

        // check attributes
        EXPECT_EQ(3u, loadedEffect->getAttributeInputCount());
        std::optional<AttributeInput> optAttrib = loadedEffect->getAttributeInput(0u);
        ASSERT_TRUE(optAttrib.has_value());
        EXPECT_STREQ("a_position1", optAttrib->getName());
        EXPECT_EQ(ramses::EDataType::Vector3F, optAttrib->getDataType());

        optAttrib = loadedEffect->getAttributeInput(1u);
        ASSERT_TRUE(optAttrib.has_value());
        EXPECT_STREQ("a_position2", optAttrib->getName());
        EXPECT_EQ(ramses::EDataType::Float, optAttrib->getDataType());

        optAttrib = loadedEffect->getAttributeInput(2u);
        ASSERT_TRUE(optAttrib.has_value());
        EXPECT_STREQ("a_position3", optAttrib->getName());
        EXPECT_EQ(ramses::EDataType::Float, optAttrib->getDataType());

        // GS
        EXPECT_TRUE(loadedEffect->hasGeometryShader());
        EDrawMode gsInputType = EDrawMode::Points;
        EXPECT_TRUE(loadedEffect->getGeometryShaderInputType(gsInputType));
        EXPECT_EQ(EDrawMode::Lines, gsInputType);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAnEffect_withUBO)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        TestEffects::CreateTestEffectWithUBOSemantics(this->m_scene, { EEffectUniformSemantic::ModelBlock, EEffectUniformSemantic::CameraBlock, EEffectUniformSemantic::ModelCameraBlock }, "eff");

        doWriteReadCycle();

        auto loadedEffect = this->getObjectForTesting<Effect>("eff");
        ASSERT_TRUE(loadedEffect);

        EXPECT_EQ(10u, loadedEffect->getUniformInputCount());

        const auto uniModel = loadedEffect->getUniformInput(0u);
        ASSERT_TRUE(uniModel);
        EXPECT_STREQ("modelUBO", uniModel->getName());
        EXPECT_EQ(ramses::EDataType::UniformBuffer, uniModel->getDataType());

        const auto uniModelField1 = loadedEffect->getUniformInput(1u);
        ASSERT_TRUE(uniModelField1);
        EXPECT_STREQ("modelUBO.ubModelMat", uniModelField1->getName());
        EXPECT_EQ(ramses::EDataType::Matrix44F, uniModelField1->getDataType());
        EXPECT_EQ(1u, uniModelField1->getElementCount());

        const auto uniCam = loadedEffect->getUniformInput(2u);
        ASSERT_TRUE(uniCam);
        EXPECT_STREQ("camUBO", uniCam->getName());
        EXPECT_EQ(ramses::EDataType::UniformBuffer, uniCam->getDataType());

        const auto uniCamProj = loadedEffect->getUniformInput(3u);
        ASSERT_TRUE(uniCamProj);
        EXPECT_STREQ("camUBO.projMat", uniCamProj->getName());
        EXPECT_EQ(ramses::EDataType::Matrix44F, uniCamProj->getDataType());
        EXPECT_EQ(1u, uniCamProj->getElementCount());

        const auto uniCamView = loadedEffect->getUniformInput(4u);
        ASSERT_TRUE(uniCamView);
        EXPECT_STREQ("camUBO.viewMat", uniCamView->getName());
        EXPECT_EQ(ramses::EDataType::Matrix44F, uniCamView->getDataType());
        EXPECT_EQ(1u, uniCamView->getElementCount());

        const auto uniCamPos = loadedEffect->getUniformInput(5u);
        ASSERT_TRUE(uniCamPos);
        EXPECT_STREQ("camUBO.cameraPos", uniCamPos->getName());
        EXPECT_EQ(ramses::EDataType::Vector3F, uniCamPos->getDataType());
        EXPECT_EQ(1u, uniCamPos->getElementCount());

        const auto uniModelCam = loadedEffect->getUniformInput(6u);
        ASSERT_TRUE(uniModelCam);
        EXPECT_STREQ("modelCamUBO", uniModelCam->getName());
        EXPECT_EQ(ramses::EDataType::UniformBuffer, uniModelCam->getDataType());

        const auto uniModelCamMVP = loadedEffect->getUniformInput(7u);
        ASSERT_TRUE(uniModelCamMVP);
        EXPECT_STREQ("modelCamUBO.mvpMat", uniModelCamMVP->getName());
        EXPECT_EQ(ramses::EDataType::Matrix44F, uniModelCamMVP->getDataType());
        EXPECT_EQ(1u, uniModelCamMVP->getElementCount());

        const auto uniModelCamMV = loadedEffect->getUniformInput(8u);
        ASSERT_TRUE(uniModelCamMV);
        EXPECT_STREQ("modelCamUBO.mvMat", uniModelCamMV->getName());
        EXPECT_EQ(ramses::EDataType::Matrix44F, uniModelCamMV->getDataType());
        EXPECT_EQ(1u, uniModelCamMV->getElementCount());

        const auto uniModelCamN = loadedEffect->getUniformInput(9u);
        ASSERT_TRUE(uniModelCamN);
        EXPECT_STREQ("modelCamUBO.normalMat", uniModelCamN->getName());
        EXPECT_EQ(ramses::EDataType::Matrix44F, uniModelCamN->getDataType());
        EXPECT_EQ(1u, uniModelCamN->getElementCount());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAnAppearance)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);
        Appearance* appearance = this->m_scene.createAppearance(*effect, "appearance");

        doWriteReadCycle();

        auto* loadedAppearance = this->getObjectForTesting<Appearance>("appearance");

        EXPECT_EQ(appearance->getEffect().impl().getLowlevelResourceHash(), loadedAppearance->getEffect().impl().getLowlevelResourceHash());
        EXPECT_EQ(appearance->impl().getRenderStateHandle(), loadedAppearance->impl().getRenderStateHandle());
        EXPECT_EQ(appearance->impl().getUniformDataInstance(), loadedAppearance->impl().getUniformDataInstance());
        EXPECT_EQ(appearance->impl().getIScene().getSceneId(), loadedAppearance->impl().getIScene().getSceneId());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAnAppearanceWithGeometryShaderAndRestrictDrawMode)
    {
        const Effect* effect = TestEffects::CreateTestEffectWithAllStages(this->m_scene);
        this->m_scene.createAppearance(*effect, "appearance");

        doWriteReadCycle();

        auto* loadedAppearance = this->getObjectForTesting<Appearance>("appearance");

        EDrawMode drawMode = EDrawMode::Points;
        EXPECT_TRUE(loadedAppearance->getDrawMode(drawMode));
        EXPECT_EQ(EDrawMode::Lines, drawMode);

        EDrawMode gsInputType = EDrawMode::Points;
        ASSERT_TRUE(loadedAppearance->getEffect().hasGeometryShader());
        EXPECT_TRUE(loadedAppearance->getEffect().getGeometryShaderInputType(gsInputType));
        EXPECT_EQ(EDrawMode::Lines, gsInputType);

        // valid draw mode change
        EXPECT_TRUE(loadedAppearance->setDrawMode(EDrawMode::LineStrip));

        // invalid draw mode change
        EXPECT_FALSE(loadedAppearance->setDrawMode(EDrawMode::Points));
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAnAppearanceWithGeometryShaderAndRestrictDrawMode_usingSameEffect)
    {
        const Effect* effect1 = TestEffects::CreateTestEffectWithAllStages(this->m_scene);
        const Effect* effect2 = TestEffects::CreateTestEffectWithAllStages(this->m_scene);
        this->m_scene.createAppearance(*effect1, "appearance1");
        this->m_scene.createAppearance(*effect2, "appearance2");

        doWriteReadCycle();

        auto* loadedAppearance1 = this->getObjectForTesting<Appearance>("appearance1");
        auto* loadedAppearance2 = this->getObjectForTesting<Appearance>("appearance2");

        // appearance 1
        EDrawMode drawMode = EDrawMode::Points;
        EDrawMode gsInputType = EDrawMode::Points;
        EXPECT_TRUE(loadedAppearance1->getDrawMode(drawMode));
        EXPECT_EQ(EDrawMode::Lines, drawMode);
        ASSERT_TRUE(loadedAppearance1->getEffect().hasGeometryShader());
        EXPECT_TRUE(loadedAppearance1->getEffect().getGeometryShaderInputType(gsInputType));
        EXPECT_EQ(EDrawMode::Lines, gsInputType);

        // appearance 2
        EXPECT_TRUE(loadedAppearance1->getDrawMode(drawMode));
        EXPECT_EQ(EDrawMode::Lines, drawMode);
        ASSERT_TRUE(loadedAppearance1->getEffect().hasGeometryShader());
        EXPECT_TRUE(loadedAppearance1->getEffect().getGeometryShaderInputType(gsInputType));
        EXPECT_EQ(EDrawMode::Lines, gsInputType);

        // valid draw mode change
        EXPECT_TRUE(loadedAppearance1->setDrawMode(EDrawMode::LineStrip));
        EXPECT_TRUE(loadedAppearance2->setDrawMode(EDrawMode::LineStrip));

        // invalid draw mode change
        EXPECT_FALSE(loadedAppearance1->setDrawMode(EDrawMode::Points));
        EXPECT_FALSE(loadedAppearance2->setDrawMode(EDrawMode::Points));
    }

    TEST_P(ASceneLoadedFromFile, keepingTrackOfsceneObjectIdsAndFindObjectByIdWork)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);

        const Appearance* appearance = this->m_scene.createAppearance(*effect, "appearance");
        const Geometry* geometry = this->m_scene.createGeometry(*effect, "geometry");

        const sceneObjectId_t appearanceIdBeforeSaveAndLoad = appearance->getSceneObjectId();
        const sceneObjectId_t geometryIdBeforeSaveAndLoad = geometry->getSceneObjectId();
        EXPECT_NE(appearanceIdBeforeSaveAndLoad, geometryIdBeforeSaveAndLoad);

        doWriteReadCycle();

        const auto& appearanceLoaded = *this->m_scene.findObject<Appearance>(appearanceIdBeforeSaveAndLoad);
        EXPECT_EQ(appearanceIdBeforeSaveAndLoad, appearanceLoaded.getSceneObjectId());

        const auto& geometryLoaded = *this->m_scene.findObject<Geometry>(geometryIdBeforeSaveAndLoad);
        EXPECT_EQ(geometryIdBeforeSaveAndLoad, geometryLoaded.getSceneObjectId());

        const ramses::Camera* camera = this->m_scene.createOrthographicCamera("camera");
        EXPECT_NE(appearanceIdBeforeSaveAndLoad, camera->getSceneObjectId());
        EXPECT_NE(geometryIdBeforeSaveAndLoad, camera->getSceneObjectId());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAnAppearanceWithUniformValuesSetOrBound)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);
        Appearance* appearance = this->m_scene.createAppearance(*effect, "appearance");

        std::optional<UniformInput> optFragColorR = effect->findUniformInput("u_FragColorR");
        std::optional<UniformInput> optFragColorG = effect->findUniformInput("u_FragColorG");

        auto fragColorDataObject = this->m_scene.createDataObject(ramses::EDataType::Float);
        fragColorDataObject->setValue(123.f);

        appearance->setInputValue(*optFragColorR, 567.f);
        appearance->bindInput(*optFragColorG, *fragColorDataObject);

        doWriteReadCycle();

        auto* loadedAppearance = this->getObjectForTesting<Appearance>("appearance");
        const Effect& loadedEffect = loadedAppearance->getEffect();

        std::optional<UniformInput> optFragColorROut = loadedEffect.findUniformInput("u_FragColorR");
        std::optional<UniformInput> optFragColorGOut = loadedEffect.findUniformInput("u_FragColorG");

        float resultR = 0.f;
        loadedAppearance->getInputValue(*optFragColorROut, resultR);
        EXPECT_FLOAT_EQ(567.f, resultR);

        ASSERT_TRUE(loadedAppearance->isInputBound(*optFragColorGOut));
        float resultG = 0.f;
        const DataObject& fragColorDataObjectOut = *loadedAppearance->getDataObjectBoundToInput(*optFragColorGOut);
        fragColorDataObjectOut.getValue(resultG);
        EXPECT_EQ(123.f, resultG);
    }

    TEST_P(ASceneLoadedFromFile, multipleAppearancesSharingSameEffectAreCorrectlyWrittenAndLoaded)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);
        const Appearance* appearance1 = m_scene.createAppearance(*effect, "appearance1");
        const Appearance* appearance2 = m_scene.createAppearance(*effect, "appearance2");

        // check data layout ref count on LL
        EXPECT_EQ(appearance1->impl().getUniformDataLayout(), appearance2->impl().getUniformDataLayout());
        const uint32_t numReferences = m_scene.impl().getIScene().getNumDataLayoutReferences(appearance1->impl().getUniformDataLayout());
        EXPECT_EQ(2u, numReferences);

        doWriteReadCycle();

        auto* loadedAppearance1 = this->getObjectForTesting<Appearance>("appearance1");
        auto* loadedAppearance2 = this->getObjectForTesting<Appearance>("appearance2");

        // check data layout ref count on LL in loaded scene
        EXPECT_EQ(loadedAppearance1->impl().getUniformDataLayout(), loadedAppearance2->impl().getUniformDataLayout());
        const uint32_t numReferencesLoaded = m_scene.impl().getIScene().getNumDataLayoutReferences(loadedAppearance1->impl().getUniformDataLayout());
        EXPECT_EQ(numReferences, numReferencesLoaded);

        m_sceneLoaded->destroy(*loadedAppearance2);
        m_sceneLoaded->destroy(*loadedAppearance1);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteGeometry)
    {
        static const uint16_t inds[3] = { 0, 1, 2 };
        ArrayResource* const indices = this->m_scene.createArrayResource(3u, inds, "indices");

        Effect* effect = TestEffects::CreateTestEffectWithAttribute(this->m_scene);

        Geometry* geometry = this->m_scene.createGeometry(*effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);
        EXPECT_TRUE(geometry->setIndices(*indices));

        doWriteReadCycle();

        const Geometry* loadedGeometry = this->getObjectForTesting<Geometry>("geometry");

        EXPECT_EQ(geometry->impl().getEffectHash(), loadedGeometry->impl().getEffectHash());
        EXPECT_EQ(geometry->impl().getAttributeDataLayout(), loadedGeometry->impl().getAttributeDataLayout());
        EXPECT_EQ(geometry->impl().getAttributeDataInstance(), loadedGeometry->impl().getAttributeDataInstance());
        EXPECT_EQ(geometry->impl().getIndicesCount(), loadedGeometry->impl().getIndicesCount());

        const Effect& loadedEffect = loadedGeometry->getEffect();
        EXPECT_EQ(effect->getResourceId(), loadedEffect.getResourceId());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), loadedEffect.impl().getLowlevelResourceHash());
        EXPECT_EQ(4u, loadedEffect.getAttributeInputCount());
        std::optional<AttributeInput> attributeInputOut = loadedEffect.findAttributeInput("a_position");
        EXPECT_TRUE(attributeInputOut.has_value());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAMeshNode)
    {
        MeshNode* meshNode = this->m_scene.createMeshNode("a meshnode");

        doWriteReadCycle();

        auto* loadedMeshNode = this->getObjectForTesting<MeshNode>("a meshnode");

        EXPECT_EQ(meshNode->getAppearance(), loadedMeshNode->getAppearance());
        EXPECT_EQ(meshNode->getGeometry(),   loadedMeshNode->getGeometry());
        EXPECT_EQ(meshNode->getStartIndex(), loadedMeshNode->getStartIndex());
        EXPECT_EQ(meshNode->getIndexCount(), loadedMeshNode->getIndexCount());
        EXPECT_EQ(meshNode->impl().getFlattenedVisibility(), loadedMeshNode->impl().getFlattenedVisibility());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAMeshNode_withVisibilityParent)
    {
        MeshNode* meshNode = this->m_scene.createMeshNode("a meshnode");

        Node* visibilityParent = this->m_scene.createNode("vis node");
        visibilityParent->setVisibility(EVisibilityMode::Invisible);
        visibilityParent->addChild(*meshNode);

        // Apply visibility state only with flush, not before
        EXPECT_EQ(meshNode->impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        this->m_scene.flush();
        EXPECT_EQ(meshNode->impl().getFlattenedVisibility(), EVisibilityMode::Invisible);

        doWriteReadCycle();

        auto* loadedMeshNode = this->getObjectForTesting<MeshNode>("a meshnode");
        EXPECT_EQ(loadedMeshNode->impl().getFlattenedVisibility(), EVisibilityMode::Invisible);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAMeshNode_withVisibilityParentOff)
    {
        MeshNode* meshNode = this->m_scene.createMeshNode("a meshnode");

        Node* visibilityParent = this->m_scene.createNode("vis node");
        visibilityParent->setVisibility(EVisibilityMode::Off);
        visibilityParent->addChild(*meshNode);

        // Apply visibility state only with flush, not before
        EXPECT_EQ(meshNode->impl().getFlattenedVisibility(), EVisibilityMode::Visible);
        this->m_scene.flush();
        EXPECT_EQ(meshNode->impl().getFlattenedVisibility(), EVisibilityMode::Off);

        doWriteReadCycle();

        auto* loadedMeshNode = this->getObjectForTesting<MeshNode>("a meshnode");
        EXPECT_EQ(loadedMeshNode->impl().getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteAMeshNode_withValues)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);
        Appearance* appearance = this->m_scene.createAppearance(*effect, "appearance");

        Geometry* geometry = this->m_scene.createGeometry(*effect, "geometry");
        const uint16_t data = 0u;
        ArrayResource* indices = this->m_scene.createArrayResource(1u, &data, "indices");
        geometry->setIndices(*indices);

        MeshNode* meshNode = this->m_scene.createMeshNode("a meshnode");

        //EXPECT_TRUE(meshNode.setIndexArray(indices));
        EXPECT_TRUE(meshNode->setAppearance(*appearance));
        EXPECT_TRUE(meshNode->setGeometry(*geometry));
        EXPECT_TRUE(meshNode->setStartIndex(456));
        EXPECT_TRUE(meshNode->setIndexCount(678u));
        EXPECT_TRUE(meshNode->impl().setFlattenedVisibility(EVisibilityMode::Off));
        doWriteReadCycle();

        auto* loadedMeshNode = this->getObjectForTesting<MeshNode>("a meshnode");

        EXPECT_EQ(meshNode->getAppearance()->getName(), loadedMeshNode->getAppearance()->getName());
        EXPECT_EQ(meshNode->getAppearance()->getSceneObjectId(), loadedMeshNode->getAppearance()->getSceneObjectId());
        EXPECT_EQ(meshNode->getGeometry()->getName(), loadedMeshNode->getGeometry()->getName());
        EXPECT_EQ(meshNode->getGeometry()->getSceneObjectId(), loadedMeshNode->getGeometry()->getSceneObjectId());
        EXPECT_EQ(meshNode->getSceneObjectId(), loadedMeshNode->getSceneObjectId());
        EXPECT_EQ(456u, loadedMeshNode->getStartIndex());
        EXPECT_EQ(678u, loadedMeshNode->getIndexCount());
        EXPECT_EQ(loadedMeshNode->impl().getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteANodeWithVisibility)
    {
        Node* visibilityNode = this->m_scene.createNode("a visibilitynode");

        visibilityNode->setVisibility(EVisibilityMode::Invisible);

        doWriteReadCycle();

        Node* loadedVisibilityNode = this->getObjectForTesting<Node>("a visibilitynode");

        EXPECT_EQ(loadedVisibilityNode->getVisibility(), EVisibilityMode::Invisible);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteARenderGroup)
    {
        ramses::RenderGroup* renderGroup = this->m_scene.createRenderGroup("a rendergroup");

        MeshNode* meshA = this->m_scene.createMeshNode("meshA");
        MeshNode* meshB = this->m_scene.createMeshNode("meshB");

        renderGroup->addMeshNode(*meshA, 1);
        renderGroup->addMeshNode(*meshB, 2);

        doWriteReadCycle();

        auto* loadedRenderGroup = this->getObjectForTesting<ramses::RenderGroup>("a rendergroup");
        const MeshNode* loadedMeshA = this->getObjectForTesting<MeshNode>("meshA");
        const MeshNode* loadedMeshB = this->getObjectForTesting<MeshNode>("meshB");

        EXPECT_EQ(renderGroup->getName(), loadedRenderGroup->getName());
        EXPECT_EQ(renderGroup->getSceneObjectId(), loadedRenderGroup->getSceneObjectId());

        EXPECT_EQ(2u, loadedRenderGroup->impl().getAllMeshes().size());
        EXPECT_EQ(&loadedMeshA->impl(), loadedRenderGroup->impl().getAllMeshes()[0]);
        EXPECT_EQ(&loadedMeshB->impl(), loadedRenderGroup->impl().getAllMeshes()[1]);

        const auto& internalRg = m_sceneLoaded->impl().getIScene().getRenderGroup(renderGroup->impl().getRenderGroupHandle());
        ASSERT_TRUE(ramses::internal::RenderGroupUtils::ContainsRenderable(meshA->impl().getRenderableHandle(), internalRg));
        ASSERT_TRUE(ramses::internal::RenderGroupUtils::ContainsRenderable(meshB->impl().getRenderableHandle(), internalRg));
        EXPECT_EQ(1, ramses::internal::RenderGroupUtils::FindRenderableEntry(meshA->impl().getRenderableHandle(), internalRg)->order);
        EXPECT_EQ(2, ramses::internal::RenderGroupUtils::FindRenderableEntry(meshB->impl().getRenderableHandle(), internalRg)->order);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteANestedRenderGroup)
    {
        ramses::RenderGroup* renderGroup = this->m_scene.createRenderGroup("a rendergroup");
        ramses::RenderGroup* nestedRenderGroup = this->m_scene.createRenderGroup("a nested rendergroup");

        MeshNode* meshA = this->m_scene.createMeshNode("meshA");
        MeshNode* meshB = this->m_scene.createMeshNode("meshB");

        ASSERT_TRUE(renderGroup->addMeshNode(*meshA, 1));
        ASSERT_TRUE(nestedRenderGroup->addMeshNode(*meshB, 2));
        ASSERT_TRUE(renderGroup->addRenderGroup(*nestedRenderGroup, 1));

        doWriteReadCycle();

        auto* loadedRenderGroup = this->getObjectForTesting<ramses::RenderGroup>("a rendergroup");
        auto* loadedNestedRenderGroup = this->getObjectForTesting<ramses::RenderGroup>("a nested rendergroup");
        const MeshNode* loadedMeshA = this->getObjectForTesting<MeshNode>("meshA");
        const MeshNode* loadedMeshB = this->getObjectForTesting<MeshNode>("meshB");

        EXPECT_EQ(nestedRenderGroup->getName(), loadedNestedRenderGroup->getName());
        EXPECT_EQ(nestedRenderGroup->getSceneObjectId(), loadedNestedRenderGroup->getSceneObjectId());

        EXPECT_EQ(1u, loadedRenderGroup->impl().getAllMeshes().size());
        EXPECT_EQ(1u, loadedRenderGroup->impl().getAllRenderGroups().size());
        EXPECT_EQ(1u, loadedNestedRenderGroup->impl().getAllMeshes().size());

        EXPECT_EQ(&loadedMeshA->impl(), loadedRenderGroup->impl().getAllMeshes()[0]);
        EXPECT_EQ(&loadedMeshB->impl(), loadedNestedRenderGroup->impl().getAllMeshes()[0]);

        EXPECT_EQ(&loadedNestedRenderGroup->impl(), loadedRenderGroup->impl().getAllRenderGroups()[0]);

        const auto& internalRg = m_sceneLoaded->impl().getIScene().getRenderGroup(renderGroup->impl().getRenderGroupHandle());
        ASSERT_TRUE(ramses::internal::RenderGroupUtils::ContainsRenderable(meshA->impl().getRenderableHandle(), internalRg));
        EXPECT_EQ(1, ramses::internal::RenderGroupUtils::FindRenderableEntry(meshA->impl().getRenderableHandle(), internalRg)->order);

        ASSERT_TRUE(ramses::internal::RenderGroupUtils::ContainsRenderGroup(nestedRenderGroup->impl().getRenderGroupHandle(), internalRg));
        EXPECT_EQ(1, ramses::internal::RenderGroupUtils::FindRenderGroupEntry(nestedRenderGroup->impl().getRenderGroupHandle(), internalRg)->order);

        const auto& internalRgNested = m_sceneLoaded->impl().getIScene().getRenderGroup(nestedRenderGroup->impl().getRenderGroupHandle());
        ASSERT_TRUE(ramses::internal::RenderGroupUtils::ContainsRenderable(meshB->impl().getRenderableHandle(), internalRgNested));
        EXPECT_EQ(2, ramses::internal::RenderGroupUtils::FindRenderableEntry(meshB->impl().getRenderableHandle(), internalRgNested)->order);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteABasicRenderPass)
    {
        const int32_t renderOrder = 1;

        ramses::RenderPass* renderPass = this->m_scene.createRenderPass("a renderpass");
        EXPECT_TRUE(renderPass->setRenderOrder(renderOrder));
        EXPECT_TRUE(renderPass->setEnabled(false));
        EXPECT_TRUE(renderPass->setRenderOnce(true));

        doWriteReadCycle();

        auto* loadedRenderPass = this->getObjectForTesting<ramses::RenderPass>("a renderpass");

        EXPECT_EQ(renderPass->getName(), loadedRenderPass->getName());
        EXPECT_EQ(renderPass->getSceneObjectId(), loadedRenderPass->getSceneObjectId());
        EXPECT_EQ(renderOrder, loadedRenderPass->getRenderOrder());
        EXPECT_FALSE(loadedRenderPass->isEnabled());
        EXPECT_TRUE(loadedRenderPass->isRenderOnce());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteARenderPassWithACamera)
    {
        ramses::RenderPass* renderPass = this->m_scene.createRenderPass("a renderpass");

        PerspectiveCamera* perspCam = this->m_scene.createPerspectiveCamera("camera");
        perspCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        perspCam->setViewport(0, 0, 100, 200);
        renderPass->setCamera(*perspCam);

        doWriteReadCycle();

        auto* loadedRenderPass = this->getObjectForTesting<ramses::RenderPass>("a renderpass");
        const auto* loadedCamera = this->getObjectForTesting<ramses::Camera>("camera");

        EXPECT_EQ(renderPass->getName(), loadedRenderPass->getName());
        EXPECT_EQ(renderPass->getSceneObjectId(), loadedRenderPass->getSceneObjectId());
        EXPECT_EQ(loadedCamera, loadedRenderPass->getCamera());
        ValidationReport report;
        loadedCamera->validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteARenderPassWhichHasRenderGroups)
    {
        ramses::RenderPass* renderPass = this->m_scene.createRenderPass("a renderpass");
        ramses::RenderGroup* groupA = this->m_scene.createRenderGroup("groupA");
        ramses::RenderGroup* groupB = this->m_scene.createRenderGroup("groupB");
        renderPass->addRenderGroup(*groupA, 1);
        renderPass->addRenderGroup(*groupB, 2);

        doWriteReadCycle();

        auto* loadedRenderPass = this->getObjectForTesting<ramses::RenderPass>("a renderpass");
        const auto* loadedMeshA = this->getObjectForTesting<ramses::RenderGroup>("groupA");
        const auto* loadedMeshB = this->getObjectForTesting<ramses::RenderGroup>("groupB");

        EXPECT_EQ(renderPass->getName(), loadedRenderPass->getName());
        EXPECT_EQ(renderPass->getSceneObjectId(), loadedRenderPass->getSceneObjectId());
        EXPECT_EQ(2u, loadedRenderPass->impl().getAllRenderGroups().size());
        EXPECT_EQ(&loadedMeshA->impl(), loadedRenderPass->impl().getAllRenderGroups()[0]);
        EXPECT_EQ(&loadedMeshB->impl(), loadedRenderPass->impl().getAllRenderGroups()[1]);

        const ramses::internal::RenderPass& internalRP = m_sceneLoaded->impl().getIScene().getRenderPass(renderPass->impl().getRenderPassHandle());
        ASSERT_TRUE(ramses::internal::RenderGroupUtils::ContainsRenderGroup(groupA->impl().getRenderGroupHandle(), internalRP));
        ASSERT_TRUE(ramses::internal::RenderGroupUtils::ContainsRenderGroup(groupB->impl().getRenderGroupHandle(), internalRP));
        EXPECT_EQ(groupA->impl().getRenderGroupHandle(), ramses::internal::RenderGroupUtils::FindRenderGroupEntry(groupA->impl().getRenderGroupHandle(), internalRP)->renderGroup);
        EXPECT_EQ(groupB->impl().getRenderGroupHandle(), ramses::internal::RenderGroupUtils::FindRenderGroupEntry(groupB->impl().getRenderGroupHandle(), internalRP)->renderGroup);
        EXPECT_EQ(1, ramses::internal::RenderGroupUtils::FindRenderGroupEntry(groupA->impl().getRenderGroupHandle(), internalRP)->order);
        EXPECT_EQ(2, ramses::internal::RenderGroupUtils::FindRenderGroupEntry(groupB->impl().getRenderGroupHandle(), internalRP)->order);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteBlitPass)
    {
        const int32_t renderOrder = 1;
        const ramses::RenderBuffer* srcRenderBuffer = this->m_scene.createRenderBuffer(23, 42, ERenderBufferFormat::Depth32, ERenderBufferAccessMode::WriteOnly, 0u, "src renderBuffer");
        const ramses::RenderBuffer* dstRenderBuffer = this->m_scene.createRenderBuffer(23, 42, ERenderBufferFormat::Depth32, ERenderBufferAccessMode::WriteOnly, 0u, "dst renderBuffer");

        ramses::BlitPass* blitPass = this->m_scene.createBlitPass(*srcRenderBuffer, *dstRenderBuffer, "a blitpass");
        EXPECT_TRUE(blitPass->setRenderOrder(renderOrder));
        EXPECT_TRUE(blitPass->setEnabled(false));

        doWriteReadCycle();

        const auto* loadedBlitPass = this->getObjectForTesting<ramses::BlitPass>("a blitpass");

        EXPECT_EQ(blitPass->getName(), loadedBlitPass->getName());
        EXPECT_EQ(blitPass->getSceneObjectId(), loadedBlitPass->getSceneObjectId());
        EXPECT_EQ(renderOrder, loadedBlitPass->getRenderOrder());
        EXPECT_FALSE(loadedBlitPass->isEnabled());

        const ramses::internal::BlitPassHandle loadedBlitPassHandle = loadedBlitPass->impl().getBlitPassHandle();
        const ramses::internal::BlitPass& blitPassInternal = m_sceneLoaded->impl().getIScene().getBlitPass(loadedBlitPassHandle);
        EXPECT_EQ(renderOrder, blitPassInternal.renderOrder);
        EXPECT_FALSE(blitPassInternal.isEnabled);
        EXPECT_EQ(srcRenderBuffer->impl().getRenderBufferHandle(), blitPassInternal.sourceRenderBuffer);
        EXPECT_EQ(dstRenderBuffer->impl().getRenderBufferHandle(), blitPassInternal.destinationRenderBuffer);
        EXPECT_EQ(renderOrder, blitPassInternal.renderOrder);

        const ramses::internal::PixelRectangle& sourceRegion = blitPassInternal.sourceRegion;
        EXPECT_EQ(0u, sourceRegion.x);
        EXPECT_EQ(0u, sourceRegion.y);
        EXPECT_EQ(static_cast<int32_t>(srcRenderBuffer->getWidth()), sourceRegion.width);
        EXPECT_EQ(static_cast<int32_t>(srcRenderBuffer->getHeight()), sourceRegion.height);

        const ramses::internal::PixelRectangle& destinationRegion = blitPassInternal.destinationRegion;
        EXPECT_EQ(0u, destinationRegion.x);
        EXPECT_EQ(0u, destinationRegion.y);
        EXPECT_EQ(static_cast<int32_t>(dstRenderBuffer->getWidth()), destinationRegion.width);
        EXPECT_EQ(static_cast<int32_t>(dstRenderBuffer->getHeight()), destinationRegion.height);

        //client HL api
        {
            uint32_t sourceXOut = std::numeric_limits<uint32_t>::max();
            uint32_t sourceYOut = std::numeric_limits<uint32_t>::max();
            uint32_t destinationXOut = std::numeric_limits<uint32_t>::max();
            uint32_t destinationYOut = std::numeric_limits<uint32_t>::max();
            uint32_t widthOut = std::numeric_limits<uint32_t>::max();
            uint32_t heightOut = std::numeric_limits<uint32_t>::max();
            loadedBlitPass->getBlittingRegion(sourceXOut, sourceYOut, destinationXOut, destinationYOut, widthOut, heightOut);
            EXPECT_EQ(0u, sourceXOut);
            EXPECT_EQ(0u, sourceYOut);
            EXPECT_EQ(0u, destinationXOut);
            EXPECT_EQ(0u, destinationYOut);
            EXPECT_EQ(dstRenderBuffer->getWidth(), widthOut);
            EXPECT_EQ(dstRenderBuffer->getHeight(), heightOut);
        }

        EXPECT_EQ(srcRenderBuffer->impl().getRenderBufferHandle(), loadedBlitPass->getSourceRenderBuffer().impl().getRenderBufferHandle());
        EXPECT_EQ(dstRenderBuffer->impl().getRenderBufferHandle(), loadedBlitPass->getDestinationRenderBuffer().impl().getRenderBufferHandle());
    }

    TEST_P(ASceneLoadedFromFile, canReadWritePickableObject)
    {
        const ramses::EDataType geometryBufferDataType = ramses::EDataType::Vector3F;
        const ArrayBuffer* geometryBuffer = this->m_scene.createArrayBuffer(geometryBufferDataType, 3u, "geometryBuffer");

        const int32_t viewPort_x = 1;
        const int32_t viewPort_y = 2;
        const uint32_t viewPort_width = 200;
        const uint32_t viewPort_height = 300;
        PerspectiveCamera* pickableCamera = m_scene.createPerspectiveCamera("pickableCamera");
        pickableCamera->setFrustum(-1.4f, 1.4f, -1.4f, 1.4f, 1.f, 100.f);
        pickableCamera->setViewport(viewPort_x, viewPort_y, viewPort_width, viewPort_height);

        const pickableObjectId_t id(2);
        ramses::PickableObject* pickableObject = this->m_scene.createPickableObject(*geometryBuffer, id, "PickableObject");
        EXPECT_TRUE(pickableObject->setCamera(*pickableCamera));
        EXPECT_TRUE(pickableObject->setEnabled(false));

        doWriteReadCycle();

        const auto* loadedPickableObject = this->getObjectForTesting<ramses::PickableObject>("PickableObject");

        EXPECT_EQ(pickableObject->getName(), loadedPickableObject->getName());
        EXPECT_EQ(pickableObject->getSceneObjectId(), loadedPickableObject->getSceneObjectId());
        EXPECT_EQ(id, loadedPickableObject->getPickableObjectId());
        EXPECT_FALSE(loadedPickableObject->isEnabled());
        EXPECT_EQ(this->getObjectForTesting<PerspectiveCamera>("pickableCamera"), loadedPickableObject->getCamera());
        EXPECT_EQ(this->getObjectForTesting<ArrayBuffer>("geometryBuffer"), &loadedPickableObject->getGeometryBuffer());

        const ramses::internal::PickableObjectHandle loadedPickableObjectPassHandle = loadedPickableObject->impl().getPickableObjectHandle();
        const ramses::internal::PickableObject& pickableObjectInternal = m_sceneLoaded->impl().getIScene().getPickableObject(loadedPickableObjectPassHandle);
        EXPECT_EQ(id.getValue(), pickableObjectInternal.id.getValue());
        EXPECT_FALSE(pickableObjectInternal.isEnabled);
        EXPECT_EQ(geometryBuffer->impl().getDataBufferHandle(), pickableObjectInternal.geometryHandle);
        EXPECT_EQ(pickableCamera->impl().getCameraHandle(), pickableObjectInternal.cameraHandle);

        EXPECT_EQ(geometryBuffer->impl().getDataBufferHandle(), loadedPickableObject->getGeometryBuffer().impl().getDataBufferHandle());
        EXPECT_EQ(pickableCamera->impl().getCameraHandle(), loadedPickableObject->getCamera()->impl().getCameraHandle());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteRenderBuffer)
    {
        ramses::RenderBuffer* renderBuffer = this->m_scene.createRenderBuffer(23, 42, ERenderBufferFormat::Depth16, ERenderBufferAccessMode::WriteOnly, 4u, "a renderTarget");

        doWriteReadCycle();

        auto* loadedRenderBuffer = this->getObjectForTesting<ramses::RenderBuffer>("a renderTarget");

        EXPECT_EQ(renderBuffer->getName(), loadedRenderBuffer->getName());
        EXPECT_EQ(renderBuffer->getSceneObjectId(), loadedRenderBuffer->getSceneObjectId());
        EXPECT_EQ(renderBuffer->getWidth(), loadedRenderBuffer->getWidth());
        EXPECT_EQ(renderBuffer->getHeight(), loadedRenderBuffer->getHeight());
        EXPECT_EQ(renderBuffer->getBufferFormat(), loadedRenderBuffer->getBufferFormat());
        EXPECT_EQ(renderBuffer->getAccessMode(), loadedRenderBuffer->getAccessMode());
        EXPECT_EQ(renderBuffer->getSampleCount(), loadedRenderBuffer->getSampleCount());

        EXPECT_EQ(renderBuffer->impl().getRenderBufferHandle(), loadedRenderBuffer->impl().getRenderBufferHandle());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteARenderPassWithARenderTargetAndCamera)
    {
        ramses::RenderPass* renderPass = this->m_scene.createRenderPass("a renderpass");

        ramses::RenderBuffer* renderBuffer = this->m_scene.createRenderBuffer(23u, 42u, ERenderBufferFormat::Depth32, ERenderBufferAccessMode::ReadWrite, 0u, "a renderBuffer");
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(*renderBuffer);
        ramses::RenderTarget* renderTarget = this->m_scene.createRenderTarget(rtDesc, "target");

        OrthographicCamera* orthoCam = this->m_scene.createOrthographicCamera("camera");
        orthoCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        orthoCam->setViewport(0, 0, 100, 200);

        renderPass->setCamera(*orthoCam);
        renderPass->setRenderTarget(renderTarget);

        doWriteReadCycle();

        const auto* loadedRenderPass = this->getObjectForTesting<ramses::RenderPass>("a renderpass");
        const auto* loadedRenderTarget = this->getObjectForTesting<ramses::RenderTarget>("target");
        const auto* loadedCamera = this->getObjectForTesting<OrthographicCamera>("camera");

        EXPECT_EQ(loadedRenderTarget, loadedRenderPass->getRenderTarget());
        EXPECT_EQ(loadedCamera, loadedRenderPass->getCamera());
        ValidationReport report;
        loadedRenderTarget->validate(report);
        loadedCamera->validate(report);
        EXPECT_FALSE(report.hasIssue()) << report.impl().toString();
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteRenderTarget)
    {
        const ramses::RenderBuffer& rb = *m_scene.createRenderBuffer(16u, 8u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(rb);

        ramses::RenderTarget* renderTarget = this->m_scene.createRenderTarget(rtDesc, "a renderTarget");

        doWriteReadCycle();

        auto* loadedRenderTarget = this->getObjectForTesting<ramses::RenderTarget>("a renderTarget");

        EXPECT_EQ(renderTarget->getName(), loadedRenderTarget->getName());
        EXPECT_EQ(renderTarget->getSceneObjectId(), loadedRenderTarget->getSceneObjectId());
        EXPECT_EQ(renderTarget->getWidth(), loadedRenderTarget->getWidth());
        EXPECT_EQ(renderTarget->getHeight(), loadedRenderTarget->getHeight());

        EXPECT_EQ(renderTarget->impl().getRenderTargetHandle(), loadedRenderTarget->impl().getRenderTargetHandle());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteIndexDataBuffer)
    {
        ArrayBuffer& buffer = *m_scene.createArrayBuffer(ramses::EDataType::UInt32, 6u, "indexDB");
        buffer.updateData(3u, 2u, std::array<uint32_t, 2>{ {6, 7} }.data());

        doWriteReadCycle();

        const auto* loadedBuffer = this->getObjectForTesting<ArrayBuffer>("indexDB");

        EXPECT_EQ(buffer.getName(), loadedBuffer->getName());
        EXPECT_EQ(buffer.impl().getDataBufferHandle(), loadedBuffer->impl().getDataBufferHandle());
        EXPECT_EQ(6 * sizeof(uint32_t), m_scene.impl().getIScene().getDataBuffer(loadedBuffer->impl().getDataBufferHandle()).data.size());
        EXPECT_EQ(5 * sizeof(uint32_t), m_scene.impl().getIScene().getDataBuffer(loadedBuffer->impl().getDataBufferHandle()).usedSize);

        const std::byte* loadedDataBufferData = m_scene.impl().getIScene().getDataBuffer(loadedBuffer->impl().getDataBufferHandle()).data.data();
        EXPECT_EQ(6u, ramses::internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedDataBufferData, 3));
        EXPECT_EQ(7u, ramses::internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedDataBufferData, 4));

        EXPECT_EQ(6u, loadedBuffer->getMaximumNumberOfElements());
        EXPECT_EQ(5u, loadedBuffer->getUsedNumberOfElements());
        EXPECT_EQ(ramses::EDataType::UInt32, loadedBuffer->getDataType());
        std::array<uint32_t, 6> bufferDataOut{};
        EXPECT_TRUE(loadedBuffer->getData(bufferDataOut.data(), static_cast<uint32_t>(bufferDataOut.size() * sizeof(uint32_t))));
        EXPECT_EQ(6u, bufferDataOut[3]);
        EXPECT_EQ(7u, bufferDataOut[4]);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteTexture2DBuffer)
    {
        Texture2DBuffer& buffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2, "textureBuffer");
        buffer.updateData(0, 0, 0, 2, 2, UnsafeTestMemoryHelpers::ConvertToBytes({12, 23, 34, 56}));
        buffer.updateData(1, 0, 0, 1, 1, UnsafeTestMemoryHelpers::ConvertToBytes({78}));

        doWriteReadCycle();

        const auto* loadedBuffer = this->getObjectForTesting<Texture2DBuffer>("textureBuffer");

        EXPECT_EQ(buffer.getName(), loadedBuffer->getName());
        EXPECT_EQ(buffer.getSceneObjectId(), loadedBuffer->getSceneObjectId());
        EXPECT_EQ(buffer.impl().getTextureBufferHandle(), loadedBuffer->impl().getTextureBufferHandle());

        //iscene
        const ramses::internal::TextureBuffer& loadedInternalBuffer = m_scene.impl().getIScene().getTextureBuffer(loadedBuffer->impl().getTextureBufferHandle());
        ASSERT_EQ(2u, loadedInternalBuffer.mipMaps.size());
        EXPECT_EQ(3u, loadedInternalBuffer.mipMaps[0].width);
        EXPECT_EQ(4u, loadedInternalBuffer.mipMaps[0].height);
        EXPECT_EQ(1u, loadedInternalBuffer.mipMaps[1].width);
        EXPECT_EQ(2u, loadedInternalBuffer.mipMaps[1].height);
        EXPECT_EQ(56u, ramses::internal::TextureBuffer::GetMipMapDataSizeInBytes(loadedInternalBuffer));
        EXPECT_EQ(ramses::internal::EPixelStorageFormat::RGBA8, loadedInternalBuffer.textureFormat);

        const std::byte* loadedBufferDataMip0 = loadedInternalBuffer.mipMaps[0].data.data();
        const std::byte* loadedBufferDataMip1 = loadedInternalBuffer.mipMaps[1].data.data();
        EXPECT_EQ(12u, ramses::internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedBufferDataMip0, 0));
        EXPECT_EQ(23u, ramses::internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedBufferDataMip0, 1));
        EXPECT_EQ(34u, ramses::internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedBufferDataMip0, 3 * 1 + 0));
        EXPECT_EQ(56u, ramses::internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedBufferDataMip0, 3 * 1 + 1));
        EXPECT_EQ(78u, ramses::internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedBufferDataMip1, 0));

        //client API
        EXPECT_EQ(2u, loadedBuffer->getMipLevelCount());
        uint32_t mipLevelWidthOut = 0u;
        uint32_t mipLevelHeightOut = 0u;
        EXPECT_TRUE(loadedBuffer->getMipLevelSize(0u, mipLevelWidthOut, mipLevelHeightOut));
        EXPECT_EQ(3u, mipLevelWidthOut);
        EXPECT_EQ(4u, mipLevelHeightOut);
        EXPECT_TRUE(loadedBuffer->getMipLevelSize(1u, mipLevelWidthOut, mipLevelHeightOut));
        EXPECT_EQ(1u, mipLevelWidthOut);
        EXPECT_EQ(2u, mipLevelHeightOut);

        EXPECT_EQ(ETextureFormat::RGBA8, loadedBuffer->getTexelFormat());
        std::array<uint32_t, 12> bufferForMip0{};
        std::array<uint32_t, 2>  bufferForMip1{};
        EXPECT_TRUE(loadedBuffer->getMipLevelData(0u, bufferForMip0.data(), static_cast<uint32_t>(bufferForMip0.size() * sizeof(uint32_t))));
        EXPECT_TRUE(loadedBuffer->getMipLevelData(1u, bufferForMip1.data(), static_cast<uint32_t>(bufferForMip1.size() * sizeof(uint32_t))));
        EXPECT_EQ(12u, bufferForMip0[0]);
        EXPECT_EQ(23u, bufferForMip0[1]);
        EXPECT_EQ(34u, bufferForMip0[3 * 1 + 0]);
        EXPECT_EQ(56u, bufferForMip0[3 * 1 + 1]);
        EXPECT_EQ(78u, bufferForMip1[0]);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteANode)
    {
        //generic node cannot be created, therefore using group node
        Node* grandParent = this->m_scene.createNode("node1");
        Node* parent = this->m_scene.createNode("node2");
        Node* child = this->m_scene.createNode("node3");

        grandParent->addChild(*parent);
        child->setParent(*parent);

        child->setTranslation({1, 2, 3});
        child->setVisibility(EVisibilityMode::Invisible);
        child->setRotation({1, 2, 3}, ERotationType::Euler_XYX);
        child->setScaling({1, 2, 3});

        doWriteReadCycle();

        auto* loadedGrandParent = this->getObjectForTesting<Node>("node1");
        auto* loadedParent = this->getObjectForTesting<Node>("node2");
        auto* loadedChild = this->getObjectForTesting<Node>("node3");
        ASSERT_TRUE(nullptr != loadedGrandParent);
        ASSERT_TRUE(nullptr != loadedParent);
        ASSERT_TRUE(nullptr != loadedChild);
        EXPECT_EQ(loadedParent, loadedChild->getParent());
        EXPECT_EQ(loadedGrandParent, loadedParent->getParent());
        EXPECT_EQ(loadedParent, loadedGrandParent->getChild(0u));
        EXPECT_EQ(1u, loadedGrandParent->getChildCount());
        EXPECT_EQ(1u, loadedParent->getChildCount());
        EXPECT_EQ(0u, loadedChild->getChildCount());

        vec3f translation;
        EXPECT_TRUE(loadedChild->getTranslation(translation));
        EXPECT_FLOAT_EQ(1, translation.x);
        EXPECT_FLOAT_EQ(2, translation.y);
        EXPECT_FLOAT_EQ(3, translation.z);

        EXPECT_EQ(loadedChild->getVisibility(), EVisibilityMode::Invisible);

        vec3f rotation;
        EXPECT_TRUE(loadedChild->getRotation(rotation));
        EXPECT_FLOAT_EQ(1, rotation.x);
        EXPECT_FLOAT_EQ(2, rotation.y);
        EXPECT_FLOAT_EQ(3, rotation.z);
        EXPECT_EQ(ERotationType::Euler_XYX, loadedChild->getRotationType());

        vec3f scale;
        EXPECT_TRUE(loadedChild->getScaling(scale));
        EXPECT_FLOAT_EQ(1, scale.x);
        EXPECT_FLOAT_EQ(2, scale.y);
        EXPECT_FLOAT_EQ(3, scale.z);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteANodeWithTranslation)
    {
        Node* node = this->m_scene.createNode("translate node 1");
        Node* child = this->m_scene.createNode("groupnode child");

        node->setTranslation({1, 2, 3});
        node->addChild(*child);

        doWriteReadCycle();

        auto* loadedTranslateNode = getObjectForTesting<Node>("translate node 1");
        auto* loadedChild = getObjectForTesting<Node>("groupnode child");

        ASSERT_TRUE(nullptr != loadedTranslateNode);
        ASSERT_TRUE(nullptr != loadedChild);

        EXPECT_EQ(1u, node->getChildCount());
        EXPECT_EQ(loadedChild, loadedTranslateNode->getChild(0u));
        vec3f translation;
        EXPECT_TRUE(node->getTranslation(translation));
        EXPECT_FLOAT_EQ(1, translation.x);
        EXPECT_FLOAT_EQ(2, translation.y);
        EXPECT_FLOAT_EQ(3, translation.z);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteANodeWithRotation)
    {
        Node* node = this->m_scene.createNode("rotate node 1");
        Node* child = this->m_scene.createNode("groupnode child");

        node->setRotation({1, 2, 3}, ERotationType::Euler_ZYX);
        child->setParent(*node);
        doWriteReadCycle();

        auto* loadedRotateNode = getObjectForTesting<Node>("rotate node 1");
        auto* loadedChild = getObjectForTesting<Node>("groupnode child");

        ASSERT_TRUE(nullptr != loadedRotateNode);
        ASSERT_TRUE(nullptr != loadedChild);

        EXPECT_EQ(1u, loadedRotateNode->getChildCount());
        EXPECT_EQ(loadedChild, loadedRotateNode->getChild(0u));
        vec3f rotation;
        EXPECT_TRUE(loadedRotateNode->getRotation(rotation));
        EXPECT_FLOAT_EQ(1, rotation.x);
        EXPECT_FLOAT_EQ(2, rotation.y);
        EXPECT_FLOAT_EQ(3, rotation.z);
        EXPECT_EQ(ERotationType::Euler_ZYX, loadedRotateNode->getRotationType());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteANodeWithScaling)
    {
        Node* node = this->m_scene.createNode("scale node");
        Node* child = this->m_scene.createNode("groupnode child");

        node->setScaling({1, 2, 3});
        child->setParent(*node);
        doWriteReadCycle();

        auto* loadedScaleNode = getObjectForTesting<Node>("scale node");
        auto* loadedChild = getObjectForTesting<Node>("groupnode child");

        ASSERT_TRUE(nullptr != loadedScaleNode);
        ASSERT_TRUE(nullptr != loadedChild);

        EXPECT_EQ(1u, loadedScaleNode->getChildCount());
        EXPECT_EQ(loadedChild, loadedScaleNode->getChild(0u));
        vec3f scale;
        EXPECT_TRUE(loadedScaleNode->getScaling(scale));
        EXPECT_FLOAT_EQ(1, scale.x);
        EXPECT_FLOAT_EQ(2, scale.y);
        EXPECT_FLOAT_EQ(3, scale.z);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteATextureSampler)
    {
        const ETextureAddressMode wrapUMode = ETextureAddressMode::Mirror;
        const ETextureAddressMode wrapVMode = ETextureAddressMode::Repeat;
        const ETextureSamplingMethod minSamplingMethod = ETextureSamplingMethod::Linear_MipMapNearest;
        const ETextureSamplingMethod magSamplingMethod = ETextureSamplingMethod::Linear;
        const std::vector<std::byte> data(4, std::byte{ 0u });
        const std::vector<MipLevelData> mipLevelData{ data };
        Texture2D* texture = this->m_scene.createTexture2D(ETextureFormat::RGBA8, 1u, 1u, mipLevelData, false, {}, "texture");

        auto* sampler = this->m_scene.createTextureSampler(wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, *texture, 8u, "sampler");
        ASSERT_TRUE(nullptr != sampler);

        doWriteReadCycle();

        auto* loadedSampler = getObjectForTesting<ramses::TextureSampler>("sampler");
        ASSERT_TRUE(nullptr != loadedSampler);

        EXPECT_EQ(wrapUMode, loadedSampler->getWrapUMode());
        EXPECT_EQ(wrapVMode, loadedSampler->getWrapVMode());
        EXPECT_EQ(minSamplingMethod, loadedSampler->getMinSamplingMethod());
        EXPECT_EQ(magSamplingMethod, loadedSampler->getMagSamplingMethod());
        EXPECT_EQ(8u, loadedSampler->getAnisotropyLevel());
        EXPECT_EQ(texture->impl().getLowlevelResourceHash(), this->m_sceneLoaded->impl().getIScene().getTextureSampler(loadedSampler->impl().getTextureSamplerHandle()).textureResource);
        EXPECT_EQ(ERamsesObjectType::Texture2D, loadedSampler->impl().getTextureType());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteATextureSamplerMS)
    {
        ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(4u, 4u, ERenderBufferFormat::RGB8, ERenderBufferAccessMode::ReadWrite, 4u);

        TextureSamplerMS* sampler = this->m_scene.createTextureSamplerMS(*renderBuffer, "sampler");
        ASSERT_TRUE(nullptr != sampler);

        doWriteReadCycle();

        auto* loadedSampler = getObjectForTesting<TextureSamplerMS>("sampler");
        ASSERT_TRUE(nullptr != loadedSampler);

        EXPECT_EQ(ERamsesObjectType::RenderBuffer, loadedSampler->impl().getTextureType());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteATextureSamplerExternal)
    {
        TextureSamplerExternal* sampler = this->m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, "sampler");
        ASSERT_TRUE(nullptr != sampler);

        doWriteReadCycle();

        auto* loadedSampler = getObjectForTesting<TextureSamplerExternal>("sampler");
        ASSERT_TRUE(nullptr != loadedSampler);

        EXPECT_EQ(ERamsesObjectType::TextureSamplerExternal, loadedSampler->impl().getTextureType());
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteSceneId)
    {
        const sceneId_t sceneId = ramses::sceneId_t(1ULL << 63);
        ramses::Scene& mScene(*client.createScene(SceneConfig(sceneId)));

        EXPECT_TRUE(mScene.saveToFile("someTempararyFile.ram", {}));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram");
        ASSERT_TRUE(nullptr != m_sceneLoaded);

        EXPECT_EQ(sceneId, m_sceneLoaded->getSceneId());
    }

    TEST_P(ASceneLoadedFromFile, defaultsToLocalPublicationMode)
    {
        const sceneId_t sceneId(81);
        EXPECT_TRUE(client.createScene(SceneConfig(sceneId))->saveToFile("someTempararyFile.ram", {}));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram");
        ASSERT_TRUE(nullptr != m_sceneLoaded);
        EXPECT_EQ(EScenePublicationMode::LocalOnly, m_sceneLoaded->impl().getPublicationModeSetFromSceneConfig());
    }

    TEST_P(ASceneLoadedFromFile, canOverridePublicationModeForLoadedFiles_savedAsLocalOnly_loadedAsRemote)
    {
        const sceneId_t sceneId(80);
        SceneConfig config(sceneId, EScenePublicationMode::LocalOnly);
        EXPECT_TRUE(client.createScene(config)->saveToFile("someTempararyFile.ram", {}));

        SceneConfig loadConfig;
        loadConfig.setPublicationMode(EScenePublicationMode::LocalAndRemote);
        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram", loadConfig);
        ASSERT_TRUE(nullptr != m_sceneLoaded);
        EXPECT_EQ(EScenePublicationMode::LocalAndRemote, m_sceneLoaded->impl().getPublicationModeSetFromSceneConfig());
    }

    TEST_P(ASceneLoadedFromFile, canOverridePublicationModeForLoadedFiles_savedAsRemote_loadedAsLocalOnly)
    {
        const sceneId_t sceneId(80);
        SceneConfig config(sceneId, EScenePublicationMode::LocalAndRemote);
        EXPECT_TRUE(client.createScene(config)->saveToFile("someTempararyFile.ram", {}));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram", {}); // default is LocalOnly
        ASSERT_TRUE(nullptr != m_sceneLoaded);
        EXPECT_EQ(EScenePublicationMode::LocalOnly, m_sceneLoaded->impl().getPublicationModeSetFromSceneConfig());
    }

    TEST_P(ASceneLoadedFromFile, reportsErrorWhenSavingSceneToFileWithInvalidFileName)
    {
        EXPECT_FALSE(m_scene.saveToFile("?Euler_ZYX:/dummyFile", {}));
    }

    TEST_P(ASceneLoadedFromFile, reportsErrorWhenSavingSceneToFileWithNoFileName)
    {
        EXPECT_FALSE(m_scene.saveToFile({}, {}));
    }

    TEST_P(ASceneLoadedFromFile, overwritesExistingFileWhenSavingSceneToIt)
    {
        {
            ramses::internal::File existingFile("dummyFile.dat");
            existingFile.createFile();
        }

        EXPECT_TRUE(m_scene.saveToFile("dummyFile.dat", {}));
        {
            ramses::internal::File fileShouldBeOverwritten("dummyFile.dat");
            EXPECT_TRUE(fileShouldBeOverwritten.open(ramses::internal::File::Mode::ReadOnly));
            size_t fileSize = 0;
            EXPECT_TRUE(fileShouldBeOverwritten.getSizeInBytes(fileSize));
            EXPECT_NE(0u, fileSize);
        }

        EXPECT_TRUE(ramses::internal::File("dummyFile.dat").remove());
    }

    TEST_P(ASceneLoadedFromFile, doesNotLoadSceneFromFileWithInvalidFileName)
    {
        ramses::Scene* scene = client.loadSceneFromFile("?Euler_ZYX:/dummyFile");
        EXPECT_TRUE(nullptr == scene);
    }

    TEST_P(ASceneLoadedFromFile, doesNotLoadSceneFromFileWithoutFileName)
    {
        EXPECT_EQ(nullptr, client.loadSceneFromFile({}));
        EXPECT_EQ(nullptr, client.loadSceneFromFile(""));
    }

    TEST_P(ASceneLoadedFromFile, doesNotLoadSceneFromInvalidMemory)
    {
        auto deleter = [](const auto* ptr) { delete[] ptr; };
        EXPECT_EQ(nullptr, client.loadSceneFromMemory(std::unique_ptr<std::byte[], void (*)(const std::byte*)>(nullptr, deleter), 1, {}));
        EXPECT_EQ(nullptr, client.loadSceneFromMemory(std::unique_ptr<std::byte[], void(*)(const std::byte*)>(new std::byte[1], deleter), 0, {}));

        EXPECT_EQ(nullptr, RamsesUtils::LoadSceneFromMemory(client, std::unique_ptr<std::byte[]>(nullptr), 1, {}));
        EXPECT_EQ(nullptr, RamsesUtils::LoadSceneFromMemory(client, std::unique_ptr<std::byte[]>(new std::byte[1]), 0, {}));
    }

    TEST_P(ASceneLoadedFromFile, doesNotLoadSceneFromInvalidFileDescriptor)
    {
        EXPECT_EQ(nullptr, client.loadSceneFromFileDescriptor(-1, 0, 1, {}));
        EXPECT_EQ(nullptr, client.loadSceneFromFileDescriptor(1, 0, 0, {}));
    }

    TEST_P(ASceneLoadedFromFile, doesNotLoadSceneFromUnexistingFile)
    {
        ramses::Scene* scene = client.loadSceneFromFile("ZEGETWTWAGTGSDGEg_thisfilename_in_this_directory_should_not_exist_DSAFDSFSTEZHDXHB");
        EXPECT_TRUE(nullptr == scene);
    }

    TEST_P(ASceneLoadedFromFile, canHandleAllZeroFileOnSceneLoad)
    {
        const char* filename = "allzerofile.dat";
        {
            ramses::internal::File file(filename);
            EXPECT_TRUE(file.open(ramses::internal::File::Mode::WriteNew));
            std::vector<char> zerovector(4096);
            EXPECT_TRUE(file.write(&zerovector[0], zerovector.size()));
            file.close();
        }

        ramses::Scene* scene = client.loadSceneFromFile(filename);
        EXPECT_TRUE(scene == nullptr);
    }

    TEST_P(ASceneLoadedFromFile, cannotLoadSameFileTwice)
    {
        const sceneId_t sceneId = ramses::sceneId_t(1ULL << 63);
        ramses::Scene* scene = client.createScene(SceneConfig(sceneId));
        EXPECT_TRUE(scene->saveToFile("someTempararyFile.ram", {}));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram");
        EXPECT_NE(nullptr, m_sceneLoaded);

        EXPECT_EQ(nullptr, m_clientForLoading.loadSceneFromFile("someTempararyFile.ram"));
    }

    TEST_P(ASceneLoadedFromFile, cannotLoadScenesWithSameSceneIdTwice)
    {
        const sceneId_t sceneId = ramses::sceneId_t(1ULL << 63);

        {
            ramses::Scene* scene = client.createScene(SceneConfig(sceneId));
            EXPECT_TRUE(scene->saveToFile("someTempararyFile.ram", {}));
            client.destroy(*scene);
        }
        {
            ramses::Scene* scene = client.createScene(SceneConfig(sceneId));
            EXPECT_TRUE(scene->saveToFile("someTempararyFile_2.ram", {}));
            client.destroy(*scene);
        }

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram");
        EXPECT_NE(nullptr, m_sceneLoaded);

        EXPECT_EQ(nullptr, m_clientForLoading.loadSceneFromFile("someTempararyFile_2.ram"));
    }

    TEST_P(ASceneLoadedFromFile, cannotLoadSceneWithMismatchingFeatureLevel)
    {
        SaveSceneWithFeatureLevelToFile(EFeatureLevel(99), "someTemporaryFile.ram");
        EXPECT_EQ(nullptr, m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram"));
    }

    TEST_P(ASceneLoadedFromFile, canGetFeatureLevelFromSceneFile)
    {
        SaveSceneWithFeatureLevelToFile(EFeatureLevel_Latest, "someTemporaryFile.ram");

        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_TRUE(RamsesClient::GetFeatureLevelFromFile("someTemporaryFile.ram", featureLevel));
        EXPECT_EQ(EFeatureLevel_Latest, featureLevel);
    }

    TEST_P(ASceneLoadedFromFile, failsToGetFeatureLevelFromFileWithUnknownFeatureLevel)
    {
        SaveSceneWithFeatureLevelToFile(EFeatureLevel(99), "someTemporaryFile.ram");

        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_FALSE(RamsesClient::GetFeatureLevelFromFile("someTemporaryFile.ram", featureLevel));
    }

    TEST_P(ASceneLoadedFromFile, failsToGetFeatureLevelFromNonexistingFile)
    {
        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_FALSE(RamsesClient::GetFeatureLevelFromFile("doesnt.Exist", featureLevel));
    }

    TEST_P(ASceneLoadedFromFile, canGetFeatureLevelFromSceneFileViaFileDescriptor)
    {
        SaveSceneWithFeatureLevelToFile(EFeatureLevel_Latest, "someTemporaryFile.ram");

        size_t fileSize = 0;
        {
            // write to a file with some offset
            ramses::internal::File inFile("someTemporaryFile.ram");
            EXPECT_TRUE(inFile.getSizeInBytes(fileSize));
            std::vector<unsigned char> data(fileSize);
            size_t numBytesRead = 0;
            EXPECT_TRUE(inFile.open(ramses::internal::File::Mode::ReadOnlyBinary));
            EXPECT_EQ(ramses::internal::EStatus::Ok, inFile.read(data.data(), fileSize, numBytesRead));

            ramses::internal::File outFile("someTemporaryFileWithOffset.ram");
            EXPECT_TRUE(outFile.open(ramses::internal::File::Mode::WriteOverWriteOldBinary));

            uint32_t zeroData = 0;
            EXPECT_TRUE(outFile.write(&zeroData, sizeof(zeroData)));
            EXPECT_TRUE(outFile.write(data.data(), data.size()));
            EXPECT_TRUE(outFile.write(&zeroData, sizeof(zeroData)));
        }
        const int fd = ramses::internal::FileDescriptorHelper::OpenFileDescriptorBinary("someTemporaryFileWithOffset.ram");

        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_TRUE(RamsesClient::GetFeatureLevelFromFile(fd, 4u, fileSize, featureLevel));
        EXPECT_EQ(EFeatureLevel_Latest, featureLevel);
    }

    TEST_P(ASceneLoadedFromFile, failsToGetFeatureLevelFromInvalidFileDescriptor)
    {
        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_FALSE(RamsesClient::GetFeatureLevelFromFile(-1, 0u, 10u, featureLevel));
        EXPECT_FALSE(RamsesClient::GetFeatureLevelFromFile(1, 0u, 0u, featureLevel));
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteTransformDataSlot)
    {
        Node* node = this->m_scene.createNode("node");

        EXPECT_TRUE(this->m_scene.impl().createTransformationDataConsumer(*node, dataConsumerId_t(2u)));
        ASSERT_EQ(1u, this->m_scene.impl().getIScene().getDataSlotCount());

        ramses::internal::DataSlotHandle slotHandle(0u);
        EXPECT_TRUE(this->m_scene.impl().getIScene().isDataSlotAllocated(slotHandle));

        doWriteReadCycle();

        const Node* nodeLoaded = this->getObjectForTesting<Node>("node");
        ramses::internal::NodeHandle nodeHandle = nodeLoaded->impl().getNodeHandle();
        ASSERT_EQ(1u, this->m_sceneLoaded->impl().getIScene().getDataSlotCount());

        EXPECT_TRUE(this->m_sceneLoaded->impl().getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(nodeHandle, this->m_sceneLoaded->impl().getIScene().getDataSlot(slotHandle).attachedNode);
        EXPECT_EQ(ramses::internal::DataSlotId(2u), this->m_sceneLoaded->impl().getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses::internal::EDataSlotType::TransformationConsumer, this->m_sceneLoaded->impl().getIScene().getDataSlot(slotHandle).type);
    }

    TEST_P(ASceneLoadedFromFile, canReadWriteDataObject)
    {
        float setValue = 5.0f;
        auto data = this->m_scene.createDataObject(ramses::EDataType::Float, "floatData");

        EXPECT_TRUE(data->setValue(setValue));

        doWriteReadCycle();

        const auto loadedData = this->getObjectForTesting<DataObject>("floatData");
        ASSERT_TRUE(loadedData);
        float loadedValue = 0.0f;
        EXPECT_EQ(ramses::EDataType::Float, loadedData->getDataType());
        EXPECT_TRUE(loadedData->getValue(loadedValue));
        EXPECT_EQ(setValue, loadedValue);
    }

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    TEST_P(ASceneLoadedFromFile, canReadWriteSceneReferences)
    {
        constexpr ramses::sceneId_t referencedSceneId(444);
        auto sr1 = this->m_scene.createSceneReference(referencedSceneId, "scene ref");
        sr1->requestState(RendererSceneState::Ready);

        constexpr ramses::sceneId_t referencedSceneId2(555);
        auto sr2 = this->m_scene.createSceneReference(referencedSceneId2, "scene ref2");
        sr2->requestState(RendererSceneState::Rendered);

        doWriteReadCycle();

        const ramses::SceneReference* loadedSceneRef = this->getObjectForTesting<ramses::SceneReference>("scene ref");
        ASSERT_TRUE(loadedSceneRef);
        EXPECT_EQ(referencedSceneId, loadedSceneRef->getReferencedSceneId());
        EXPECT_EQ(ramses::RendererSceneState::Ready, loadedSceneRef->getRequestedState());

        const ramses::SceneReference* loadedSceneRef2 = this->getObjectForTesting<ramses::SceneReference>("scene ref2");
        ASSERT_TRUE(loadedSceneRef2);
        EXPECT_EQ(referencedSceneId2, loadedSceneRef2->getReferencedSceneId());
        EXPECT_EQ(ramses::RendererSceneState::Rendered, loadedSceneRef2->getRequestedState());
    }
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

    TEST_P(ASceneLoadedFromFile, savesLLResourceOnlyOnceIfTwoHLResourcesReferToIt)
    {
        std::vector<uint16_t> inds(300);
        std::iota(inds.begin(), inds.end(), static_cast<uint16_t>(0u));
        this->m_scene.createArrayResource(300u, inds.data(), "indices");
        this->m_scene.createArrayResource(300u, inds.data(), "indices");
        this->m_scene.createArrayResource(300u, inds.data(), "indices");
        this->m_scene.createArrayResource(300u, inds.data(), "indices2");
        this->m_scene.createArrayResource(300u, inds.data(), "indices2");
        this->m_scene.createArrayResource(300u, inds.data(), "indices2");

        doWriteReadCycle();
        std::ifstream in("someTemporaryFile.ram", std::ifstream::ate | std::ifstream::binary);
        auto size = in.tellg();
        EXPECT_GT(1600, size) << "scene file size exceeds allowed max size. verify that LL resource is saved only once before adapting this number";
    }

    TEST_P(ASceneLoadedFromFile, compressedFileIsSmallerThanUncompressedWhenUsingSaveSceneToFile)
    {
        ramses::Scene* scene = client.createScene(SceneConfig(sceneId_t(1)));
        const std::vector<uint16_t> data(1000u, 0u);
        EXPECT_TRUE(scene->createArrayResource(static_cast<uint32_t>(data.size()), data.data()));

        SaveFileConfig saveConfig;
        saveConfig.setCompressionEnabled(false);
        EXPECT_TRUE(scene->saveToFile("testscene.ramscene", saveConfig));

        ramses::internal::File file("testscene.ramscene");
        EXPECT_TRUE(file.exists());
        size_t uncompressedFileSize = 0;
        EXPECT_TRUE(file.getSizeInBytes(uncompressedFileSize));

        saveConfig.setCompressionEnabled(true);
        EXPECT_TRUE(scene->saveToFile("testscene.ramscene", saveConfig));

        ramses::internal::File file2("testscene.ramscene");
        EXPECT_TRUE(file2.exists());
        size_t compressedFileSize = 0;
        EXPECT_TRUE(file2.getSizeInBytes(compressedFileSize));

        EXPECT_GT(uncompressedFileSize, compressedFileSize);
    }

    TEST_P(ASceneLoadedFromFile, savedFilesAreConsistent)
    {
        for (const auto& name : { "ts1.ramscene", "ts2.ramscene", "ts3.ramscene", "ts4.ramscene", "ts5.ramscene", "ts6.ramscene" })
        {
            EXPECT_TRUE(this->m_scene.saveToFile(name, {}));
        }

        for (const auto& name : { "ts2.ramscene", "ts3.ramscene", "ts4.ramscene", "ts5.ramscene", "ts6.ramscene" })
        {
            EXPECT_TRUE(ClientTestUtils::CompareBinaryFiles("ts1.ramscene", name));
        }
    }

    TEST_P(ASceneLoadedFromFile, closesSceneFileAndLowLevelResourceWhenDestroyed)
    {
        EXPECT_TRUE(m_scene.saveToFile("someTemporaryFile.ram", {}));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", {});
        ASSERT_TRUE(nullptr != m_sceneLoaded);

        auto handles = m_sceneLoaded->impl().getSceneFileHandles();
        for (const auto& handle: handles)
        {
            EXPECT_TRUE(m_clientForLoading.impl().getClientApplication().hasResourceFile(handle));
        }
        m_clientForLoading.destroy(*m_sceneLoaded);

        // scene gets destroyed asynchronously, so we can't just test after the destroy
        // unfortunately there is no callback, but I don't want to skip the test
        // => wait for it to happen in finite time, we don't test for performance here
        uint32_t ticks = 60000u;
        for (; ticks > 0 && !handles.empty(); --ticks)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            std::vector<ramses::internal::SceneFileHandle> notRemovedHandles;
            notRemovedHandles.reserve(handles.size());
            for (const auto& handle: handles)
            {
                if (m_clientForLoading.impl().getClientApplication().hasResourceFile(handle))
                {
                    notRemovedHandles.push_back(handle);
                }
            }
            handles.swap(notRemovedHandles);
        }
        EXPECT_GT(ticks, 0u);
    }

    TEST_P(ASceneLoadedFromFile, closesSceneFileAndLowLevelResourceOfMergedSceneWhenDestroyed)
    {
        EXPECT_TRUE(m_scene.saveToFile("someTemporaryFile.ram", {}));
        EXPECT_TRUE(m_scene.saveToFile("someTemporaryFile2.ram", {}));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", {});
        ASSERT_TRUE(nullptr != m_sceneLoaded);

        bool success = m_clientForLoading.mergeSceneFromFile(*m_sceneLoaded, "someTemporaryFile2.ram");
        // scene merge is supported starting from feature level 02
        if (GetParam() < EFeatureLevel::EFeatureLevel_02)
        {
            EXPECT_FALSE(success);
            return;
        }
        ASSERT_TRUE(success);

        auto handles = m_sceneLoaded->impl().getSceneFileHandles();
        EXPECT_EQ(2, handles.size());

        for (const auto& handle: handles)
        {
            EXPECT_TRUE(m_clientForLoading.impl().getClientApplication().hasResourceFile(handle));
        }
        m_clientForLoading.destroy(*m_sceneLoaded);

        // scene gets destroyed asynchronously, so we can't just test after the destroy
        // unfortunately there is no callback, but I don't want to skip the test
        // => wait for it to happen in finite time, we don't test for performance here
        uint32_t ticks = 60000u;
        for (; ticks > 0 && !handles.empty(); --ticks)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            std::vector<ramses::internal::SceneFileHandle> notRemovedHandles;
            notRemovedHandles.reserve(handles.size());
            for (const auto& handle: handles)
            {
                if (m_clientForLoading.impl().getClientApplication().hasResourceFile(handle))
                {
                    notRemovedHandles.push_back(handle);
                }
            }
            handles.swap(notRemovedHandles);
        }
        EXPECT_GT(ticks, 0u);
    }

    TEST_P(ASceneLoadedFromFile, closesSceneFileAndLowLevelResourceOfSameMergedSceneWhenDestroyed)
    {
        EXPECT_TRUE(m_scene.saveToFile("someTemporaryFile.ram", {}));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", {});
        ASSERT_TRUE(nullptr != m_sceneLoaded);

        bool success = m_clientForLoading.mergeSceneFromFile(*m_sceneLoaded, "someTemporaryFile.ram");
        // scene merge is supported starting from feature level 02
        if (GetParam() < EFeatureLevel::EFeatureLevel_02)
        {
            EXPECT_FALSE(success);
            return;
        }
        ASSERT_TRUE(success);

        auto handles = m_sceneLoaded->impl().getSceneFileHandles();
        EXPECT_EQ(2, handles.size());

        for (const auto& handle: handles)
        {
            EXPECT_TRUE(m_clientForLoading.impl().getClientApplication().hasResourceFile(handle));
        }
        m_clientForLoading.destroy(*m_sceneLoaded);

        // scene gets destroyed asynchronously, so we can't just test after the destroy
        // unfortunately there is no callback, but I don't want to skip the test
        // => wait for it to happen in finite time, we don't test for performance here
        uint32_t ticks = 60000u;
        for (; ticks > 0 && !handles.empty(); --ticks)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            std::vector<ramses::internal::SceneFileHandle> notRemovedHandles;
            notRemovedHandles.reserve(handles.size());
            for (const auto& handle: handles)
            {
                if (m_clientForLoading.impl().getClientApplication().hasResourceFile(handle))
                {
                    notRemovedHandles.push_back(handle);
                }
            }
            handles.swap(notRemovedHandles);
        }
        EXPECT_GT(ticks, 0u);
    }

    template <typename T>
    class ASceneLoadedFromFileNodesTest : public SceneLoadedFromFile, public ::testing::Test
    {
    public:
        ASceneLoadedFromFileNodesTest()
            : SceneLoadedFromFile{ EFeatureLevel_Latest }
        {
        }
    };

    TYPED_TEST_SUITE(ASceneLoadedFromFileNodesTest, NodeTypes);
    TYPED_TEST(ASceneLoadedFromFileNodesTest, canReadWriteAllNodes)
    {
        auto node = &this->template createObject<TypeParam>("a node");

        node->setVisibility(EVisibilityMode::Invisible);

        auto child = &this->template createObject<ramses::Node>("child");
        auto parent = &this->template createObject<ramses::Node>("parent");

        node->setTranslation({ 1, 2, 3 });
        node->setRotation({ 4, 5, 6 }, ERotationType::Euler_XZX);
        node->setScaling({ 7, 8, 9 });
        node->addChild(*child);
        node->setParent(*parent);

        this->m_scene.flush();

        this->doWriteReadCycle();

        const auto loadedSuperNode = this->template getObjectForTesting<TypeParam>("a node");
        const auto loadedChild = this->template getObjectForTesting<ramses::Node>("child");
        const auto loadedParent = this->template getObjectForTesting<ramses::Node>("parent");

        ASSERT_TRUE(nullptr != loadedSuperNode);
        ASSERT_TRUE(nullptr != loadedChild);
        ASSERT_TRUE(nullptr != loadedParent);

        ASSERT_EQ(1u, loadedSuperNode->getChildCount());
        EXPECT_EQ(loadedChild, loadedSuperNode->getChild(0u));
        EXPECT_EQ(loadedParent, loadedSuperNode->getParent());
        vec3f value;
        EXPECT_TRUE(loadedSuperNode->getTranslation(value));
        EXPECT_FLOAT_EQ(1, value.x);
        EXPECT_FLOAT_EQ(2, value.y);
        EXPECT_FLOAT_EQ(3, value.z);
        EXPECT_TRUE(loadedSuperNode->getRotation(value));
        EXPECT_FLOAT_EQ(4, value.x);
        EXPECT_FLOAT_EQ(5, value.y);
        EXPECT_FLOAT_EQ(6, value.z);
        EXPECT_EQ(ERotationType::Euler_XZX, loadedSuperNode->getRotationType());
        EXPECT_TRUE(loadedSuperNode->getScaling(value));
        EXPECT_FLOAT_EQ(7, value.x);
        EXPECT_FLOAT_EQ(8, value.y);
        EXPECT_FLOAT_EQ(9, value.z);

        EXPECT_EQ(loadedSuperNode->getVisibility(), EVisibilityMode::Invisible);
    }
}
