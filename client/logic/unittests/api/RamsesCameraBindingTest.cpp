//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

#include "RamsesObjectResolverMock.h"
#include "RamsesTestUtils.h"
#include "SerializationTestUtils.h"
#include "WithTempDirectory.h"

#include "impl/LogicEngineImpl.h"
#include "impl/RamsesCameraBindingImpl.h"
#include "impl/PropertyImpl.h"
#include "internals/RamsesHelper.h"
#include "generated/RamsesCameraBindingGen.h"

#include "ramses-logic/RamsesCameraBinding.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/RamsesCameraBinding.h"

#include "ramses-utils.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"

namespace rlogic::internal
{
    constexpr int32_t DefaultViewportOffsetX = 0;
    constexpr int32_t DefaultViewportOffsetY = 0;
    constexpr uint32_t DefaultViewportWidth = 16u;
    constexpr uint32_t DefaultViewportHeight = 16u;

    constexpr float NearPlaneDefault = 0.1f;
    constexpr float FarPlaneDefault = 1.0f;

    constexpr float PerspectiveFrustumFOVdefault = 168.579f;
    constexpr float PerspectiveFrustumARdefault = 1.f;

    constexpr float OrthoFrustumLPdefault = -1.f;
    constexpr float OrthoFrustumRPdefault = 1.f;
    constexpr float OrthoFrustumBPdefault = -1.f;
    constexpr float OrthoFrustumTPdefault = 1.0f;

    class ARamsesCameraBinding : public ALogicEngine
    {
    protected:
        ARamsesCameraBinding()
            : m_testScene(*m_ramsesTestSetup.createScene(ramses::sceneId_t(1)))
        {
        }

        static void ExpectPropertyTypeAndChildCount(const Property* prop, EPropertyType type, uint32_t childCount)
        {
            ASSERT_NE(nullptr, prop);
            EXPECT_EQ(type, prop->getType());
            EXPECT_EQ(childCount, prop->getChildCount());
        }

        static void ExpectDefaultViewportValues(const ramses::Camera& camera)
        {
            EXPECT_EQ(camera.getViewportX(), DefaultViewportOffsetX);
            EXPECT_EQ(camera.getViewportY(), DefaultViewportOffsetY);
            EXPECT_EQ(camera.getViewportWidth(), DefaultViewportWidth);
            EXPECT_EQ(camera.getViewportHeight(), DefaultViewportHeight);
        }

        static void ExpectDefaultPerspectiveCameraFrustumValues(const ramses::PerspectiveCamera& camera)
        {
            EXPECT_NEAR(camera.getVerticalFieldOfView(), PerspectiveFrustumFOVdefault, 0.001f);
            EXPECT_EQ(camera.getAspectRatio(), PerspectiveFrustumARdefault);
            EXPECT_EQ(camera.getNearPlane(), NearPlaneDefault);
            EXPECT_EQ(camera.getFarPlane(), FarPlaneDefault);
        }

        static void ExpectDefaultCameraFrustumPlanes(const ramses::Camera& camera)
        {
            EXPECT_EQ(camera.getLeftPlane(), OrthoFrustumLPdefault);
            EXPECT_EQ(camera.getRightPlane(), OrthoFrustumRPdefault);
            EXPECT_EQ(camera.getBottomPlane(), OrthoFrustumBPdefault);
            EXPECT_EQ(camera.getTopPlane(), OrthoFrustumTPdefault);
            EXPECT_EQ(camera.getNearPlane(), NearPlaneDefault);
            EXPECT_EQ(camera.getFarPlane(), FarPlaneDefault);
        }

        static void ExpectDefaultValues(const rlogic::RamsesCameraBinding& cameraBinding)
        {
            ExpectDefaultViewportValues(cameraBinding.getRamsesCamera());
            if (cameraBinding.m_cameraBinding.hasFrustumPlanesProperties())
            {
                ExpectDefaultCameraFrustumPlanes(cameraBinding.getRamsesCamera());
            }
            else
            {
                ExpectDefaultPerspectiveCameraFrustumValues(*ramses::RamsesUtils::TryConvert<ramses::PerspectiveCamera>(cameraBinding.getRamsesCamera()));
            }
        }

        static void ExpectInputPropertiesWithFrustumPlanes(const RamsesCameraBindingImpl& cameraBinding)
        {
            EXPECT_TRUE(cameraBinding.hasFrustumPlanesProperties());

            const auto inputs = cameraBinding.getInputs();
            ASSERT_EQ(2u, inputs->getChildCount());

            const auto vpProperties = inputs->getChild("viewport");
            const auto frustum = inputs->getChild("frustum");
            ASSERT_EQ(4u, vpProperties->getChildCount());
            ASSERT_EQ(6u, frustum->getChildCount());

            const auto vpOffsetX = vpProperties->getChild("offsetX");
            const auto vpOffsety = vpProperties->getChild("offsetY");
            const auto vpWidth = vpProperties->getChild("width");
            const auto vpHeight = vpProperties->getChild("height");
            const auto nP = frustum->getChild("nearPlane");
            const auto fP = frustum->getChild("farPlane");
            const auto lp = frustum->getChild("leftPlane");
            const auto rP = frustum->getChild("rightPlane");
            const auto bP = frustum->getChild("bottomPlane");
            const auto tP = frustum->getChild("topPlane");

            // Test that internal indices match properties resolved by name
            EXPECT_EQ(vpProperties, inputs->getChild(static_cast<size_t>(ECameraPropertyStructStaticIndex::Viewport)));
            EXPECT_EQ(frustum, inputs->getChild(static_cast<size_t>(ECameraPropertyStructStaticIndex::Frustum)));

            EXPECT_EQ(vpOffsetX, vpProperties->m_impl->getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetX)));
            EXPECT_EQ(vpOffsety, vpProperties->m_impl->getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetY)));
            EXPECT_EQ(vpWidth, vpProperties->m_impl->getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortWidth)));
            EXPECT_EQ(vpHeight, vpProperties->m_impl->getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortHeight)));
            EXPECT_EQ(nP, frustum->m_impl->getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::NearPlane)));
            EXPECT_EQ(fP, frustum->m_impl->getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::FarPlane)));
            EXPECT_EQ(lp, frustum->m_impl->getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::LeftPlane)));
            EXPECT_EQ(rP, frustum->m_impl->getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::RightPlane)));
            EXPECT_EQ(bP, frustum->m_impl->getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::BottomPlane)));
            EXPECT_EQ(tP, frustum->m_impl->getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::TopPlane)));

            ExpectPropertyTypeAndChildCount(inputs->getChild("viewport"), EPropertyType::Struct, 4);
            ExpectPropertyTypeAndChildCount(inputs->getChild("frustum"), EPropertyType::Struct, 6);

            ExpectPropertyTypeAndChildCount(vpProperties->getChild("offsetX"), EPropertyType::Int32, 0);
            ExpectPropertyTypeAndChildCount(vpProperties->getChild("offsetY"), EPropertyType::Int32, 0);
            ExpectPropertyTypeAndChildCount(vpProperties->getChild("width"), EPropertyType::Int32, 0);
            ExpectPropertyTypeAndChildCount(vpProperties->getChild("height"), EPropertyType::Int32, 0);
            ExpectPropertyTypeAndChildCount(frustum->getChild("nearPlane"), EPropertyType::Float, 0);
            ExpectPropertyTypeAndChildCount(frustum->getChild("farPlane"), EPropertyType::Float, 0);
            ExpectPropertyTypeAndChildCount(frustum->getChild("leftPlane"), EPropertyType::Float, 0);
            ExpectPropertyTypeAndChildCount(frustum->getChild("rightPlane"), EPropertyType::Float, 0);
            ExpectPropertyTypeAndChildCount(frustum->getChild("bottomPlane"), EPropertyType::Float, 0);
            ExpectPropertyTypeAndChildCount(frustum->getChild("topPlane"), EPropertyType::Float, 0);
        }

        static void ExpectInputPropertiesWithoutFrustumPlanes(const RamsesCameraBindingImpl& cameraBinding)
        {
            EXPECT_FALSE(cameraBinding.hasFrustumPlanesProperties());

            const auto inputs = cameraBinding.getInputs();
            ASSERT_EQ(2u, inputs->getChildCount());

            const auto vpProperties = inputs->getChild("viewport");
            const auto frustum = inputs->getChild("frustum");
            ASSERT_EQ(4u, vpProperties->getChildCount());
            ASSERT_EQ(4u, frustum->getChildCount());

            const auto vpOffsetX = vpProperties->getChild("offsetX");
            const auto vpOffsety = vpProperties->getChild("offsetY");
            const auto vpWidth = vpProperties->getChild("width");
            const auto vpHeight = vpProperties->getChild("height");

            const auto nP = frustum->getChild("nearPlane");
            const auto fP = frustum->getChild("farPlane");
            const auto fov = frustum->getChild("fieldOfView");
            const auto aR = frustum->getChild("aspectRatio");

            // Test that internal indices match properties resolved by name
            EXPECT_EQ(vpProperties, inputs->getChild(static_cast<size_t>(ECameraPropertyStructStaticIndex::Viewport)));
            EXPECT_EQ(frustum, inputs->getChild(static_cast<size_t>(ECameraPropertyStructStaticIndex::Frustum)));

            EXPECT_EQ(vpOffsetX, vpProperties->getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetX)));
            EXPECT_EQ(vpOffsety, vpProperties->getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetY)));
            EXPECT_EQ(vpWidth, vpProperties->getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortWidth)));
            EXPECT_EQ(vpHeight, vpProperties->getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortHeight)));
            EXPECT_EQ(nP, frustum->m_impl->getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::NearPlane)));
            EXPECT_EQ(fP, frustum->m_impl->getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::FarPlane)));
            EXPECT_EQ(fov, frustum->m_impl->getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::FieldOfView)));
            EXPECT_EQ(aR, frustum->m_impl->getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::AspectRatio)));

            ExpectPropertyTypeAndChildCount(inputs->getChild("viewport"), EPropertyType::Struct, 4);
            ExpectPropertyTypeAndChildCount(inputs->getChild("frustum"), EPropertyType::Struct, 4);

            ExpectPropertyTypeAndChildCount(vpProperties->getChild("offsetX"), EPropertyType::Int32, 0);
            ExpectPropertyTypeAndChildCount(vpProperties->getChild("offsetY"), EPropertyType::Int32, 0);
            ExpectPropertyTypeAndChildCount(vpProperties->getChild("width"), EPropertyType::Int32, 0);
            ExpectPropertyTypeAndChildCount(vpProperties->getChild("height"), EPropertyType::Int32, 0);
            ExpectPropertyTypeAndChildCount(frustum->getChild("nearPlane"), EPropertyType::Float, 0);
            ExpectPropertyTypeAndChildCount(frustum->getChild("farPlane"), EPropertyType::Float, 0);
            ExpectPropertyTypeAndChildCount(frustum->getChild("fieldOfView"), EPropertyType::Float, 0);
            ExpectPropertyTypeAndChildCount(frustum->getChild("aspectRatio"), EPropertyType::Float, 0);
        }

        RamsesTestSetup m_ramsesTestSetup;
        ramses::Scene& m_testScene;
        LogicEngine m_logicEngine;
        ramses::OrthographicCamera& m_orthoCam = { *m_testScene.createOrthographicCamera() };
        ramses::PerspectiveCamera& m_perspectiveCam = { *m_testScene.createPerspectiveCamera() };
    };

    TEST_F(ARamsesCameraBinding, HasANameAfterCreation)
    {
        auto& cameraBinding = *m_logicEngine.createRamsesCameraBinding(*m_camera, "CameraBinding");
        EXPECT_EQ("CameraBinding", cameraBinding.getName());
    }

    TEST_F(ARamsesCameraBinding, HasAIdAfterCreation)
    {
        auto& cameraBinding = *m_logicEngine.createRamsesCameraBinding(*m_camera, "CameraBinding");
        EXPECT_EQ(cameraBinding.getId(), 1u);
    }

    TEST_F(ARamsesCameraBinding, HasNoOutputsAfterCreation)
    {
        auto& cameraBinding = *m_logicEngine.createRamsesCameraBinding(*m_camera, "");
        EXPECT_EQ(nullptr, cameraBinding.getOutputs());
    }

    TEST_F(ARamsesCameraBinding, ProducesNoErrorsDuringUpdate_IfNoRamsesCameraIsAssigned)
    {
        auto& cameraBinding = *m_logicEngine.createRamsesCameraBinding(*m_camera, "");
        EXPECT_EQ(std::nullopt, cameraBinding.m_impl.update());
    }

    TEST_F(ARamsesCameraBinding, ReturnsReferenceToRamsesCamera)
    {
        RamsesCameraBinding& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");
        EXPECT_EQ(&m_perspectiveCam, &cameraBinding.getRamsesCamera());
    }

    TEST_F(ARamsesCameraBinding, HasInputsAfterInitializingWithPerspectiveCamera)
    {
        RamsesCameraBinding& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");
        ExpectInputPropertiesWithoutFrustumPlanes(cameraBinding.m_cameraBinding);
    }

    TEST_F(ARamsesCameraBinding, HasInputsAfterInitializingFromOrthoCamera)
    {
        RamsesCameraBinding& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_orthoCam, "");
        ExpectInputPropertiesWithFrustumPlanes(cameraBinding.m_cameraBinding);
    }

    TEST_F(ARamsesCameraBinding, HasInputsAfterInitializingFromOrthoCamera_createdWithFrustumPlanes)
    {
        LogicEngine engineFeature02{ EFeatureLevel_02 };
        RamsesCameraBinding& cameraBinding = *engineFeature02.createRamsesCameraBindingWithFrustumPlanes(m_orthoCam);
        ExpectInputPropertiesWithFrustumPlanes(cameraBinding.m_cameraBinding);
    }

    TEST_F(ARamsesCameraBinding, HasInputsAfterInitializingFromPerspectiveCamera_createdWithFrustumPlanes)
    {
        LogicEngine engineFeature02{ EFeatureLevel_02 };
        RamsesCameraBinding& cameraBinding = *engineFeature02.createRamsesCameraBindingWithFrustumPlanes(m_perspectiveCam);
        ExpectInputPropertiesWithFrustumPlanes(cameraBinding.m_cameraBinding);
    }

    TEST_F(ARamsesCameraBinding, FailsToBeCreatedWithFrustumPlanesOnFeatureLevel01)
    {
        EXPECT_EQ(nullptr, m_logicEngine.createRamsesCameraBindingWithFrustumPlanes(m_orthoCam));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create RamsesCameraBinding with frustum planes properties, feature level 02 or higher is required, feature level in this runtime set to 01.");

        EXPECT_EQ(nullptr, m_logicEngine.createRamsesCameraBindingWithFrustumPlanes(m_perspectiveCam));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create RamsesCameraBinding with frustum planes properties, feature level 02 or higher is required, feature level in this runtime set to 01.");
    }

    TEST_F(ARamsesCameraBinding, DoesNotOverwriteDefaultValues_WhenCreatedFromOrthoCamera)
    {
        m_logicEngine.createRamsesCameraBinding(m_orthoCam, "");
        m_logicEngine.update();

        //Expect default values on the camera, because nothing was set so far
        ExpectDefaultViewportValues(m_orthoCam);
        ExpectDefaultCameraFrustumPlanes(m_orthoCam);
    }

    TEST_F(ARamsesCameraBinding, DoesNotOverwriteDefaultValues_WhenCreatedFromPerspectiveCamera)
    {
        m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");
        m_logicEngine.update();

        //Expect default values on the camera, because nothing was set so far
        ExpectDefaultViewportValues(m_perspectiveCam);
        ExpectDefaultPerspectiveCameraFrustumValues(m_perspectiveCam);
    }

    TEST_F(ARamsesCameraBinding, ReportsErrorOnUpdate_WhenSettingZeroToViewportSize)
    {
        RamsesCameraBinding& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_orthoCam, "");
        const auto inputs = cameraBinding.getInputs();
        auto vpProperties = inputs->getChild("viewport");
        // Setting illegal viewport values: width and height cannot be 0 so an error will be produced on ramses camera
        vpProperties->getChild("width")->set<int32_t>(0);

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Camera viewport size must be positive! (width: 0; height: 16)");

        // Fix width, break height -> still generates error
        vpProperties->getChild("width")->set<int32_t>(8);
        vpProperties->getChild("height")->set<int32_t>(0);

        // Expect default values on the camera, because setting values failed
        ExpectDefaultViewportValues(m_orthoCam);

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Camera viewport size must be positive! (width: 8; height: 0)");

        // Fix height and update recovers the errors
        vpProperties->getChild("height")->set<int32_t>(32);
        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_EQ(m_orthoCam.getViewportWidth(), 8u);
        EXPECT_EQ(m_orthoCam.getViewportHeight(), 32u);
    }

    TEST_F(ARamsesCameraBinding, ReportsErrorOnUpdate_WhenSettingNegativeViewportSize)
    {
        RamsesCameraBinding& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_orthoCam, "");
        const auto inputs = cameraBinding.getInputs();
        auto vpProperties = inputs->getChild("viewport");
        // Setting illegal viewport values: width and height cannot be 0 so an error will be produced on ramses camera
        vpProperties->getChild("width")->set<int32_t>(-1);
        vpProperties->getChild("height")->set<int32_t>(-1);

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Camera viewport size must be positive! (width: -1; height: -1)");

        // Setting positive values recovers from the error
        vpProperties->getChild("width")->set<int32_t>(10);
        vpProperties->getChild("height")->set<int32_t>(12);
        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_EQ(m_orthoCam.getViewportWidth(), 10u);
        EXPECT_EQ(m_orthoCam.getViewportHeight(), 12u);
    }

    TEST_F(ARamsesCameraBinding, ReportsErrorOnUpdate_WhenSettingInvalidFrustumValuesOnOrthoCamera)
    {
        RamsesCameraBinding& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_orthoCam, "");

        auto frustum = cameraBinding.getInputs()->getChild("frustum");
        // Setting illegal frustum values: left plane cannot be smaller than right plane so an error will be produced on ramses camera
        frustum->getChild("leftPlane")->set<float>(2.f);
        frustum->getChild("rightPlane")->set<float>(1.f);

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Camera::setFrustum failed - check validity of given frustum planes");

        //Still expect default values on the camera, because setting values failed
        ExpectDefaultCameraFrustumPlanes(m_orthoCam);

        // Recovers from the error once values are ok
        frustum->getChild("rightPlane")->set<float>(3.f);
        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ARamsesCameraBinding, ReportsErrorOnUpdate_WhenSettingInvalidFrustumValuesOnPerspectiveCamera)
    {
        RamsesCameraBinding& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");

        auto frustum = cameraBinding.getInputs()->getChild("frustum");
        // Setting illegal frustum values: fov and aspect ratio cannot be 0 so an error will be produced on ramses camera
        frustum->getChild("fieldOfView")->set<float>(0.f);
        frustum->getChild("aspectRatio")->set<float>(0.f);

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "PerspectiveCamera::setFrustum failed - check validity of given frustum planes");

        // Fixing just the FOV does not fix the issue, need to also fix aspect ratio
        frustum->getChild("fieldOfView")->set<float>(15.f);
        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "PerspectiveCamera::setFrustum failed - check validity of given frustum planes");

        //Still expect default values on the camera, because setting values failed
        ExpectDefaultViewportValues(m_perspectiveCam);
        ExpectDefaultPerspectiveCameraFrustumValues(m_perspectiveCam);

        frustum->getChild("aspectRatio")->set<float>(1.f);
        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ARamsesCameraBinding, InitializesInputPropertiesOfPerpespectiveCameraToMatchRamsesDefaultValues)
    {
        RamsesCameraBinding& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");

        const auto inputs = cameraBinding.getInputs();
        ASSERT_NE(nullptr, inputs);

        const auto vpProperties = inputs->getChild("viewport");
        const auto frustum = inputs->getChild("frustum");
        ASSERT_EQ(4u, vpProperties->getChildCount());
        ASSERT_EQ(4u, frustum->getChildCount());

        EXPECT_EQ(*vpProperties->getChild("offsetX")->get<int32_t>(), m_perspectiveCam.getViewportX());
        EXPECT_EQ(*vpProperties->getChild("offsetY")->get<int32_t>(), m_perspectiveCam.getViewportY());
        EXPECT_EQ(static_cast<uint32_t>(*vpProperties->getChild("width")->get<int32_t>()), m_perspectiveCam.getViewportWidth());
        EXPECT_EQ(static_cast<uint32_t>(*vpProperties->getChild("height")->get<int32_t>()), m_perspectiveCam.getViewportHeight());

        EXPECT_EQ(*frustum->getChild("nearPlane")->get<float>(), m_perspectiveCam.getNearPlane());
        EXPECT_EQ(*frustum->getChild("farPlane")->get<float>(), m_perspectiveCam.getFarPlane());
        EXPECT_NEAR(*frustum->getChild("fieldOfView")->get<float>(), m_perspectiveCam.getVerticalFieldOfView(), 0.001f);
        EXPECT_EQ(*frustum->getChild("aspectRatio")->get<float>(), m_perspectiveCam.getAspectRatio());
    }

    TEST_F(ARamsesCameraBinding, InitializesInputPropertiesOfOrthographicCameraToMatchRamsesDefaultValues)
    {
        RamsesCameraBinding& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_orthoCam, "");

        const auto inputs = cameraBinding.getInputs();
        ASSERT_NE(nullptr, inputs);

        const auto vpProperties = inputs->getChild("viewport");
        const auto frustum = inputs->getChild("frustum");
        ASSERT_EQ(4u, vpProperties->getChildCount());
        ASSERT_EQ(6u, frustum->getChildCount());

        EXPECT_EQ(*vpProperties->getChild("offsetX")->get<int32_t>(), m_orthoCam.getViewportX());
        EXPECT_EQ(*vpProperties->getChild("offsetY")->get<int32_t>(), m_orthoCam.getViewportY());
        EXPECT_EQ(static_cast<uint32_t>(*vpProperties->getChild("width")->get<int32_t>()), m_orthoCam.getViewportWidth());
        EXPECT_EQ(static_cast<uint32_t>(*vpProperties->getChild("height")->get<int32_t>()), m_orthoCam.getViewportHeight());

        EXPECT_EQ(*frustum->getChild("nearPlane")->get<float>(), m_orthoCam.getNearPlane());
        EXPECT_EQ(*frustum->getChild("farPlane")->get<float>(), m_orthoCam.getFarPlane());
        EXPECT_EQ(*frustum->getChild("leftPlane")->get<float>(), m_orthoCam.getLeftPlane());
        EXPECT_EQ(*frustum->getChild("rightPlane")->get<float>(), m_orthoCam.getRightPlane());
        EXPECT_EQ(*frustum->getChild("bottomPlane")->get<float>(), m_orthoCam.getBottomPlane());
        EXPECT_EQ(*frustum->getChild("topPlane")->get<float>(), m_orthoCam.getTopPlane());
    }

    TEST_F(ARamsesCameraBinding, MarksInputsAsBindingInputsForPerspectiveCameraBinding)
    {
        auto* cameraBinding = m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");

        const auto inputs = cameraBinding->getInputs();
        for (size_t i = 0; i < inputs->getChildCount(); ++i)
        {
            const auto inputStruct = inputs->getChild(i);
            EXPECT_EQ(EPropertySemantics::BindingInput, inputStruct->m_impl->getPropertySemantics());

            for (size_t j = 0; j < inputs->getChild(i)->getChildCount(); ++j)
            {
                const auto inputProperty = inputStruct->m_impl->getChild(j);
                EXPECT_EQ(EPropertySemantics::BindingInput, inputProperty->m_impl->getPropertySemantics());
            }
        }
    }

    TEST_F(ARamsesCameraBinding, MarksInputsAsBindingInputsForOrthoCameraBinding)
    {
        auto* cameraBinding = m_logicEngine.createRamsesCameraBinding(m_orthoCam, "");

        const auto inputs = cameraBinding->getInputs();
        for (size_t i = 0; i < inputs->getChildCount(); ++i)
        {
            const auto inputStruct = inputs->getChild(i);
            EXPECT_EQ(EPropertySemantics::BindingInput, inputStruct->m_impl->getPropertySemantics());

            for (size_t j = 0; j < inputs->getChild(i)->getChildCount(); ++j)
            {
                const auto inputProperty = inputStruct->m_impl->getChild(j);
                EXPECT_EQ(EPropertySemantics::BindingInput, inputProperty->m_impl->getPropertySemantics());
            }
        }
    }

    TEST_F(ARamsesCameraBinding, ReturnsBoundRamsesCamera)
    {
        auto* cameraBinding = m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");

        EXPECT_EQ(&m_perspectiveCam, &cameraBinding->getRamsesCamera());
    }

    TEST_F(ARamsesCameraBinding, DoesNotModifyRamsesWithoutUpdateBeingCalledWithPerspectiveCamera)
    {
        auto* cameraBinding = m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");

        auto inputs = cameraBinding->getInputs();
        auto vpProperties = inputs->getChild("viewport");
        auto frustum = inputs->getChild("frustum");

        vpProperties->getChild("offsetX")->set<int32_t>(4);
        vpProperties->getChild("offsetY")->set<int32_t>(7);
        vpProperties->getChild("width")->set<int32_t>(11);
        vpProperties->getChild("height")->set<int32_t>(19);

        frustum->getChild("nearPlane")->set<float>(3.1f);
        frustum->getChild("farPlane")->set<float>(.2f);
        frustum->getChild("fieldOfView")->set<float>(4.2f);
        frustum->getChild("aspectRatio")->set<float>(2.1f);

        ExpectDefaultValues(*cameraBinding);
    }

    TEST_F(ARamsesCameraBinding, DoesNotModifyRamsesWithoutUpdateBeingCalledWithOrthoCamera)
    {
        auto* cameraBinding = m_logicEngine.createRamsesCameraBinding(m_orthoCam, "");

        auto inputs = cameraBinding->getInputs();
        auto vpProperties = inputs->getChild("viewport");
        auto frustum = inputs->getChild("frustum");

        vpProperties->getChild("offsetX")->set<int32_t>(4);
        vpProperties->getChild("offsetY")->set<int32_t>(7);
        vpProperties->getChild("width")->set<int32_t>(11);
        vpProperties->getChild("height")->set<int32_t>(19);

        frustum->getChild("nearPlane")->set<float>(3.1f);
        frustum->getChild("farPlane")->set<float>(.2f);
        frustum->getChild("leftPlane")->set<float>(6.2f);
        frustum->getChild("rightPlane")->set<float>(2.8f);
        frustum->getChild("bottomPlane")->set<float>(1.9f);
        frustum->getChild("topPlane")->set<float>(7.1f);

        ExpectDefaultValues(*cameraBinding);
    }

    TEST_F(ARamsesCameraBinding, ModifiesRamsesPerspectiveCamOnUpdate_OnlyAfterExplicitlyAssignedToInputs)
    {
        auto* cameraBinding = m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");

        auto inputs = cameraBinding->getInputs();
        auto vpProperties = inputs->getChild("viewport");
        auto frustum = inputs->getChild("frustum");

        const int32_t newVpOffsetX = 23;
        vpProperties->getChild("offsetX")->set<int32_t>(newVpOffsetX);

        // Update not called yet -> still default values
        ExpectDefaultValues(*cameraBinding);

        cameraBinding->m_cameraBinding.update();
        // Only propagated vpOffsetX, the others have default values
        EXPECT_EQ(m_perspectiveCam.getViewportX(), newVpOffsetX);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 0);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 16u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 16u);

        ExpectDefaultPerspectiveCameraFrustumValues(m_perspectiveCam);

        // Set and test all properties
        const int32_t newVpOffsetY = 13;
        const int32_t newVpWidth = 56;
        const int32_t newVpHeight = 45;

        const float newFov = 30.f;
        const float newAR = 640.f / 480.f;
        const float newNearPlane = 4.4f;
        const float newFarPlane = 5.1f;

        vpProperties->getChild("offsetY")->set<int32_t>(newVpOffsetY);
        vpProperties->getChild("width")->set<int32_t>(newVpWidth);
        vpProperties->getChild("height")->set<int32_t>(newVpHeight);

        frustum->getChild("fieldOfView")->set<float>(newFov);
        frustum->getChild("aspectRatio")->set<float>(newAR);
        frustum->getChild("nearPlane")->set<float>(newNearPlane);
        frustum->getChild("farPlane")->set<float>(newFarPlane);
        cameraBinding->m_cameraBinding.update();

        EXPECT_EQ(m_perspectiveCam.getViewportX(), newVpOffsetX);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), newVpOffsetY);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), static_cast<uint32_t>(newVpWidth));
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), static_cast<uint32_t>(newVpHeight));

        EXPECT_NEAR(m_perspectiveCam.getVerticalFieldOfView(), newFov, 0.001f);
        EXPECT_EQ(m_perspectiveCam.getAspectRatio(), newAR);
        EXPECT_EQ(m_perspectiveCam.getNearPlane(), newNearPlane);
        EXPECT_EQ(m_perspectiveCam.getFarPlane(), newFarPlane);
    }

    TEST_F(ARamsesCameraBinding, ModifiesRamsesOrthoCamOnUpdate_OnlyAfterExplicitlyAssignedToInputs)
    {
        auto* cameraBinding = m_logicEngine.createRamsesCameraBinding(m_orthoCam, "");

        auto inputs = cameraBinding->getInputs();
        auto vpProperties = inputs->getChild("viewport");
        auto frustum = inputs->getChild("frustum");

        const int32_t newVpOffsetX = 23;
        vpProperties->getChild("offsetX")->set<int32_t>(newVpOffsetX);

        // Update not called yet -> still default values
        ExpectDefaultValues(*cameraBinding);

        cameraBinding->m_cameraBinding.update();
        // Only propagated vpOffsetX, the others have default values
        EXPECT_EQ(m_orthoCam.getViewportX(), newVpOffsetX);
        EXPECT_EQ(m_orthoCam.getViewportY(), 0);
        EXPECT_EQ(m_orthoCam.getViewportWidth(), 16u);
        EXPECT_EQ(m_orthoCam.getViewportHeight(), 16u);

        ExpectDefaultCameraFrustumPlanes(m_orthoCam);

        // Set and test all properties
        const int32_t newVpOffsetY = 13;
        const int32_t newVpWidth = 56;
        const int32_t newVpHeight = 45;

        const float newLeftPlane = 0.2f;
        const float newRightPlane = 0.3f;
        const float newBottomPlane = 0.4f;
        const float newTopPlane = 0.5f;
        const float newNearPlane = 4.f;
        const float newFarPlane = 5.1f;

        vpProperties->getChild("offsetY")->set<int32_t>(newVpOffsetY);
        vpProperties->getChild("width")->set<int32_t>(newVpWidth);
        vpProperties->getChild("height")->set<int32_t>(newVpHeight);

        frustum->getChild("leftPlane")->set<float>(newLeftPlane);
        frustum->getChild("rightPlane")->set<float>(newRightPlane);
        frustum->getChild("bottomPlane")->set<float>(newBottomPlane);
        frustum->getChild("topPlane")->set<float>(newTopPlane);
        frustum->getChild("nearPlane")->set<float>(newNearPlane);
        frustum->getChild("farPlane")->set<float>(newFarPlane);
        cameraBinding->m_cameraBinding.update();

        EXPECT_EQ(m_orthoCam.getViewportX(), newVpOffsetX);
        EXPECT_EQ(m_orthoCam.getViewportY(), newVpOffsetY);
        EXPECT_EQ(m_orthoCam.getViewportWidth(), static_cast<uint32_t>(newVpWidth));
        EXPECT_EQ(m_orthoCam.getViewportHeight(), static_cast<uint32_t>(newVpHeight));

        EXPECT_EQ(m_orthoCam.getLeftPlane(), newLeftPlane);
        EXPECT_EQ(m_orthoCam.getRightPlane(), newRightPlane);
        EXPECT_EQ(m_orthoCam.getBottomPlane(), newBottomPlane);
        EXPECT_EQ(m_orthoCam.getTopPlane(), newTopPlane);
        EXPECT_EQ(m_orthoCam.getNearPlane(), newNearPlane);
        EXPECT_EQ(m_orthoCam.getFarPlane(), newFarPlane);
    }

    TEST_F(ARamsesCameraBinding, PropagatesItsInputsToRamsesPerspectiveCameraOnUpdate_WithLinksInsteadOfSetCall)
    {
        const std::string_view scriptSrc = R"(
            function interface(IN,OUT)
                OUT.vpProps = {
                    vpX = Type:Int32(),
                    vpY = Type:Int32(),
                    vpW = Type:Int32(),
                    vpH = Type:Int32()
                }
            end
            function run(IN,OUT)
                OUT.vpProps = {
                    vpX = 5,
                    vpY = 10,
                    vpW = 35,
                    vpH = 19
                }
            end
        )";

        LuaScript* script = m_logicEngine.createLuaScript(scriptSrc);

        auto* cameraBinding = m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");

        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("vpProps")->getChild("vpX"), *cameraBinding->getInputs()->getChild("viewport")->getChild("offsetX")));
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("vpProps")->getChild("vpY"), *cameraBinding->getInputs()->getChild("viewport")->getChild("offsetY")));
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("vpProps")->getChild("vpW"), *cameraBinding->getInputs()->getChild("viewport")->getChild("width")));
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("vpProps")->getChild("vpH"), *cameraBinding->getInputs()->getChild("viewport")->getChild("height")));

        // Links have no effect before update() explicitly called
        ExpectDefaultValues(*cameraBinding);

        m_logicEngine.update();

        // Linked values got updates, not-linked values were not modified
        EXPECT_EQ(m_perspectiveCam.getViewportX(), 5);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 10);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 35u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 19u);
        ExpectDefaultPerspectiveCameraFrustumValues(m_perspectiveCam);
    }

    TEST_F(ARamsesCameraBinding, PropagatesItsInputsToRamsesOrthoCameraOnUpdate_WithLinksInsteadOfSetCall)
    {
        const std::string_view scriptSrc = R"(
            function interface(IN,OUT)
                OUT.frustProps = {
                    lP = Type:Float(),
                    rP = Type:Float(),
                    bP = Type:Float(),
                    tP = Type:Float(),
                    nP = Type:Float(),
                    fP = Type:Float()
                }
            end
            function run(IN,OUT)
                OUT.frustProps = {
                    lP = 0.2,
                    rP = 0.3,
                    bP = 0.4,
                    tP = 0.5,
                    nP = 0.6,
                    fP = 0.7
                }
            end
        )";

        LuaScript* script = m_logicEngine.createLuaScript(scriptSrc);

        auto* cameraBinding = m_logicEngine.createRamsesCameraBinding(m_orthoCam, "");

        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("frustProps")->getChild("lP"), *cameraBinding->getInputs()->getChild("frustum")->getChild("leftPlane")));
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("frustProps")->getChild("rP"), *cameraBinding->getInputs()->getChild("frustum")->getChild("rightPlane")));
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("frustProps")->getChild("bP"), *cameraBinding->getInputs()->getChild("frustum")->getChild("bottomPlane")));
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("frustProps")->getChild("tP"), *cameraBinding->getInputs()->getChild("frustum")->getChild("topPlane")));
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("frustProps")->getChild("nP"), *cameraBinding->getInputs()->getChild("frustum")->getChild("nearPlane")));
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("frustProps")->getChild("fP"), *cameraBinding->getInputs()->getChild("frustum")->getChild("farPlane")));

        // Links have no effect before update() explicitly called
        ExpectDefaultValues(*cameraBinding);

        m_logicEngine.update();

        // Linked values got updates, not-linked values were not modified
        EXPECT_EQ(m_orthoCam.getLeftPlane(), 0.2f);
        EXPECT_EQ(m_orthoCam.getRightPlane(), 0.3f);
        EXPECT_EQ(m_orthoCam.getBottomPlane(), 0.4f);
        EXPECT_EQ(m_orthoCam.getTopPlane(), 0.5f);
        EXPECT_EQ(m_orthoCam.getNearPlane(), 0.6f);
        EXPECT_EQ(m_orthoCam.getFarPlane(), 0.7f);
        ExpectDefaultViewportValues(m_orthoCam);
    }

    TEST_F(ARamsesCameraBinding, DoesNotOverrideExistingValuesAfterRamsesCameraIsAssignedToBinding)
    {
        m_perspectiveCam.setViewport(3, 4, 10u, 11u);
        m_perspectiveCam.setFrustum(30.f, 640.f / 480.f, 2.3f, 5.6f);

        m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");

        EXPECT_EQ(m_perspectiveCam.getViewportX(), 3);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 4);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 10u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 11u);

        EXPECT_NEAR(m_perspectiveCam.getVerticalFieldOfView(), 30.f, 0.001f);
        EXPECT_EQ(m_perspectiveCam.getAspectRatio(), 640.f / 480.f);
        EXPECT_EQ(m_perspectiveCam.getNearPlane(), 2.3f);
        EXPECT_EQ(m_perspectiveCam.getFarPlane(), 5.6f);
    }

    // This fixture only contains serialization unit tests, for higher order tests see `ARamsesCameraBinding_SerializationWithFile`
    class ARamsesCameraBinding_SerializationLifecycle : public ARamsesCameraBinding
    {
    protected:
        flatbuffers::Offset<rlogic_serialization::Property> serializeRootInput(bool withFrustumPlanes, bool withError = false)
        {
            std::vector<TypeData> frustumPlanes = {
                TypeData{ "nearPlane", EPropertyType::Float },
                TypeData{ "farPlane", EPropertyType::Float },
            };

            if (withFrustumPlanes)
            {
                frustumPlanes.emplace_back("leftPlane", EPropertyType::Float);
                frustumPlanes.emplace_back("rightPlane", EPropertyType::Float);
                frustumPlanes.emplace_back("bottomPlane", EPropertyType::Float);
                if (!withError)
                    frustumPlanes.emplace_back("topPlane", EPropertyType::Float);
            }
            else
            {
                frustumPlanes.emplace_back("fieldOfView", EPropertyType::Float);
                frustumPlanes.emplace_back("aspectRatio", EPropertyType::Float);
            }

            HierarchicalTypeData cameraBindingInputs(
                TypeData{ "", EPropertyType::Struct },
            {
                MakeStruct("viewport",
                    {
                        TypeData{"offsetX", EPropertyType::Int32},
                        TypeData{"offsetY", EPropertyType::Int32},
                        TypeData{"width", EPropertyType::Int32},
                        TypeData{"height", EPropertyType::Int32}
                    }
                ),
                MakeStruct("frustum", frustumPlanes),
            }
            );

            return PropertyImpl::Serialize(PropertyImpl{ cameraBindingInputs, EPropertySemantics::BindingInput }, m_flatBufferBuilder, m_serializationMap);
        }

        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        SerializationTestUtils m_testUtils{ m_flatBufferBuilder };
        ::testing::StrictMock<RamsesObjectResolverMock> m_resolverMock;
        ErrorReporting m_errorReporting;
        SerializationMap m_serializationMap;
        DeserializationMap m_deserializationMap;
    };

    // More unit tests with inputs/outputs declared in LogicNode (base class) serialization tests
    TEST_F(ARamsesCameraBinding_SerializationLifecycle, RemembersBaseClassData)
    {
        // Serialize
        {
            RamsesCameraBindingImpl binding(*m_camera, true, "name", 1u);
            binding.createRootProperties();
            (void)RamsesCameraBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap, EFeatureLevel_01);
        }

        // Inspect flatbuffers data
        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::RamsesCameraBinding>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serializedBinding.base());
        ASSERT_TRUE(serializedBinding.base()->base());
        ASSERT_TRUE(serializedBinding.base()->base()->name());
        EXPECT_EQ(serializedBinding.base()->base()->name()->string_view(), "name");
        EXPECT_EQ(serializedBinding.base()->base()->id(), 1u);

        ASSERT_TRUE(serializedBinding.base()->rootInput());
        EXPECT_EQ(serializedBinding.base()->rootInput()->rootType(), rlogic_serialization::EPropertyRootType::Struct);
        ASSERT_TRUE(serializedBinding.base()->rootInput()->children());
        EXPECT_EQ(serializedBinding.base()->rootInput()->children()->size(), 2u);

        // Deserialize
        {
            EXPECT_CALL(m_resolverMock, findRamsesCameraInScene(::testing::Eq("name"), m_camera->getSceneObjectId())).WillOnce(::testing::Return(m_camera));
            std::unique_ptr<RamsesCameraBindingImpl> deserializedBinding = RamsesCameraBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);

            ASSERT_TRUE(deserializedBinding);
            EXPECT_EQ(deserializedBinding->getName(), "name");
            EXPECT_EQ(deserializedBinding->getId(), 1u);
            EXPECT_EQ(deserializedBinding->getInputs()->getType(), EPropertyType::Struct);
            EXPECT_EQ(deserializedBinding->getInputs()->m_impl->getPropertySemantics(), EPropertySemantics::BindingInput);
            EXPECT_EQ(deserializedBinding->getInputs()->getName(), "");
            EXPECT_EQ(deserializedBinding->getInputs()->getChildCount(), 2u);
        }
    }

    TEST_F(ARamsesCameraBinding_SerializationLifecycle, RemembersRamsesCameraId)
    {
        // Serialize
        {
            RamsesCameraBindingImpl binding(*m_camera, true, "name", 1u);
            binding.createRootProperties();
            (void)RamsesCameraBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap, EFeatureLevel_01);
        }

        // Inspect flatbuffers data
        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::RamsesCameraBinding>(m_flatBufferBuilder.GetBufferPointer());

        EXPECT_EQ(serializedBinding.base()->boundRamsesObject()->objectId(), m_camera->getSceneObjectId().getValue());
        EXPECT_EQ(serializedBinding.base()->boundRamsesObject()->objectType(), static_cast<uint32_t>(ramses::ERamsesObjectType_OrthographicCamera));

        // Deserialize
        {
            EXPECT_CALL(m_resolverMock, findRamsesCameraInScene(::testing::Eq("name"), m_camera->getSceneObjectId())).WillOnce(::testing::Return(m_camera));
            std::unique_ptr<RamsesCameraBindingImpl> deserializedBinding = RamsesCameraBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);

            ASSERT_TRUE(deserializedBinding);
            EXPECT_EQ(&deserializedBinding->getRamsesCamera(), m_camera);
        }
    }

    TEST_F(ARamsesCameraBinding_SerializationLifecycle, SerializesInputProperties_withFrustumPlanes)
    {
        // Serialize
        {
            RamsesCameraBindingImpl binding(*m_camera, true, "name", 1u);
            binding.createRootProperties();
            (void)RamsesCameraBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap, EFeatureLevel_02);
        }

        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::RamsesCameraBinding>(m_flatBufferBuilder.GetBufferPointer());

        // Deserialize
        {
            EXPECT_CALL(m_resolverMock, findRamsesCameraInScene(::testing::Eq("name"), m_camera->getSceneObjectId())).WillOnce(::testing::Return(m_camera));
            std::unique_ptr<RamsesCameraBindingImpl> deserializedBinding = RamsesCameraBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);
            ASSERT_TRUE(deserializedBinding);

            ExpectInputPropertiesWithFrustumPlanes(*deserializedBinding);
        }
    }

    TEST_F(ARamsesCameraBinding_SerializationLifecycle, SerializesInputProperties_withoutFrustumPlanes)
    {
        auto perspCamera = m_scene->createPerspectiveCamera();

        // Serialize
        {
            RamsesCameraBindingImpl binding(*perspCamera, false, "name", 1u);
            binding.createRootProperties();
            (void)RamsesCameraBindingImpl::Serialize(binding, m_flatBufferBuilder, m_serializationMap, EFeatureLevel_02);
        }

        const auto& serializedBinding = *flatbuffers::GetRoot<rlogic_serialization::RamsesCameraBinding>(m_flatBufferBuilder.GetBufferPointer());

        // Deserialize
        {
            EXPECT_CALL(m_resolverMock, findRamsesCameraInScene(::testing::Eq("name"), perspCamera->getSceneObjectId())).WillOnce(::testing::Return(perspCamera));
            std::unique_ptr<RamsesCameraBindingImpl> deserializedBinding = RamsesCameraBindingImpl::Deserialize(serializedBinding, m_resolverMock, m_errorReporting, m_deserializationMap);
            ASSERT_TRUE(deserializedBinding);

            ExpectInputPropertiesWithoutFrustumPlanes(*deserializedBinding);
        }
    }

    TEST_F(ARamsesCameraBinding_SerializationLifecycle, ProducesErrorIfInputPropertiesInvalid)
    {
        {
            auto ramsesRef = rlogic_serialization::CreateRamsesReference(
                m_flatBufferBuilder,
                12u,
                ramses::ERamsesObjectType_OrthographicCamera
            );
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                ramsesRef,
                serializeRootInput(true, true)
            );
            auto binding = rlogic_serialization::CreateRamsesCameraBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesCameraBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesCameraBindingImpl> deserialized = RamsesCameraBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);
        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesCameraBinding from serialized data: missing or invalid input properties!");
    }

    TEST_F(ARamsesCameraBinding_SerializationLifecycle, ErrorWhenNoBindingBaseData)
    {
        {
            auto binding = rlogic_serialization::CreateRamsesCameraBinding(
                m_flatBufferBuilder,
                0 // no base binding info
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesCameraBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesCameraBindingImpl> deserialized = RamsesCameraBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesCameraBinding from serialized data: missing base class info!");
    }

    TEST_F(ARamsesCameraBinding_SerializationLifecycle, ErrorWhenNoBindingName)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    0, // no name!
                    1u)
            );
            auto binding = rlogic_serialization::CreateRamsesCameraBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesCameraBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesCameraBindingImpl> deserialized = RamsesCameraBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(2u, this->m_errorReporting.getErrors().size());
        EXPECT_EQ("Fatal error during loading of LogicObject base from serialized data: missing name!", this->m_errorReporting.getErrors()[0].message);
        EXPECT_EQ("Fatal error during loading of RamsesCameraBinding from serialized data: missing name and/or ID!", this->m_errorReporting.getErrors()[1].message);
    }

    TEST_F(ARamsesCameraBinding_SerializationLifecycle, ErrorWhenNoBindingId)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    0));
            auto binding = rlogic_serialization::CreateRamsesCameraBinding(m_flatBufferBuilder, base);
            m_flatBufferBuilder.Finish(binding);
        }

        const auto&                              serialized   = *flatbuffers::GetRoot<rlogic_serialization::RamsesCameraBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesCameraBindingImpl> deserialized = RamsesCameraBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(2u, this->m_errorReporting.getErrors().size());
        EXPECT_EQ("Fatal error during loading of LogicObject base from serialized data: missing or invalid ID!", this->m_errorReporting.getErrors()[0].message);
        EXPECT_EQ("Fatal error during loading of RamsesCameraBinding from serialized data: missing name and/or ID!", this->m_errorReporting.getErrors()[1].message);
    }

    TEST_F(ARamsesCameraBinding_SerializationLifecycle, ErrorWhenNoRootInput)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                0 // no root input
            );
            auto binding = rlogic_serialization::CreateRamsesCameraBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesCameraBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesCameraBindingImpl> deserialized = RamsesCameraBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesCameraBinding from serialized data: missing root input!");
    }

    TEST_F(ARamsesCameraBinding_SerializationLifecycle, ErrorWhenRootInputHasErrors)
    {
        {
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                0,
                m_testUtils.serializeTestProperty("", rlogic_serialization::EPropertyRootType::Struct, false, true) // rootInput with errors
            );
            auto binding = rlogic_serialization::CreateRamsesCameraBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesCameraBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesCameraBindingImpl> deserialized = RamsesCameraBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of Property from serialized data: missing name!");
    }

    TEST_F(ARamsesCameraBinding_SerializationLifecycle, ErrorWhenBoundCameraCannotBeResolved)
    {
        const ramses::sceneObjectId_t mockObjectId{ 12 };
        {
            auto ramsesRef = rlogic_serialization::CreateRamsesReference(
                m_flatBufferBuilder,
                mockObjectId.getValue()
            );
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                ramsesRef,
                serializeRootInput(true)
            );
            auto binding = rlogic_serialization::CreateRamsesCameraBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        EXPECT_CALL(m_resolverMock, findRamsesCameraInScene(::testing::Eq("name"), mockObjectId)).WillOnce(::testing::Return(nullptr));

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesCameraBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesCameraBindingImpl> deserialized = RamsesCameraBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
    }

    TEST_F(ARamsesCameraBinding_SerializationLifecycle, ErrorWhenSavedCameraTypeDoesNotMatchResolvedCameraType)
    {
        RamsesTestSetup ramses;
        ramses::Scene* scene = ramses.createScene();
        auto* perspCamera = scene->createPerspectiveCamera();

        const ramses::sceneObjectId_t mockObjectId{ 12 };
        {
            auto ramsesRef = rlogic_serialization::CreateRamsesReference(
                m_flatBufferBuilder,
                mockObjectId.getValue(),
                uint32_t(ramses::ERamsesObjectType_OrthographicCamera) // save ortho camera
            );
            auto base = rlogic_serialization::CreateRamsesBinding(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                ramsesRef,
                serializeRootInput(false)
            );
            auto binding = rlogic_serialization::CreateRamsesCameraBinding(
                m_flatBufferBuilder,
                base
            );
            m_flatBufferBuilder.Finish(binding);
        }

        // resolver returns perspective camera, but orthographic camera is expected -> error
        EXPECT_CALL(m_resolverMock, findRamsesCameraInScene(::testing::Eq("name"), mockObjectId)).WillOnce(::testing::Return(perspCamera));

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::RamsesCameraBinding>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<RamsesCameraBindingImpl> deserialized = RamsesCameraBindingImpl::Deserialize(serialized, m_resolverMock, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesCameraBinding from serialized data: loaded type does not match referenced camera type!");
    }

    class ARamsesCameraBinding_SerializationWithFile : public ARamsesCameraBinding
    {
    protected:
        WithTempDirectory tempFolder;
    };

    TEST_F(ARamsesCameraBinding_SerializationWithFile, ContainsItsDataAfterLoading)
    {
        const int32_t newVpOffsetX = 10;
        const int32_t newVpOffsetY = 13;
        const int32_t newVpWidth = 56;
        const int32_t newVpHeight = 45;

        const float newFov = 30.f;
        const float newAR = 640.f / 480.f;
        const float newNearPlane = 4.4f;
        const float newFarPlane = 5.1f;
        {
            auto& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "CameraBinding");

            auto inputs = cameraBinding.getInputs();
            auto vpProperties = inputs->getChild("viewport");
            auto frustum = inputs->getChild("frustum");

            vpProperties->getChild("offsetX")->set<int32_t>(newVpOffsetX);
            vpProperties->getChild("offsetY")->set<int32_t>(newVpOffsetY);
            vpProperties->getChild("width")->set<int32_t>(newVpWidth);
            vpProperties->getChild("height")->set<int32_t>(newVpHeight);

            frustum->getChild("fieldOfView")->set<float>(newFov);
            frustum->getChild("aspectRatio")->set<float>(newAR);
            frustum->getChild("nearPlane")->set<float>(newNearPlane);
            frustum->getChild("farPlane")->set<float>(newFarPlane);
            m_logicEngine.update();
            EXPECT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "camerabinding.bin"));
        }
        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("camerabinding.bin", &m_testScene));
            const auto& loadedCameraBinding = *m_logicEngine.findByName<RamsesCameraBinding>("CameraBinding");
            EXPECT_EQ("CameraBinding", loadedCameraBinding.getName());
            EXPECT_EQ(loadedCameraBinding.getId(), 1u);
            EXPECT_EQ(loadedCameraBinding.getRamsesCamera().getSceneObjectId(), m_perspectiveCam.getSceneObjectId());

            const auto& inputs = loadedCameraBinding.getInputs();
            ASSERT_EQ(inputs->getChildCount(), 2u);
            auto vpProperties = inputs->getChild("viewport");
            auto frustum = inputs->getChild("frustum");
            ASSERT_EQ(vpProperties->getChildCount(), 4u);
            ASSERT_EQ(vpProperties->m_impl->getPropertySemantics(), EPropertySemantics::BindingInput);
            ASSERT_EQ(frustum->getChildCount(), 4u);
            ASSERT_EQ(frustum->m_impl->getPropertySemantics(), EPropertySemantics::BindingInput);

            EXPECT_EQ(*vpProperties->getChild("offsetX")->get<int32_t>(), newVpOffsetX);
            EXPECT_EQ(vpProperties->getChild("offsetX")->m_impl->getPropertySemantics(), EPropertySemantics::BindingInput);
            EXPECT_EQ(*vpProperties->getChild("offsetY")->get<int32_t>(), newVpOffsetY);
            EXPECT_EQ(vpProperties->getChild("offsetY")->m_impl->getPropertySemantics(), EPropertySemantics::BindingInput);
            EXPECT_EQ(*vpProperties->getChild("width")->get<int32_t>(), newVpWidth);
            EXPECT_EQ(vpProperties->getChild("width")->m_impl->getPropertySemantics(), EPropertySemantics::BindingInput);
            EXPECT_EQ(*vpProperties->getChild("height")->get<int32_t>(), newVpHeight);
            EXPECT_EQ(vpProperties->getChild("height")->m_impl->getPropertySemantics(), EPropertySemantics::BindingInput);

            EXPECT_EQ(*frustum->getChild("nearPlane")->get<float>(), newNearPlane);
            EXPECT_EQ(frustum->getChild("nearPlane")->m_impl->getPropertySemantics(), EPropertySemantics::BindingInput);
            EXPECT_EQ(*frustum->getChild("farPlane")->get<float>(), newFarPlane);
            EXPECT_EQ(frustum->getChild("farPlane")->m_impl->getPropertySemantics(), EPropertySemantics::BindingInput);
            EXPECT_NEAR(*frustum->getChild("fieldOfView")->get<float>(), newFov, 0.001f);
            EXPECT_EQ(frustum->getChild("fieldOfView")->m_impl->getPropertySemantics(), EPropertySemantics::BindingInput);
            EXPECT_EQ(*frustum->getChild("aspectRatio")->get<float>(), newAR);
            EXPECT_EQ(frustum->getChild("aspectRatio")->m_impl->getPropertySemantics(), EPropertySemantics::BindingInput);

            // Test that internal indices match properties resolved by name
            EXPECT_EQ(vpProperties->getChild("offsetX"), vpProperties->m_impl->getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetX)));
            EXPECT_EQ(vpProperties->getChild("offsetY"), vpProperties->m_impl->getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetY)));
            EXPECT_EQ(vpProperties->getChild("width"), vpProperties->m_impl->getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortWidth)));
            EXPECT_EQ(vpProperties->getChild("height"), vpProperties->m_impl->getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortHeight)));

            EXPECT_EQ(frustum->getChild("nearPlane"), frustum->m_impl->getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::NearPlane)));
            EXPECT_EQ(frustum->getChild("farPlane"), frustum->m_impl->getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::FarPlane)));
            EXPECT_EQ(frustum->getChild("fieldOfView"), frustum->m_impl->getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::FieldOfView)));
            EXPECT_EQ(frustum->getChild("aspectRatio"), frustum->m_impl->getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::AspectRatio)));
        }
    }

    TEST_F(ARamsesCameraBinding_SerializationWithFile, KeepsItsProperties_WhenNoRamsesLinksAndSceneProvided)
    {
        {
            m_logicEngine.createRamsesCameraBinding(*m_camera, "CameraBinding");
            ASSERT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "camerabinding.bin"));
        }

        {
            ASSERT_TRUE(m_logicEngine.loadFromFile("camerabinding.bin", m_scene));
            auto loadedCameraBinding = m_logicEngine.findByName<RamsesCameraBinding>("CameraBinding");
            EXPECT_EQ(&loadedCameraBinding->getRamsesCamera(), m_camera);
            EXPECT_EQ(loadedCameraBinding->getInputs()->getChildCount(), 2u);
            EXPECT_EQ(loadedCameraBinding->getOutputs(), nullptr);
            EXPECT_EQ(loadedCameraBinding->getName(), "CameraBinding");
        }
    }

    TEST_F(ARamsesCameraBinding_SerializationWithFile, RestoresLinkToRamsesCamera)
    {
        {
            m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "CameraBinding");
            EXPECT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "camerabinding.bin"));
        }
        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("camerabinding.bin", &m_testScene));
            const auto& cameraBinding = *m_logicEngine.findByName<RamsesCameraBinding>("CameraBinding");
            EXPECT_EQ(&cameraBinding.getRamsesCamera(), &m_perspectiveCam);
        }
    }

    TEST_F(ARamsesCameraBinding_SerializationWithFile, ProducesError_WhenHavingLinkToRamsesCamera_ButNoSceneWasProvided)
    {
        {
            m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "CameraBinding");
            EXPECT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "camerabinding.bin"));
        }
        {
            EXPECT_FALSE(m_logicEngine.loadFromFile("camerabinding.bin"));
            auto errors = m_logicEngine.getErrors();
            ASSERT_EQ(errors.size(), 1u);
            EXPECT_EQ(errors[0].message, "Fatal error during loading from file! File contains references to Ramses objects but no Ramses scene was provided!");
        }
    }

    TEST_F(ARamsesCameraBinding_SerializationWithFile, HandlesErrorWhenRamsesSceneWasSerializedWithOneTypeOfCamera_ButLoadedWithADifferentOne)
    {
        {
            m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");
            EXPECT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "camerabinding.bin"));
        }

        // Fake that the ramses scene is exactly the same, just camera type changed (perspective -> ortho)
        // This is a bit evil, but quite possible e.g. if camera was switched from perspective -> ortho and the ramses id didn't change because no other change in the scene and
        // the camera is exported at the exact same time -> receives the same ID
        ramses::Scene& slightlyModifiedScene = *m_ramsesTestSetup.createScene(ramses::sceneId_t(2));
        slightlyModifiedScene.createOrthographicCamera("");
        {
            // loadFromFile catches the error -> content is not modified
            EXPECT_FALSE(m_logicEngine.loadFromFile("camerabinding.bin", &slightlyModifiedScene));
            // Update still works with the old state of the logic engine
            EXPECT_TRUE(m_logicEngine.update());
        }
    }

    TEST_F(ARamsesCameraBinding_SerializationWithFile, ProducesError_WhenHavingLinkToRamsesCamera_WhichWasDeleted)
    {
        {
            m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "CameraBinding");
            EXPECT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "camerabinding.bin"));
        }

        m_testScene.destroy(m_perspectiveCam);

        {
            EXPECT_FALSE(m_logicEngine.loadFromFile("camerabinding.bin", &m_testScene));
            auto errors = m_logicEngine.getErrors();
            ASSERT_EQ(errors.size(), 1u);
            EXPECT_EQ(errors[0].message, "Fatal error during loading from file! Serialized Ramses Logic object 'CameraBinding' points to a Ramses object (id: 2) which couldn't be found in the provided scene!");
        }
    }

    TEST_F(ARamsesCameraBinding_SerializationWithFile, DoesNotModifyRamsesCameraProperties_WhenNoValuesWereExplicitlySetBeforeSaving)
    {
        {
            m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "CameraBinding");
            EXPECT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "camerabinding.bin"));
        }
        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("camerabinding.bin", &m_testScene));
            EXPECT_TRUE(m_logicEngine.update());

            ExpectDefaultValues(*m_logicEngine.findByName<RamsesCameraBinding>("CameraBinding"));
        }
    }

    // Tests that the camera properties don't overwrite ramses' values after loading from file, until
    // set() is called again explicitly after loadFromFile()
    TEST_F(ARamsesCameraBinding_SerializationWithFile, ReappliesViewportPropertiesToRamsesCamera_OnlyAfterExplicitlySetAgain)
    {
        {
            auto& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "CameraBinding");
            // Set some values to the binding's inputs. Those should be lost/discarded on save() because we dont' call update() below
            auto vpProperties = cameraBinding.getInputs()->getChild("viewport");
            vpProperties->getChild("offsetX")->set<int32_t>(4);
            vpProperties->getChild("offsetY")->set<int32_t>(5);
            vpProperties->getChild("width")->set<int32_t>(6);
            vpProperties->getChild("height")->set<int32_t>(7);
            m_logicEngine.update();
            EXPECT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "camerabinding.bin"));
        }

        // These values will be used to fill the cache of camera bindings on load()
        m_perspectiveCam.setViewport(11, 12, 13u, 14u);

        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("camerabinding.bin", &m_testScene));

            // Artificially set to other values so that we can verify update() didn't change them
            m_perspectiveCam.setViewport(9, 8, 1u, 2u);

            EXPECT_TRUE(m_logicEngine.update());

            // Camera binding does not re-apply its cached values to ramses camera viewport
            EXPECT_EQ(m_perspectiveCam.getViewportX(), 9);
            EXPECT_EQ(m_perspectiveCam.getViewportY(), 8);
            EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 1u);
            EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 2u);

            // Set only one value of viewport struct. Use the same value as the one in cache on purpose!
            // Calling set forces set on ramses regardless of the value used
            m_logicEngine.findByName<RamsesCameraBinding>("CameraBinding")->getInputs()->getChild("viewport")->getChild("offsetX")->set<int32_t>(11);
            m_logicEngine.update();
            EXPECT_TRUE(m_logicEngine.update());

            // vpOffsetX changed, the rest is taken from the initially saved inputs, not what was set on the camera!
            EXPECT_EQ(m_perspectiveCam.getViewportX(), 11);
            EXPECT_EQ(m_perspectiveCam.getViewportY(), 12);
            EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 13u);
            EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 14u);
        }
    }

    // This is sort of a confidence test, testing a combination of:
    // - bindings only propagating their values to ramses camera if the value was set by an incoming link
    // - saving and loading files
    // The general expectation is that after loading + update(), the logic scene would overwrite only ramses
    // properties wrapped by a LogicBinding which is linked to a script
    TEST_F(ARamsesCameraBinding_SerializationWithFile, SetsOnlyRamsesCameraPropertiesForWhichTheBindingInputIsLinked_WhenCallingUpdateAfterLoading)
    {
        // These values should not be overwritten by logic on update()
        m_perspectiveCam.setViewport(9, 8, 1u, 2u);

        {
            const std::string_view scriptSrc = R"(
            function interface(IN,OUT)
                OUT.frustProps = {
                    fov = Type:Float(),
                    aR = Type:Float(),
                    nP = Type:Float(),
                    fP = Type:Float()
                }
            end
            function run(IN,OUT)
                OUT.frustProps = {
                    fov = 30.0,
                    aR = 640.0 / 480.0,
                    nP = 2.3,
                    fP = 5.6
                }
            end
            )";

            LuaScript* script = m_logicEngine.createLuaScript(scriptSrc);

            RamsesCameraBinding& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "CameraBinding");

            ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("frustProps")->getChild("fov"), *cameraBinding.getInputs()->getChild("frustum")->getChild("fieldOfView")));
            ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("frustProps")->getChild("aR"), *cameraBinding.getInputs()->getChild("frustum")->getChild("aspectRatio")));
            ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("frustProps")->getChild("nP"), *cameraBinding.getInputs()->getChild("frustum")->getChild("nearPlane")));
            ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("frustProps")->getChild("fP"), *cameraBinding.getInputs()->getChild("frustum")->getChild("farPlane")));

            m_logicEngine.update();
            EXPECT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "camerabinding.bin"));
        }

        // Modify 'linked' properties before loading to check if logic will overwrite them after load + update
        m_perspectiveCam.setFrustum(15.f, 320.f / 240.f, 4.1f, 7.9f);

        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("camerabinding.bin", &m_testScene));

            EXPECT_TRUE(m_logicEngine.update());

            // Viewport properties were not linked -> their values are not modified
            EXPECT_EQ(m_perspectiveCam.getViewportX(), 9);
            EXPECT_EQ(m_perspectiveCam.getViewportY(), 8);
            EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 1u);
            EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 2u);
            // Frustum properties are linked -> values were updated
            EXPECT_NEAR(m_perspectiveCam.getVerticalFieldOfView(), 30.f, 0.001f);
            EXPECT_EQ(m_perspectiveCam.getAspectRatio(), 640.f / 480.f);
            EXPECT_EQ(m_perspectiveCam.getNearPlane(), 2.3f);
            EXPECT_EQ(m_perspectiveCam.getFarPlane(), 5.6f);

            // Manually setting values on ramses followed by a logic update has no effect
            // Logic is not "dirty" and it doesn't know it needs to update ramses
            m_perspectiveCam.setViewport(43, 34, 84u, 62u);
            EXPECT_TRUE(m_logicEngine.update());
            EXPECT_EQ(m_perspectiveCam.getViewportX(), 43);
            EXPECT_EQ(m_perspectiveCam.getViewportY(), 34);
            EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 84u);
            EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 62u);
        }
    }

    // Larger confidence tests which verify and document the entire data flow cycle of bindings
    // There are smaller tests which test only properties and their data propagation rules (see property unit tests)
    // There are also "dirtiness" tests which test when a camera is being re-updated (see logic engine dirtiness tests)
    // These tests test everything in combination

    class ARamsesCameraBinding_DataFlow : public ARamsesCameraBinding
    {
    };

    TEST_F(ARamsesCameraBinding_DataFlow, WithExplicitSet)
    {
        // Create camera and preset values
        m_perspectiveCam.setViewport(11, 12, 13u, 14u);
        RamsesCameraBinding& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "");
        // set other values to artificially check that the binding won't override them
        m_perspectiveCam.setViewport(9, 8, 1u, 2u);

        // Nothing happens here - binding does not overwrite ramses values because no user value set() was called and no link exists
        m_logicEngine.update();
        EXPECT_EQ(m_perspectiveCam.getViewportX(), 9);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 8);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 1u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 2u);

        // Set only two view port properties
        auto vpProperties = cameraBinding.getInputs()->getChild("viewport");
        vpProperties->getChild("offsetX")->set<int32_t>(4);
        vpProperties->getChild("width")->set<int32_t>(21);

        // Update not called yet -> still has preset values for vpOffsetX and vpWidth in ramses camera, and default frustum values
        EXPECT_EQ(m_perspectiveCam.getViewportX(), 9);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 8);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 1u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 2u);
        ExpectDefaultPerspectiveCameraFrustumValues(m_perspectiveCam);

        // Update() triggers all viewport to be set on ramses to the two values that were explicitly set
        // and the other two previous values of the binding input
        m_logicEngine.update();
        EXPECT_EQ(m_perspectiveCam.getViewportX(), 4);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 12);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 21u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 14u);
        // Frustum is not modified - only viewport was explicitly set
        ExpectDefaultPerspectiveCameraFrustumValues(m_perspectiveCam);

        // Set two properties of each viewPort and frustum property struct
        vpProperties->getChild("offsetY")->set<int32_t>(13);
        vpProperties->getChild("height")->set<int32_t>(63);
        auto frustum = cameraBinding.getInputs()->getChild("frustum");
        frustum->getChild("nearPlane")->set<float>(2.3f);
        frustum->getChild("farPlane")->set<float>(5.6f);

        // On update all values of both structs are set
        m_logicEngine.update();
        EXPECT_EQ(m_perspectiveCam.getViewportX(), 4);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 13);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 21u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 63u);

        EXPECT_NEAR(m_perspectiveCam.getVerticalFieldOfView(), PerspectiveFrustumFOVdefault, 0.001f);
        EXPECT_EQ(m_perspectiveCam.getAspectRatio(), PerspectiveFrustumARdefault);
        EXPECT_EQ(m_perspectiveCam.getNearPlane(), 2.3f);
        EXPECT_EQ(m_perspectiveCam.getFarPlane(), 5.6f);

        // Calling update again does not "rewrite" the data to ramses. Check this by setting a value manually and call update() again
        m_perspectiveCam.setViewport(9, 8, 1u, 2u);
        m_logicEngine.update();
        EXPECT_EQ(m_perspectiveCam.getViewportX(), 9);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 8);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 1u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 2u);

        // Set all properties manually this time
        vpProperties->getChild("offsetX")->set<int32_t>(4);
        vpProperties->getChild("offsetY")->set<int32_t>(5);
        vpProperties->getChild("width")->set<int32_t>(6);
        vpProperties->getChild("height")->set<int32_t>(7);

        frustum->getChild("fieldOfView")->set<float>(30.f);
        frustum->getChild("aspectRatio")->set<float>(640.f / 480.f);
        frustum->getChild("nearPlane")->set<float>(1.3f);
        frustum->getChild("farPlane")->set<float>(7.6f);
        m_logicEngine.update();

        // All of the property values were passed to ramses
        EXPECT_EQ(m_perspectiveCam.getViewportX(), 4);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 5);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 6u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 7u);

        EXPECT_NEAR(m_perspectiveCam.getVerticalFieldOfView(), 30.f, 0.001f);
        EXPECT_EQ(m_perspectiveCam.getAspectRatio(), 640.f / 480.f);
        EXPECT_EQ(m_perspectiveCam.getNearPlane(), 1.3f);
        EXPECT_EQ(m_perspectiveCam.getFarPlane(), 7.6f);
    }

    TEST_F(ARamsesCameraBinding_DataFlow, WithLinks)
    {
        const std::string_view scriptSrc = R"(
            function interface(IN,OUT)
                OUT.vpOffsetX = Type:Int32()
            end
            function run(IN,OUT)
                OUT.vpOffsetX = 15
            end
        )";

        // Create camera and preset values
        m_perspectiveCam.setViewport(11, 12, 13u, 14u);

        LuaScript* script = m_logicEngine.createLuaScript(scriptSrc);
        RamsesCameraBinding& cameraBinding = *m_logicEngine.createRamsesCameraBinding(m_perspectiveCam, "CameraBinding");
        // set other values to artificially check that the binding won't override them
        m_perspectiveCam.setViewport(9, 8, 1u, 2u);

        // Adding and removing link does not set anything in ramses
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("vpOffsetX"), *cameraBinding.getInputs()->getChild("viewport")->getChild("offsetX")));
        ASSERT_TRUE(m_logicEngine.unlink(*script->getOutputs()->getChild("vpOffsetX"), *cameraBinding.getInputs()->getChild("viewport")->getChild("offsetX")));
        m_logicEngine.update();
        EXPECT_EQ(m_perspectiveCam.getViewportX(), 9);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 8);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 1u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 2u);
        ExpectDefaultPerspectiveCameraFrustumValues(m_perspectiveCam);

        // Create link and calling update -> sets values to ramses set by the link (vpOffsetX)
        // and uses cached values in the binding for the other vp properties
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("vpOffsetX"), *cameraBinding.getInputs()->getChild("viewport")->getChild("offsetX")));
        m_logicEngine.update();
        EXPECT_EQ(m_perspectiveCam.getViewportX(), 15);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 12);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 13u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 14u);
        // Does not touch the frustum because not linked or set at all
        ExpectDefaultPerspectiveCameraFrustumValues(m_perspectiveCam);

        // Link does not overwrite manually set values as long as the actual value didnt change to avoid causing unnecessary sets on ramses
        m_perspectiveCam.setViewport(9, 8, 1u, 2u);
        m_logicEngine.update();
        EXPECT_EQ(m_perspectiveCam.getViewportX(), 9);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 8);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 1u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 2u);
        ExpectDefaultPerspectiveCameraFrustumValues(m_perspectiveCam);

        // Remove link -> value is not overwritten any more
        ASSERT_TRUE(m_logicEngine.unlink(*script->getOutputs()->getChild("vpOffsetX"), *cameraBinding.getInputs()->getChild("viewport")->getChild("offsetX")));
        m_perspectiveCam.setViewport(9, 8, 1u, 2u);
        m_logicEngine.update();
        EXPECT_EQ(m_perspectiveCam.getViewportX(), 9);
        EXPECT_EQ(m_perspectiveCam.getViewportY(), 8);
        EXPECT_EQ(m_perspectiveCam.getViewportWidth(), 1u);
        EXPECT_EQ(m_perspectiveCam.getViewportHeight(), 2u);
        ExpectDefaultPerspectiveCameraFrustumValues(m_perspectiveCam);
    }
}
