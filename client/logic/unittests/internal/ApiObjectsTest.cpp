//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"

#include "generated/LogicEngineGen.h"

#include "internals/ApiObjects.h"
#include "internals/SolState.h"
#include "internals/ErrorReporting.h"

#include "impl/LogicEngineImpl.h"
#include "impl/PropertyImpl.h"
#include "impl/LuaModuleImpl.h"
#include "impl/LuaScriptImpl.h"
#include "impl/LuaInterfaceImpl.h"
#include "impl/RamsesNodeBindingImpl.h"
#include "impl/RamsesAppearanceBindingImpl.h"
#include "impl/RamsesCameraBindingImpl.h"
#include "impl/RamsesRenderPassBindingImpl.h"
#include "impl/RamsesRenderGroupBindingImpl.h"
#include "impl/RamsesMeshNodeBindingImpl.h"
#include "impl/DataArrayImpl.h"
#include "impl/AnchorPointImpl.h"
#include "impl/SkinBindingImpl.h"

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/LuaInterface.h"
#include "ramses-logic/LuaModule.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"
#include "ramses-logic/RamsesRenderPassBinding.h"
#include "ramses-logic/RamsesRenderGroupBinding.h"
#include "ramses-logic/RamsesRenderGroupBindingElements.h"
#include "ramses-logic/RamsesMeshNodeBinding.h"
#include "ramses-logic/DataArray.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/AnimationNodeConfig.h"
#include "ramses-logic/TimerNode.h"
#include "ramses-logic/AnchorPoint.h"
#include "ramses-logic/SkinBinding.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/RenderPass.h"
#include "RamsesTestUtils.h"
#include "LogTestUtils.h"
#include "SerializationTestUtils.h"
#include "RamsesObjectResolverMock.h"
#include "FeatureLevelTestValues.h"

namespace rlogic::internal
{
    class AnApiObjects : public ::testing::TestWithParam<EFeatureLevel>
    {
    protected:
        AnApiObjects()
        {
            m_renderGroup->addMeshNode(*m_meshNode);
        }

        ErrorReporting  m_errorReporting;
        ApiObjects      m_apiObjects{ GetParam() };
        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        SerializationTestUtils m_testUtils{ m_flatBufferBuilder };
        ::testing::StrictMock<RamsesObjectResolverMock> m_resolverMock;

        RamsesTestSetup m_ramses;
        ramses::Scene* m_scene = { m_ramses.createScene() };
        ramses::Node* m_node = { m_scene->createNode() };
        ramses::PerspectiveCamera* m_camera = { m_scene->createPerspectiveCamera() };
        ramses::Appearance* m_appearance = { &RamsesTestSetup::CreateTrivialTestAppearance(*m_scene) };
        ramses::RenderPass* m_renderPass = { m_scene->createRenderPass() };
        ramses::RenderGroup* m_renderGroup = { m_scene->createRenderGroup() };
        ramses::MeshNode* m_meshNode = { m_scene->createMeshNode("meshNode") };

        const std::string_view m_moduleSrc = R"(
            local mymath = {}
            return mymath
        )";

        const std::string_view m_valid_empty_script = R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
            end
        )";

        const std::string_view m_valid_empty_interface = R"(
            function interface(IN,OUT)
            end
        )";

        LuaScript* createScript()
        {
            return createScript(m_apiObjects, m_valid_empty_script);
        }

        LuaScript* createScript(ApiObjects& apiObjects, std::string_view source)
        {
            auto script = apiObjects.createLuaScript(source, {}, "script", m_errorReporting);
            EXPECT_NE(nullptr, script);
            return script;
        }

        LuaInterface* createInterface()
        {
            return createInterface(m_apiObjects);
        }

        LuaInterface* createInterface(ApiObjects& apiObjects)
        {
            auto intf = apiObjects.createLuaInterface(m_valid_empty_interface, {}, "intf", m_errorReporting, true);
            EXPECT_NE(nullptr, intf);
            return intf;
        }

        AnchorPoint* createAnchorPoint()
        {
            const auto node = m_apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
            const auto camera = m_apiObjects.createRamsesCameraBinding(*m_camera, true, "camera");
            EXPECT_TRUE(node && camera);
            return m_apiObjects.createAnchorPoint(node->m_nodeBinding, camera->m_cameraBinding, "anchor");
        }

        RamsesRenderGroupBinding* createRenderGroupBinding()
        {
            RamsesRenderGroupBindingElements elements;
            EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
            return m_apiObjects.createRamsesRenderGroupBinding(*m_renderGroup, elements, "renderGroupBinding");
        }

        static SkinBinding* createSkinBinding(const RamsesNodeBinding& joint, const RamsesAppearanceBinding& appearance, ApiObjects& apiObjects)
        {
            ramses::UniformInput uniform;
            appearance.getRamsesAppearance().getEffect().findUniformInput("jointMat", uniform);
            EXPECT_TRUE(uniform.isValid());
            return apiObjects.createSkinBinding({ &joint.m_nodeBinding }, { matrix44f{ 0.f } }, appearance.m_appearanceBinding, uniform, "skin");
        }

        SkinBinding* createSkinBinding(ApiObjects& apiObjects)
        {
            const auto node = apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodeForSkin");
            const auto appearance = apiObjects.createRamsesAppearanceBinding(*m_appearance, "appearanceForSkin");
            return createSkinBinding(*node, *appearance, apiObjects);
        }

        // Silence logs, unless explicitly enabled, to reduce spam and speed up tests
        ScopedLogContextLevel m_silenceLogs{ ELogMessageType::Off };

        size_t m_emptySerializedSizeTotal{164u};
    };


    INSTANTIATE_TEST_SUITE_P(
        AnApiObjectsTests,
        AnApiObjects,
        rlogic::internal::GetFeatureLevelTestValues());

    TEST_P(AnApiObjects, CreatesScriptFromValidLuaWithoutErrors)
    {
        const LuaScript* script = createScript();
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_EQ(script, m_apiObjects.getApiObject(script->m_impl));
        EXPECT_EQ(script, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(script, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysScriptWithoutErrors)
    {
        LuaScript* script = createScript();
        ASSERT_TRUE(m_apiObjects.destroy(*script, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingScriptFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam() };
        LuaScript* script = createScript(otherInstance, m_valid_empty_script);
        EXPECT_EQ(script, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(script, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*script, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Can't find script in logic engine!");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, script);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(script, otherInstance.getApiObject(script->m_impl));
        EXPECT_EQ(script, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(script, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesInterfaceFromValidLuaWithoutErrors)
    {
        const LuaInterface* intf = createInterface();
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_EQ(intf, m_apiObjects.getApiObject(intf->m_impl));
        EXPECT_EQ(intf, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(intf, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysInterfaceWithoutErrors)
    {
        LuaInterface* intf = createInterface();
        ASSERT_TRUE(m_apiObjects.destroy(*intf, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingInterfaceFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam() };
        LuaInterface* intf = createInterface(otherInstance);
        EXPECT_EQ(intf, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(intf, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*intf, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Can't find interface in logic engine!");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, intf);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(intf, otherInstance.getApiObject(intf->m_impl));
        EXPECT_EQ(intf, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(intf, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesLuaModule)
    {
        auto module = m_apiObjects.createLuaModule(m_moduleSrc, {}, "module", m_errorReporting);
        EXPECT_NE(nullptr, module);

        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        ASSERT_EQ(1u, m_apiObjects.getApiObjectContainer<LuaModule>().size());
        EXPECT_EQ(1u, m_apiObjects.getApiObjectContainer<LogicObject>().size());
        EXPECT_EQ(1u, m_apiObjects.getApiObjectOwningContainer().size());
        EXPECT_EQ(module, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(module, m_apiObjects.getApiObjectContainer<LogicObject>().back());
        EXPECT_EQ(module, m_apiObjects.getApiObjectContainer<LuaModule>().front());
    }

    TEST_P(AnApiObjects, CreatesRamsesNodeBindingWithoutErrors)
    {
        RamsesNodeBinding* ramsesNodeBinding = m_apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        EXPECT_NE(nullptr, ramsesNodeBinding);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_EQ(ramsesNodeBinding, m_apiObjects.getApiObject(ramsesNodeBinding->m_impl));
        EXPECT_EQ(ramsesNodeBinding, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(ramsesNodeBinding, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysRamsesNodeBindingWithoutErrors)
    {
        RamsesNodeBinding* ramsesNodeBinding = m_apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        ASSERT_NE(nullptr, ramsesNodeBinding);
        m_apiObjects.destroy(*ramsesNodeBinding, m_errorReporting);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingRamsesNodeBindingFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam() };
        RamsesNodeBinding* ramsesNodeBinding = otherInstance.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        ASSERT_TRUE(ramsesNodeBinding);
        EXPECT_EQ(ramsesNodeBinding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(ramsesNodeBinding, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*ramsesNodeBinding, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Can't find RamsesNodeBinding in logic engine!");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, ramsesNodeBinding);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(ramsesNodeBinding, otherInstance.getApiObject(ramsesNodeBinding->m_impl));
        EXPECT_EQ(ramsesNodeBinding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(ramsesNodeBinding, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesRamsesCameraBindingWithoutErrors)
    {
        RamsesCameraBinding* ramsesCameraBinding = m_apiObjects.createRamsesCameraBinding(*m_camera, false, "CameraBinding");
        EXPECT_NE(nullptr, ramsesCameraBinding);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_EQ(ramsesCameraBinding, m_apiObjects.getApiObject(ramsesCameraBinding->m_impl));
        EXPECT_EQ(ramsesCameraBinding, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(ramsesCameraBinding, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysRamsesCameraBindingWithoutErrors)
    {
        RamsesCameraBinding* ramsesCameraBinding = m_apiObjects.createRamsesCameraBinding(*m_camera, true, "CameraBinding");
        ASSERT_NE(nullptr, ramsesCameraBinding);
        m_apiObjects.destroy(*ramsesCameraBinding, m_errorReporting);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingRamsesCameraBindingFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam() };
        RamsesCameraBinding* ramsesCameraBinding = otherInstance.createRamsesCameraBinding(*m_camera, true, "CameraBinding");
        ASSERT_TRUE(ramsesCameraBinding);
        EXPECT_EQ(ramsesCameraBinding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(ramsesCameraBinding, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*ramsesCameraBinding, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Can't find RamsesCameraBinding in logic engine!");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, ramsesCameraBinding);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(ramsesCameraBinding, otherInstance.getApiObject(ramsesCameraBinding->m_impl));
        EXPECT_EQ(ramsesCameraBinding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(ramsesCameraBinding, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesRamsesRenderPassBindingWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        RamsesRenderPassBinding* binding = m_apiObjects.createRamsesRenderPassBinding(*m_renderPass, "RenderPassBinding");
        EXPECT_NE(nullptr, binding);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_EQ(binding, m_apiObjects.getApiObject(binding->m_impl));
        EXPECT_EQ(binding, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysRamsesRenderPassBindingWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        RamsesRenderPassBinding* binding = m_apiObjects.createRamsesRenderPassBinding(*m_renderPass, "RenderPassBinding");
        ASSERT_NE(nullptr, binding);
        m_apiObjects.destroy(*binding, m_errorReporting);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingRamsesRenderPassBindingFromAnotherClassInstance)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        ApiObjects otherInstance{ GetParam() };
        RamsesRenderPassBinding* binding = otherInstance.createRamsesRenderPassBinding(*m_renderPass, "RenderPassBinding");
        ASSERT_TRUE(binding);
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*binding, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Can't find RamsesRenderPassBinding in logic engine!");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, binding);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(binding, otherInstance.getApiObject(binding->m_impl));
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesRamsesRenderGroupBindingWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_03)
            GTEST_SKIP();

        const auto binding = createRenderGroupBinding();
        EXPECT_NE(nullptr, binding);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_EQ(binding, m_apiObjects.getApiObject(binding->m_impl));
        EXPECT_EQ(binding, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysRamsesRenderGroupBindingWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_03)
            GTEST_SKIP();

        auto binding = createRenderGroupBinding();
        ASSERT_NE(nullptr, binding);
        m_apiObjects.destroy(*binding, m_errorReporting);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingRamsesRenderGroupBindingFromAnotherClassInstance)
    {
        if (GetParam() < EFeatureLevel_03)
            GTEST_SKIP();

        ApiObjects otherInstance{ GetParam() };
        RamsesRenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(*m_meshNode));
        auto binding = otherInstance.createRamsesRenderGroupBinding(*m_renderGroup, elements, "RenderGroupBinding");
        ASSERT_TRUE(binding);
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*binding, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Can't find RamsesRenderGroupBinding in logic engine!");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, binding);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(binding, otherInstance.getApiObject(binding->m_impl));
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesRamsesMeshNodeBindingWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_05)
            GTEST_SKIP();

        const auto binding = m_apiObjects.createRamsesMeshNodeBinding(*m_meshNode, "mb");
        EXPECT_NE(nullptr, binding);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_EQ(binding, m_apiObjects.getApiObject(binding->m_impl));
        EXPECT_EQ(binding, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysRamsesMeshNodeBindingWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_05)
            GTEST_SKIP();

        auto binding = m_apiObjects.createRamsesMeshNodeBinding(*m_meshNode, "mb");
        ASSERT_NE(nullptr, binding);
        m_apiObjects.destroy(*binding, m_errorReporting);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingRamsesMeshNodeBindingFromAnotherClassInstance)
    {
        if (GetParam() < EFeatureLevel_05)
            GTEST_SKIP();

        ApiObjects otherInstance{ GetParam() };
        auto binding = otherInstance.createRamsesMeshNodeBinding(*m_meshNode, "mb");
        ASSERT_TRUE(binding);
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
        EXPECT_FALSE(m_apiObjects.destroy(*binding, m_errorReporting));
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Can't find RamsesMeshNodeBinding in logic engine!");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, binding);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(binding, otherInstance.getApiObject(binding->m_impl));
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesRamsesAppearanceBindingWithoutErrors)
    {
        RamsesAppearanceBinding* binding = m_apiObjects.createRamsesAppearanceBinding(*m_appearance, "AppearanceBinding");
        EXPECT_NE(nullptr, binding);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_EQ(binding, m_apiObjects.getApiObject(binding->m_impl));
        EXPECT_EQ(binding, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysRamsesAppearanceBindingWithoutErrors)
    {
        RamsesAppearanceBinding* binding = m_apiObjects.createRamsesAppearanceBinding(*m_appearance, "AppearanceBinding");
        ASSERT_TRUE(binding);
        ASSERT_TRUE(m_apiObjects.destroy(*binding, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingRamsesAppearanceBindingFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam() };
        RamsesAppearanceBinding* binding = otherInstance.createRamsesAppearanceBinding(*m_appearance, "AppearanceBinding");
        ASSERT_TRUE(binding);
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*binding, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Can't find RamsesAppearanceBinding in logic engine!");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, binding);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(binding, otherInstance.getApiObject(binding->m_impl));
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesDataArray)
    {
        const std::vector<float> data{ 1.f, 2.f, 3.f };
        auto dataArray = m_apiObjects.createDataArray(data, "data");
        EXPECT_NE(nullptr, dataArray);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        ASSERT_EQ(1u, m_apiObjects.getApiObjectContainer<DataArray>().size());
        EXPECT_EQ(dataArray, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(dataArray, m_apiObjects.getApiObjectContainer<LogicObject>().back());
        EXPECT_EQ(EPropertyType::Float, m_apiObjects.getApiObjectContainer<DataArray>().front()->getDataType());
        ASSERT_NE(nullptr, m_apiObjects.getApiObjectContainer<DataArray>().front()->getData<float>());
        EXPECT_EQ(data, *m_apiObjects.getApiObjectContainer<DataArray>().front()->getData<float>());
    }

    TEST_P(AnApiObjects, DestroysDataArray)
    {
        auto dataArray = m_apiObjects.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        EXPECT_TRUE(m_apiObjects.destroy(*dataArray, m_errorReporting));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<DataArray>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, FailsToDestroysDataArrayIfUsedInAnimationNode)
    {
        auto dataArray1 = m_apiObjects.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data1");
        auto dataArray2 = m_apiObjects.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data2");
        auto dataArray3 = m_apiObjects.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data3");
        auto dataArray4 = m_apiObjects.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data4");

        AnimationNodeConfig config;
        EXPECT_TRUE(config.addChannel({ "channel1", dataArray1, dataArray2 }));
        EXPECT_TRUE(config.addChannel({ "channel2", dataArray1, dataArray2, EInterpolationType::Cubic, dataArray3, dataArray4 }));
        auto animNode = m_apiObjects.createAnimationNode(*config.m_impl, "animNode");

        EXPECT_FALSE(m_apiObjects.destroy(*dataArray1, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Failed to destroy data array 'data1', it is used in animation node 'animNode' channel 'channel1'");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, dataArray1);
        m_errorReporting.clear();

        EXPECT_FALSE(m_apiObjects.destroy(*dataArray2, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Failed to destroy data array 'data2', it is used in animation node 'animNode' channel 'channel1'");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, dataArray2);
        m_errorReporting.clear();

        EXPECT_FALSE(m_apiObjects.destroy(*dataArray3, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Failed to destroy data array 'data3', it is used in animation node 'animNode' channel 'channel2'");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, dataArray3);
        m_errorReporting.clear();

        EXPECT_FALSE(m_apiObjects.destroy(*dataArray4,  m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Failed to destroy data array 'data4', it is used in animation node 'animNode' channel 'channel2'");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, dataArray4);
        m_errorReporting.clear();

        // succeeds after destroying animation node
        EXPECT_TRUE(m_apiObjects.destroy(*animNode, m_errorReporting));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.destroy(*dataArray1, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*dataArray2, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*dataArray3, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*dataArray4, m_errorReporting));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
    }

    TEST_P(AnApiObjects, FailsToDestroyDataArrayFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam() };
        auto dataArray = otherInstance.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        ASSERT_NE(nullptr, dataArray);
        EXPECT_EQ(dataArray, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(dataArray, otherInstance.getApiObjectContainer<LogicObject>().back());
        EXPECT_FALSE(m_apiObjects.destroy(*dataArray, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Can't find data array in logic engine!");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, dataArray);

        // Did not affect existence in otherInstance!
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<DataArray>().empty());
        EXPECT_EQ(dataArray, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(dataArray, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesAnimationNode)
    {
        auto dataArray = m_apiObjects.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        ASSERT_NE(nullptr, dataArray);
        AnimationNodeConfig config;
        EXPECT_TRUE(config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear }));
        auto animNode = m_apiObjects.createAnimationNode(*config.m_impl, "animNode");
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_EQ(animNode, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(animNode, m_apiObjects.getApiObjectContainer<LogicObject>().back());
        EXPECT_EQ(2u, m_apiObjects.getApiObjectContainer<LogicObject>().size());
        EXPECT_EQ(2u, m_apiObjects.getApiObjectOwningContainer().size());
        ASSERT_EQ(1u, m_apiObjects.getApiObjectContainer<AnimationNode>().size());
        EXPECT_EQ(animNode, m_apiObjects.getApiObjectContainer<AnimationNode>().front());
    }

    TEST_P(AnApiObjects, DestroysAnimationNode)
    {
        auto dataArray = m_apiObjects.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        ASSERT_NE(nullptr, dataArray);
        AnimationNodeConfig config;
        EXPECT_TRUE(config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear }));
        auto animNode = m_apiObjects.createAnimationNode(*config.m_impl, "animNode");
        EXPECT_TRUE(m_apiObjects.destroy(*animNode, m_errorReporting));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<AnimationNode>().empty());
        // did not affect data array
        EXPECT_TRUE(!m_apiObjects.getApiObjectContainer<DataArray>().empty());
        EXPECT_EQ(1u, m_apiObjects.getApiObjectOwningContainer().size());
        EXPECT_EQ(1u, m_apiObjects.getApiObjectContainer<LogicObject>().size());
    }

    TEST_P(AnApiObjects, FailsToDestroyAnimationNodeFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam() };
        auto dataArray = otherInstance.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        ASSERT_NE(nullptr, dataArray);
        AnimationNodeConfig config;
        EXPECT_TRUE(config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear }));
        auto animNode = otherInstance.createAnimationNode(*config.m_impl, "animNode");
        EXPECT_EQ(animNode, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(animNode, otherInstance.getApiObjectContainer<LogicObject>().back());
        EXPECT_FALSE(m_apiObjects.destroy(*animNode, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Can't find AnimationNode in logic engine!");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, animNode);

        // Did not affect existence in otherInstance!
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<AnimationNode>().empty());
        EXPECT_EQ(animNode, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(animNode, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesTimerNode)
    {
        auto timerNode = m_apiObjects.createTimerNode("timerNode");
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        ASSERT_EQ(1u, m_apiObjects.getApiObjectOwningContainer().size());
        EXPECT_EQ(timerNode, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_THAT(m_apiObjects.getApiObjectContainer<LogicObject>(), ::testing::ElementsAre(timerNode));
        EXPECT_THAT(m_apiObjects.getApiObjectContainer<TimerNode>(), ::testing::ElementsAre(timerNode));
    }

    TEST_P(AnApiObjects, DestroysTimerNode)
    {
        auto timerNode = m_apiObjects.createTimerNode("timerNode");
        EXPECT_TRUE(m_apiObjects.destroy(*timerNode, m_errorReporting));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<TimerNode>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, FailsToDestroyTimerNodeFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam() };
        auto timerNode = otherInstance.createTimerNode("timerNode");
        EXPECT_FALSE(m_apiObjects.destroy(*timerNode, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Can't find TimerNode in logic engine!");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, timerNode);

        // Did not affect existence in otherInstance!
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<TimerNode>().empty());
        EXPECT_EQ(timerNode, otherInstance.getApiObjectContainer<TimerNode>().front());
        EXPECT_EQ(timerNode, otherInstance.getApiObjectOwningContainer().front().get());
        EXPECT_EQ(timerNode, otherInstance.getApiObjectContainer<LogicObject>().front());
    }

    TEST_P(AnApiObjects, CreatesAnchorPointWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        AnchorPoint* anchor = createAnchorPoint();
        EXPECT_NE(nullptr, anchor);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_EQ(anchor, m_apiObjects.getApiObjectContainer<AnchorPoint>().front());
        EXPECT_EQ(anchor, m_apiObjects.getApiObject(anchor->m_impl));
        EXPECT_EQ(anchor, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(anchor, m_apiObjects.getApiObjectContainer<LogicObject>().back());
        EXPECT_EQ(anchor, m_apiObjects.getApiObject(anchor->m_impl));
    }

    TEST_P(AnApiObjects, DestroysAnchorPointWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        AnchorPoint* anchor = createAnchorPoint();
        ASSERT_NE(nullptr, anchor);
        m_apiObjects.destroy(*anchor, m_errorReporting);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<AnchorPoint>().empty());
        EXPECT_NE(anchor, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_NE(anchor, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingAnchorPointFromAnotherClassInstance)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        ApiObjects otherInstance{ GetParam() };
        const auto node = otherInstance.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
        const auto camera = otherInstance.createRamsesCameraBinding(*m_camera, true, "camera");
        AnchorPoint* anchor = otherInstance.createAnchorPoint(node->m_nodeBinding, camera->m_cameraBinding, "anchor");
        ASSERT_TRUE(anchor);
        EXPECT_EQ(anchor, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(anchor, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*anchor, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Can't find AnchorPoint in logic engine!");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, anchor);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(anchor, otherInstance.getApiObject(anchor->m_impl));
        EXPECT_EQ(anchor, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(anchor, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, FailsToDestroyNodeOrCameraBindingIfUsedInAnchorPoint)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        const auto node = m_apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
        const auto camera = m_apiObjects.createRamsesCameraBinding(*m_camera, true, "camera");
        AnchorPoint* anchor = m_apiObjects.createAnchorPoint(node->m_nodeBinding, camera->m_cameraBinding, "anchor");

        EXPECT_FALSE(m_apiObjects.destroy(*node, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Failed to destroy Ramses node binding 'node', it is used in anchor point 'anchor'");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, node);
        m_errorReporting.clear();

        EXPECT_FALSE(m_apiObjects.destroy(*camera, m_errorReporting));
        EXPECT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Failed to destroy Ramses camera binding 'camera', it is used in anchor point 'anchor'");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, camera);
        m_errorReporting.clear();

        // succeeds after destroying anchor point
        EXPECT_TRUE(m_apiObjects.destroy(*anchor, m_errorReporting));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.destroy(*node, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*camera, m_errorReporting));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
    }

    TEST_P(AnApiObjects, CreatesSkinBindingWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        auto skin = createSkinBinding(m_apiObjects);
        ASSERT_NE(nullptr, skin);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_EQ(skin, m_apiObjects.getApiObjectContainer<SkinBinding>().front());
        EXPECT_EQ(skin, m_apiObjects.getApiObject(skin->m_impl));
        EXPECT_EQ(skin, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(skin, m_apiObjects.getApiObjectContainer<LogicObject>().back());
        EXPECT_EQ(skin, m_apiObjects.getApiObject(skin->m_impl));
    }

    TEST_P(AnApiObjects, DestroysSkinBindingWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        auto skin = createSkinBinding(m_apiObjects);
        ASSERT_NE(nullptr, skin);
        m_apiObjects.destroy(*skin, m_errorReporting);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<SkinBinding>().empty());
        EXPECT_NE(skin, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_NE(skin, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingSkinBindingFromAnotherClassInstance)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        auto skin = createSkinBinding(m_apiObjects);
        ASSERT_NE(nullptr, skin);

        ApiObjects otherInstance{ GetParam() };

        ErrorReporting  errorReporting;
        EXPECT_FALSE(otherInstance.destroy(*skin, errorReporting));
        ASSERT_EQ(errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(errorReporting.getErrors()[0].message, "Can't find SkinBinding in logic engine!");
        EXPECT_EQ(errorReporting.getErrors()[0].object, skin);
    }

    TEST_P(AnApiObjects, FailsToDestroyNodeOrAppearanceBindingIfUsedInSkinBinding)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        auto skin = createSkinBinding(m_apiObjects);
        ASSERT_NE(nullptr, skin);

        const auto& nodes = m_apiObjects.getApiObjectContainer<RamsesNodeBinding>();
        const auto it = std::find_if(nodes.cbegin(), nodes.cend(), [](const auto& n) { return n->getName() == "nodeForSkin"; });
        ASSERT_TRUE(it != nodes.cend());
        const auto nodeUsedInSkin = *it;

        const auto& appearances = m_apiObjects.getApiObjectContainer<RamsesAppearanceBinding>();
        const auto it2 = std::find_if(appearances.cbegin(), appearances.cend(), [](const auto& a) { return a->getName() == "appearanceForSkin"; });
        ASSERT_TRUE(it2 != appearances.cend());
        const auto appearanceUsedInSkin = *it2;

        EXPECT_FALSE(m_apiObjects.destroy(*nodeUsedInSkin, m_errorReporting));
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Failed to destroy Ramses node binding 'nodeForSkin', it is used in skin binding 'skin'");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, nodeUsedInSkin);
        m_errorReporting.clear();

        EXPECT_FALSE(m_apiObjects.destroy(*appearanceUsedInSkin, m_errorReporting));
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Failed to destroy Ramses appearance binding 'appearanceForSkin', it is used in skin binding 'skin'");
        EXPECT_EQ(m_errorReporting.getErrors()[0].object, appearanceUsedInSkin);
        m_errorReporting.clear();

        // succeeds after destroying skin binding
        EXPECT_TRUE(m_apiObjects.destroy(*skin, m_errorReporting));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
        EXPECT_TRUE(m_apiObjects.destroy(*nodeUsedInSkin, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*appearanceUsedInSkin, m_errorReporting));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
    }

    TEST_P(AnApiObjects, ProvidesEmptyCollections_WhenNothingWasCreated)
    {
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LuaScript>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<RamsesNodeBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<RamsesAppearanceBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<RamsesCameraBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<RamsesRenderPassBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<RamsesRenderGroupBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<RamsesMeshNodeBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<DataArray>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<AnimationNode>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<TimerNode>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<AnchorPoint>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<SkinBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getReverseImplMapping().empty());

        const ApiObjects& apiObjectsConst = m_apiObjects;
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<LuaScript>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<RamsesNodeBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<RamsesAppearanceBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<RamsesCameraBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<RamsesRenderPassBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<RamsesRenderGroupBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<RamsesMeshNodeBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<DataArray>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<AnimationNode>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<TimerNode>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<AnchorPoint>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<SkinBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<LogicObject>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(apiObjectsConst.getReverseImplMapping().empty());
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyScriptCollection_WhenScriptsWereCreated)
    {
        const LuaScript* script = createScript();
        ApiObjectContainer<LuaScript>& scripts = m_apiObjects.getApiObjectContainer<LuaScript>();

        EXPECT_EQ(*scripts.begin(), script);
        EXPECT_EQ(*scripts.cbegin(), script);

        EXPECT_NE(scripts.begin(), scripts.end());
        EXPECT_NE(scripts.cbegin(), scripts.cend());

        EXPECT_EQ(script, *scripts.begin());
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyInterfaceCollection_WhenInterfacesWereCreated)
    {
        const LuaInterface* intf = createInterface();
        EXPECT_THAT(m_apiObjects.getApiObjectContainer<LuaInterface>(), ::testing::ElementsAre(intf));
        const auto& constApiObjects = m_apiObjects;
        EXPECT_THAT(constApiObjects.getApiObjectContainer<LuaInterface>(), ::testing::ElementsAre(intf));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyNodeBindingsCollection_WhenNodeBindingsWereCreated)
    {
        RamsesNodeBinding* binding = m_apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        ApiObjectContainer<RamsesNodeBinding>& nodes = m_apiObjects.getApiObjectContainer<RamsesNodeBinding>();
        EXPECT_THAT(nodes, ::testing::ElementsAre(binding));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyAppearanceBindingsCollection_WhenAppearanceBindingsWereCreated)
    {
        RamsesAppearanceBinding* binding = m_apiObjects.createRamsesAppearanceBinding(*m_appearance, "");
        ApiObjectContainer<RamsesAppearanceBinding>& appearances = m_apiObjects.getApiObjectContainer<RamsesAppearanceBinding>();
        EXPECT_THAT(appearances, ::testing::ElementsAre(binding));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyCameraBindingsCollection_WhenCameraBindingsWereCreated)
    {
        RamsesCameraBinding* binding = m_apiObjects.createRamsesCameraBinding(*m_camera, true, "");
        ApiObjectContainer<RamsesCameraBinding>& cameras = m_apiObjects.getApiObjectContainer<RamsesCameraBinding>();
        EXPECT_THAT(cameras, ::testing::ElementsAre(binding));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyRenderPassBindingsCollection_WhenRenderPassBindingsWereCreated)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        RamsesRenderPassBinding* binding = m_apiObjects.createRamsesRenderPassBinding(*m_renderPass, "");
        ApiObjectContainer<RamsesRenderPassBinding>& renderPasses = m_apiObjects.getApiObjectContainer<RamsesRenderPassBinding>();
        EXPECT_THAT(renderPasses, ::testing::ElementsAre(binding));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyRenderGroupBindingsCollection_WhenRenderGroupBindingsWereCreated)
    {
        if (GetParam() < EFeatureLevel_03)
            GTEST_SKIP();

        const auto binding = createRenderGroupBinding();
        ApiObjectContainer<RamsesRenderGroupBinding>& renderGroups = m_apiObjects.getApiObjectContainer<RamsesRenderGroupBinding>();
        EXPECT_THAT(renderGroups, ::testing::ElementsAre(binding));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyAnchorPointsCollection_WhenAnchorPointsWereCreated)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        AnchorPoint* anchor = createAnchorPoint();
        const auto& anchors = m_apiObjects.getApiObjectContainer<AnchorPoint>();
        EXPECT_THAT(anchors, ::testing::ElementsAre(anchor));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptySkinBindingsCollection_WhenSkinBindingsWereCreated)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        const auto skin = createSkinBinding(m_apiObjects);
        const auto& skins = m_apiObjects.getApiObjectContainer<SkinBinding>();
        EXPECT_THAT(skins, ::testing::ElementsAre(skin));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyOwningAndLogicObjectsCollection_WhenLogicObjectsWereCreated)
    {
        const ApiObjectContainer<LogicObject>& logicObjects = m_apiObjects.getApiObjectContainer<LogicObject>();
        const ApiObjectOwningContainer& ownedObjects = m_apiObjects.getApiObjectOwningContainer();

        auto* luaModule         = m_apiObjects.createLuaModule(m_moduleSrc, {}, "module", m_errorReporting);
        auto* luaScript         = createScript(m_apiObjects, m_valid_empty_script);
        auto* luaInterface      = createInterface();
        auto* nodeBinding       = m_apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        auto* appearanceBinding = m_apiObjects.createRamsesAppearanceBinding(*m_appearance, "");
        auto* cameraBinding     = m_apiObjects.createRamsesCameraBinding(*m_camera, true, "");
        auto* dataArray         = m_apiObjects.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "data");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
        auto* animationNode     = m_apiObjects.createAnimationNode(*config.m_impl, "animNode");
        auto* timerNode         = m_apiObjects.createTimerNode("timerNode");

        // feature level 01 always present
        std::vector<LogicObject*> expectedObjects{ luaModule, luaScript, luaInterface, nodeBinding, appearanceBinding, cameraBinding, dataArray, animationNode, timerNode };

        if (GetParam() >= EFeatureLevel_02)
        {
            auto* renderPassBinding = m_apiObjects.createRamsesRenderPassBinding(*m_renderPass, "");
            auto* anchor = m_apiObjects.createAnchorPoint(nodeBinding->m_nodeBinding, cameraBinding->m_cameraBinding, "anchor");
            expectedObjects.push_back(renderPassBinding);
            expectedObjects.push_back(anchor);
        }

        if (GetParam() >= EFeatureLevel_03)
        {
            auto* renderGroupBinding = createRenderGroupBinding();
            expectedObjects.push_back(renderGroupBinding);
        }

        if (GetParam() >= EFeatureLevel_04)
        {
            auto* skin = createSkinBinding(*nodeBinding, *appearanceBinding, m_apiObjects);
            expectedObjects.push_back(skin);
        }

        if (GetParam() >= EFeatureLevel_05)
        {
            auto* meshBinding = m_apiObjects.createRamsesMeshNodeBinding(*m_meshNode, "mb");
            expectedObjects.push_back(meshBinding);
        }

        std::vector<LogicObject*> ownedLogicObjectsRawPointers;
        std::transform(ownedObjects.cbegin(), ownedObjects.cend(), std::back_inserter(ownedLogicObjectsRawPointers), [&](std::unique_ptr<LogicObject>const& obj) { return obj.get(); });
        EXPECT_EQ(logicObjects, expectedObjects);
        EXPECT_EQ(ownedLogicObjectsRawPointers, expectedObjects);

        const ApiObjects& apiObjectsConst = m_apiObjects;
        EXPECT_EQ(logicObjects, apiObjectsConst.getApiObjectContainer<LogicObject>());
    }

    TEST_P(AnApiObjects, logicObjectsGetUniqueIds)
    {
        const auto* luaModule         = m_apiObjects.createLuaModule(m_moduleSrc, {}, "module", m_errorReporting);
        const auto* luaScript         = createScript(m_apiObjects, m_valid_empty_script);
        const auto* luaInterface      = createInterface();
        const auto* nodeBinding       = m_apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        const auto* appearanceBinding = m_apiObjects.createRamsesAppearanceBinding(*m_appearance, "");
        const auto* cameraBinding     = m_apiObjects.createRamsesCameraBinding(*m_camera, true, "");
        const auto* dataArray         = m_apiObjects.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "data");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
        const auto* animationNode     = m_apiObjects.createAnimationNode(*config.m_impl, "animNode");
        const auto* timerNode         = m_apiObjects.createTimerNode("timerNode");

        std::unordered_set<uint64_t> logicObjectIds =
        {
            luaModule->getId(),
            luaScript->getId(),
            luaInterface->getId(),
            nodeBinding->getId(),
            appearanceBinding->getId(),
            cameraBinding->getId(),
            dataArray->getId(),
            animationNode->getId(),
            timerNode->getId()
        };
        EXPECT_EQ(9u, logicObjectIds.size());

        if (GetParam() >= EFeatureLevel_02)
        {
            const auto* renderPassBinding = m_apiObjects.createRamsesRenderPassBinding(*m_renderPass, "");
            const auto* anchor = createAnchorPoint();
            logicObjectIds.insert(renderPassBinding->getId());
            logicObjectIds.insert(anchor->getId());
            EXPECT_EQ(11u, logicObjectIds.size());
        }

        if (GetParam() >= EFeatureLevel_03)
        {
            const auto* renderGroupBinding = createRenderGroupBinding();
            logicObjectIds.insert(renderGroupBinding->getId());
            EXPECT_EQ(12u, logicObjectIds.size());
        }

        if (GetParam() >= EFeatureLevel_04)
        {
            const auto* skin = createSkinBinding(*nodeBinding, *appearanceBinding, m_apiObjects);
            logicObjectIds.insert(skin->getId());
            EXPECT_EQ(13u, logicObjectIds.size());
        }

        if (GetParam() >= EFeatureLevel_05)
        {
            const auto* meshBinding = m_apiObjects.createRamsesMeshNodeBinding(*m_meshNode, "mb");
            logicObjectIds.insert(meshBinding->getId());
            EXPECT_EQ(14u, logicObjectIds.size());
        }
    }

    TEST_P(AnApiObjects, canGetLogicObjectById)
    {
        const auto* luaModule         = m_apiObjects.createLuaModule(m_moduleSrc, {}, "module", m_errorReporting);
        const auto* luaScript         = createScript(m_apiObjects, m_valid_empty_script);
        const auto* nodeBinding       = m_apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        const auto* appearanceBinding = m_apiObjects.createRamsesAppearanceBinding(*m_appearance, "");
        const auto* cameraBinding     = m_apiObjects.createRamsesCameraBinding(*m_camera, true, "");
        const auto* dataArray         = m_apiObjects.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "data");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
        const auto* animationNode     = m_apiObjects.createAnimationNode(*config.m_impl, "animNode");
        const auto* timerNode         = m_apiObjects.createTimerNode("timerNode");
        const auto* luaInterface      = createInterface();

        EXPECT_EQ(luaModule->getId(), 1u);
        EXPECT_EQ(luaScript->getId(), 2u);
        EXPECT_EQ(nodeBinding->getId(), 3u);
        EXPECT_EQ(appearanceBinding->getId(), 4u);
        EXPECT_EQ(cameraBinding->getId(), 5u);
        EXPECT_EQ(dataArray->getId(), 6u);
        EXPECT_EQ(animationNode->getId(), 7u);
        EXPECT_EQ(timerNode->getId(), 8u);
        EXPECT_EQ(luaInterface->getId(), 9u);

        EXPECT_EQ(m_apiObjects.getApiObjectById(1u), luaModule);
        EXPECT_EQ(m_apiObjects.getApiObjectById(2u), luaScript);
        EXPECT_EQ(m_apiObjects.getApiObjectById(3u), nodeBinding);
        EXPECT_EQ(m_apiObjects.getApiObjectById(4u), appearanceBinding);
        EXPECT_EQ(m_apiObjects.getApiObjectById(5u), cameraBinding);
        EXPECT_EQ(m_apiObjects.getApiObjectById(6u), dataArray);
        EXPECT_EQ(m_apiObjects.getApiObjectById(7u), animationNode);
        EXPECT_EQ(m_apiObjects.getApiObjectById(8u), timerNode);
        EXPECT_EQ(m_apiObjects.getApiObjectById(9u), luaInterface);

        if (GetParam() >= EFeatureLevel_02)
        {
            const auto* renderPassBinding = m_apiObjects.createRamsesRenderPassBinding(*m_renderPass, "");
            EXPECT_EQ(renderPassBinding->getId(), 10u);
            EXPECT_EQ(m_apiObjects.getApiObjectById(10u), renderPassBinding);
            const auto* anchor = createAnchorPoint();
            EXPECT_EQ(anchor->getId(), 13u);
            EXPECT_EQ(m_apiObjects.getApiObjectById(13u), anchor);
        }

        if (GetParam() >= EFeatureLevel_03)
        {
            const auto* renderGroupBinding = createRenderGroupBinding();
            EXPECT_EQ(renderGroupBinding->getId(), 14u);
            EXPECT_EQ(m_apiObjects.getApiObjectById(14u), renderGroupBinding);
        }

        if (GetParam() >= EFeatureLevel_04)
        {
            const auto* skin = createSkinBinding(*nodeBinding, *appearanceBinding, m_apiObjects);
            EXPECT_EQ(skin->getId(), 15u);
            EXPECT_EQ(m_apiObjects.getApiObjectById(15u), skin);
        }

        if (GetParam() >= EFeatureLevel_05)
        {
            const auto* meshBinding = m_apiObjects.createRamsesMeshNodeBinding(*m_meshNode, "mb");
            EXPECT_EQ(meshBinding->getId(), 16u);
            EXPECT_EQ(m_apiObjects.getApiObjectById(16u), meshBinding);
        }
    }

    TEST_P(AnApiObjects, logicObjectIdsAreRemovedFromIdMappingWhenObjectIsDestroyed)
    {
        const auto* luaModule         = m_apiObjects.createLuaModule(m_moduleSrc, {}, "module", m_errorReporting);
        auto* luaScript               = createScript(m_apiObjects, m_valid_empty_script);
        const auto* nodeBinding       = m_apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        auto* appearanceBinding       = m_apiObjects.createRamsesAppearanceBinding(*m_appearance, "");
        const auto* cameraBinding     = m_apiObjects.createRamsesCameraBinding(*m_camera, true, "");
        const auto* dataArray         = m_apiObjects.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "data");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
        auto* animationNode           = m_apiObjects.createAnimationNode(*config.m_impl, "animNode");
        const auto* timerNode         = m_apiObjects.createTimerNode("timerNode");
        const auto* luaInterface      = createInterface();
        const RamsesRenderPassBinding* renderPassBinding = nullptr;
        const AnchorPoint* anchor = nullptr;
        if (GetParam() >= EFeatureLevel_02)
        {
            renderPassBinding = m_apiObjects.createRamsesRenderPassBinding(*m_renderPass, "");
            anchor = createAnchorPoint();
        }
        const RamsesRenderGroupBinding* renderGroupBinding = nullptr;
        if (GetParam() >= EFeatureLevel_03)
            renderGroupBinding = createRenderGroupBinding();
        const SkinBinding* skinBinding = nullptr;
        if (GetParam() >= EFeatureLevel_04)
            skinBinding = createSkinBinding(m_apiObjects);
        const RamsesMeshNodeBinding* meshBinding = nullptr;
        if (GetParam() >= EFeatureLevel_05)
            meshBinding = m_apiObjects.createRamsesMeshNodeBinding(*m_meshNode, "mb");

        EXPECT_EQ(m_apiObjects.getApiObjectById(1u), luaModule);
        EXPECT_EQ(m_apiObjects.getApiObjectById(2u), luaScript);
        EXPECT_EQ(m_apiObjects.getApiObjectById(3u), nodeBinding);
        EXPECT_EQ(m_apiObjects.getApiObjectById(4u), appearanceBinding);
        EXPECT_EQ(m_apiObjects.getApiObjectById(5u), cameraBinding);
        EXPECT_EQ(m_apiObjects.getApiObjectById(6u), dataArray);
        EXPECT_EQ(m_apiObjects.getApiObjectById(7u), animationNode);
        EXPECT_EQ(m_apiObjects.getApiObjectById(8u), timerNode);
        EXPECT_EQ(m_apiObjects.getApiObjectById(9u), luaInterface);
        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_EQ(m_apiObjects.getApiObjectById(10u), renderPassBinding);
            EXPECT_EQ(m_apiObjects.getApiObjectById(13u), anchor);
        }
        if (GetParam() >= EFeatureLevel_03)
        {
            EXPECT_EQ(m_apiObjects.getApiObjectById(14u), renderGroupBinding);
        }
        if (GetParam() >= EFeatureLevel_04)
        {
            EXPECT_EQ(m_apiObjects.getApiObjectById(17u), skinBinding);
        }
        if (GetParam() >= EFeatureLevel_05)
        {
            EXPECT_EQ(m_apiObjects.getApiObjectById(18u), meshBinding);
        }

        EXPECT_TRUE(m_apiObjects.destroy(*luaScript, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*appearanceBinding, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*animationNode, m_errorReporting));

        EXPECT_EQ(m_apiObjects.getApiObjectById(1u), luaModule);
        EXPECT_EQ(m_apiObjects.getApiObjectById(2u), nullptr);
        EXPECT_EQ(m_apiObjects.getApiObjectById(3u), nodeBinding);
        EXPECT_EQ(m_apiObjects.getApiObjectById(4u), nullptr);
        EXPECT_EQ(m_apiObjects.getApiObjectById(5u), cameraBinding);
        EXPECT_EQ(m_apiObjects.getApiObjectById(6u), dataArray);
        EXPECT_EQ(m_apiObjects.getApiObjectById(7u), nullptr);
        EXPECT_EQ(m_apiObjects.getApiObjectById(8u), timerNode);
        EXPECT_EQ(m_apiObjects.getApiObjectById(9u), luaInterface);
        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_EQ(m_apiObjects.getApiObjectById(10u), renderPassBinding);
            EXPECT_EQ(m_apiObjects.getApiObjectById(13u), anchor);
        }
        if (GetParam() >= EFeatureLevel_03)
        {
            EXPECT_EQ(m_apiObjects.getApiObjectById(14u), renderGroupBinding);
        }
        if (GetParam() >= EFeatureLevel_04)
        {
            EXPECT_EQ(m_apiObjects.getApiObjectById(17u), skinBinding);
        }
        if (GetParam() >= EFeatureLevel_05)
        {
            EXPECT_EQ(m_apiObjects.getApiObjectById(18u), meshBinding);
        }
    }

    TEST_P(AnApiObjects, logicObjectsGenerateIdentificationString)
    {
        const auto* luaModule = m_apiObjects.createLuaModule(m_moduleSrc, {}, "module", m_errorReporting);
        auto* luaScript = createScript(m_apiObjects, m_valid_empty_script);
        const auto* nodeBinding = m_apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodeBinding");
        auto* appearanceBinding = m_apiObjects.createRamsesAppearanceBinding(*m_appearance, "appearanceBinding");
        const auto* cameraBinding = m_apiObjects.createRamsesCameraBinding(*m_camera, true, "cameraBinding");
        const auto* dataArray = m_apiObjects.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataArray");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
        auto* animationNode = m_apiObjects.createAnimationNode(*config.m_impl, "animNode");
        const auto* timerNode = m_apiObjects.createTimerNode("timerNode");
        const auto* luaInterface = createInterface();

        EXPECT_EQ(luaModule->m_impl.getIdentificationString(), "module [Id=1]");
        EXPECT_EQ(luaScript->m_impl.getIdentificationString(), "script [Id=2]");
        EXPECT_EQ(nodeBinding->m_impl.getIdentificationString(), "nodeBinding [Id=3]");
        EXPECT_EQ(appearanceBinding->m_impl.getIdentificationString(), "appearanceBinding [Id=4]");
        EXPECT_EQ(cameraBinding->m_impl.getIdentificationString(), "cameraBinding [Id=5]");
        EXPECT_EQ(dataArray->m_impl.getIdentificationString(), "dataArray [Id=6]");
        EXPECT_EQ(animationNode->m_impl.getIdentificationString(), "animNode [Id=7]");
        EXPECT_EQ(timerNode->m_impl.getIdentificationString(), "timerNode [Id=8]");
        EXPECT_EQ(luaInterface->m_impl.getIdentificationString(), "intf [Id=9]");

        if (GetParam() >= EFeatureLevel_02)
        {
            const auto* renderPassBinding = m_apiObjects.createRamsesRenderPassBinding(*m_renderPass, "renderPassBinding");
            EXPECT_EQ(renderPassBinding->m_impl.getIdentificationString(), "renderPassBinding [Id=10]");
            const auto* anchor = createAnchorPoint();
            EXPECT_EQ(anchor->m_impl.getIdentificationString(), "anchor [Id=13]");
        }
        if (GetParam() >= EFeatureLevel_03)
        {
            const auto renderGroupBinding = createRenderGroupBinding();
            EXPECT_EQ(renderGroupBinding->m_impl.getIdentificationString(), "renderGroupBinding [Id=14]");
        }
        if (GetParam() >= EFeatureLevel_04)
        {
            const auto skin = createSkinBinding(m_apiObjects);
            EXPECT_EQ(skin->m_impl.getIdentificationString(), "skin [Id=17]");
        }
        if (GetParam() >= EFeatureLevel_05)
        {
            const auto* meshBinding = m_apiObjects.createRamsesMeshNodeBinding(*m_meshNode, "mb");
            EXPECT_EQ(meshBinding->m_impl.getIdentificationString(), "mb [Id=18]");
        }
    }

    TEST_P(AnApiObjects, logicObjectsGenerateIdentificationStringWithUserId)
    {
        auto* luaModule = m_apiObjects.createLuaModule(m_moduleSrc, {}, "module", m_errorReporting);
        auto* luaScript = createScript(m_apiObjects, m_valid_empty_script);
        auto* nodeBinding = m_apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodeBinding");
        auto* appearanceBinding = m_apiObjects.createRamsesAppearanceBinding(*m_appearance, "appearanceBinding");
        auto* cameraBinding = m_apiObjects.createRamsesCameraBinding(*m_camera, true, "cameraBinding");
        RamsesRenderPassBinding* renderPassBinding = nullptr;
        auto* dataArray = m_apiObjects.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataArray");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
        auto* animationNode = m_apiObjects.createAnimationNode(*config.m_impl, "animNode");
        auto* timerNode = m_apiObjects.createTimerNode("timerNode");
        auto* luaInterface = createInterface();
        AnchorPoint* anchor = nullptr;
        if (GetParam() >= EFeatureLevel_02)
        {
            renderPassBinding = m_apiObjects.createRamsesRenderPassBinding(*m_renderPass, "renderPassBinding");
            anchor = createAnchorPoint();
        }
        RamsesRenderGroupBinding* renderGroupBinding = nullptr;
        if (GetParam() >= EFeatureLevel_03)
            renderGroupBinding = createRenderGroupBinding();
        SkinBinding* skin = nullptr;
        if (GetParam() >= EFeatureLevel_04)
            skin = createSkinBinding(m_apiObjects);
        RamsesMeshNodeBinding* meshBinding = nullptr;
        if (GetParam() >= EFeatureLevel_05)
            meshBinding = m_apiObjects.createRamsesMeshNodeBinding(*m_meshNode, "mb");

        EXPECT_TRUE(luaModule->setUserId(1u, 2u));
        EXPECT_TRUE(luaScript->setUserId(3u, 4u));
        EXPECT_TRUE(nodeBinding->setUserId(5u, 6u));
        EXPECT_TRUE(appearanceBinding->setUserId(7u, 8u));
        EXPECT_TRUE(cameraBinding->setUserId(9u, 10u));
        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_TRUE(renderPassBinding->setUserId(11u, 12u));
        }
        EXPECT_TRUE(dataArray->setUserId(13u, 14u));
        EXPECT_TRUE(animationNode->setUserId(15u, 16u));
        EXPECT_TRUE(timerNode->setUserId(17u, 18u));
        EXPECT_TRUE(luaInterface->setUserId(19u, 20u));
        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_TRUE(anchor->setUserId(21u, 22u));
        }
        if (GetParam() >= EFeatureLevel_03)
        {
            EXPECT_TRUE(renderGroupBinding->setUserId(23u, 24u));
        }
        if (GetParam() >= EFeatureLevel_04)
        {
            EXPECT_TRUE(skin->setUserId(25u, 26u));
        }
        if (GetParam() >= EFeatureLevel_05)
        {
            EXPECT_TRUE(meshBinding->setUserId(27u, 28u));
        }

        EXPECT_EQ(luaModule->m_impl.getIdentificationString(), "module [Id=1 UserId=00000000000000010000000000000002]");
        EXPECT_EQ(luaScript->m_impl.getIdentificationString(), "script [Id=2 UserId=00000000000000030000000000000004]");
        EXPECT_EQ(nodeBinding->m_impl.getIdentificationString(), "nodeBinding [Id=3 UserId=00000000000000050000000000000006]");
        EXPECT_EQ(appearanceBinding->m_impl.getIdentificationString(), "appearanceBinding [Id=4 UserId=00000000000000070000000000000008]");
        EXPECT_EQ(cameraBinding->m_impl.getIdentificationString(), "cameraBinding [Id=5 UserId=0000000000000009000000000000000A]");
        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_EQ(renderPassBinding->m_impl.getIdentificationString(), "renderPassBinding [Id=10 UserId=000000000000000B000000000000000C]");
        }
        EXPECT_EQ(dataArray->m_impl.getIdentificationString(), "dataArray [Id=6 UserId=000000000000000D000000000000000E]");
        EXPECT_EQ(animationNode->m_impl.getIdentificationString(), "animNode [Id=7 UserId=000000000000000F0000000000000010]");
        EXPECT_EQ(timerNode->m_impl.getIdentificationString(), "timerNode [Id=8 UserId=00000000000000110000000000000012]");
        EXPECT_EQ(luaInterface->m_impl.getIdentificationString(), "intf [Id=9 UserId=00000000000000130000000000000014]");
        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_EQ(anchor->m_impl.getIdentificationString(), "anchor [Id=13 UserId=00000000000000150000000000000016]");
        }
        if (GetParam() >= EFeatureLevel_03)
        {
            EXPECT_EQ(renderGroupBinding->m_impl.getIdentificationString(), "renderGroupBinding [Id=14 UserId=00000000000000170000000000000018]");
        }
        if (GetParam() >= EFeatureLevel_04)
        {
            EXPECT_EQ(skin->m_impl.getIdentificationString(), "skin [Id=17 UserId=0000000000000019000000000000001A]");
        }
        if (GetParam() >= EFeatureLevel_05)
        {
            EXPECT_EQ(meshBinding->m_impl.getIdentificationString(), "mb [Id=18 UserId=000000000000001B000000000000001C]");
        }
    }

    TEST_P(AnApiObjects, ValidatesThatAllLuaInterfaceOutputsAreLinked_GeneratesWarningsIfOutputsNotLinked)
    {
        LuaInterface* intf = m_apiObjects.createLuaInterface(R"(
            function interface(IN,OUT)

                IN.param1 = Type:Int32()
                IN.param2 = {a=Type:Float(), b=Type:Int32()}

            end
        )", {}, "intf name", m_errorReporting, true);
        ASSERT_NE(nullptr, intf);

        ValidationResults validationResults;
        m_apiObjects.validateInterfaces(validationResults);
        EXPECT_EQ(3u, validationResults.getWarnings().size());
        EXPECT_THAT(validationResults.getWarnings(),
            ::testing::Each(::testing::Field(&WarningData::message, ::testing::HasSubstr("Interface [intf name] has unlinked output"))));
        EXPECT_THAT(validationResults.getWarnings(), ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(AnApiObjects, ValidatesThatAllLuaInterfaceOutputsAreLinked_DoesNotGenerateWarningsIfAllOutputsLinked)
    {
        LuaInterface* intf = m_apiObjects.createLuaInterface(R"(
            function interface(IN,OUT)

                IN.param1 = Type:Int32()
                IN.param2 = {a=Type:Float(), b=Type:Int32()}

            end
        )", {}, "intf name", m_errorReporting, true);

        LuaScript* inputsScript = m_apiObjects.createLuaScript(R"LUA_SCRIPT(
        function interface(IN,OUT)

            IN.param1 = Type:Int32()
            IN.param21 = Type:Float()
            IN.param22 = Type:Int32()

        end

        function run(IN,OUT)
        end
        )LUA_SCRIPT", {}, "inputs script", m_errorReporting);

        const auto* output1 = intf->getOutputs()->getChild(0);
        const auto* output21 = intf->getOutputs()->getChild(1)->getChild(0);
        const auto* output22 = intf->getOutputs()->getChild(1)->getChild(1);

        m_apiObjects.getLogicNodeDependencies().link(*output1->m_impl, *inputsScript->getInputs()->getChild(0)->m_impl, false, m_errorReporting);
        m_apiObjects.getLogicNodeDependencies().link(*output21->m_impl, *inputsScript->getInputs()->getChild(1)->m_impl, false, m_errorReporting);
        m_apiObjects.getLogicNodeDependencies().link(*output22->m_impl, *inputsScript->getInputs()->getChild(2)->m_impl, false, m_errorReporting);

        ValidationResults validationResults;
        m_apiObjects.validateInterfaces(validationResults);
        EXPECT_TRUE(validationResults.getWarnings().empty());
    }

    TEST_P(AnApiObjects, ValidatesThatLuaInterfacesNamesAreUnique)
    {
        // single interface -> no warning
        const auto intf1 = createInterface();
        {
            ValidationResults validationResults;
            m_apiObjects.validateInterfaces(validationResults);
            EXPECT_TRUE(validationResults.getWarnings().empty());
        }

        // two interfaces with same name -> warning
        auto intf2 = createInterface();
        {
            ValidationResults validationResults;
            m_apiObjects.validateInterfaces(validationResults);
            ASSERT_EQ(1u, validationResults.getWarnings().size());
            EXPECT_EQ(validationResults.getWarnings().front().message, "Interface [intf] does not have a unique name");
            EXPECT_EQ(validationResults.getWarnings().front().object, intf1);
            EXPECT_EQ(validationResults.getWarnings().front().type, EWarningType::Other);
        }

        // rename conflicting intf -> no warning
        intf2->setName("otherIntf");
        {
            ValidationResults validationResults;
            m_apiObjects.validateInterfaces(validationResults);
            EXPECT_TRUE(validationResults.getWarnings().empty());
        }

        // another interface with same name -> warning
        auto intf3 = createInterface();
        intf3->setName("otherIntf");
        {
            ValidationResults validationResults;
            m_apiObjects.validateInterfaces(validationResults);
            ASSERT_EQ(1u, validationResults.getWarnings().size());
            EXPECT_EQ(validationResults.getWarnings().front().message, "Interface [otherIntf] does not have a unique name");
            EXPECT_EQ(validationResults.getWarnings().front().object, intf2);
            EXPECT_EQ(validationResults.getWarnings().front().type, EWarningType::Other);
        }
    }

    TEST_P(AnApiObjects, ValidatesDanglingNodes_ProducesWarningIfNodeHasNoIngoingOrOutgoingLinks)
    {
        auto* script = m_apiObjects.createLuaScript(R"(
            function interface(IN, OUT)
                IN.param1 = Type:Int32()
                OUT.param1 = Type:Int32()
            end
            function run(IN,OUT)
            end
        )", {}, "script name", m_errorReporting);
        ASSERT_NE(nullptr, script);

        ValidationResults validationResults;
        m_apiObjects.validateDanglingNodes(validationResults);
        EXPECT_EQ(2u, validationResults.getWarnings().size());
        EXPECT_THAT(validationResults.getWarnings()[0].message, ::testing::HasSubstr("Node [script name] has no outgoing links"));
        EXPECT_THAT(validationResults.getWarnings()[1].message, ::testing::HasSubstr("Node [script name] has no ingoing links"));
        EXPECT_THAT(validationResults.getWarnings(), ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(AnApiObjects, ValidatesDanglingNodes_DoesNotProduceWarningIfNodeHasNoInputs)
    {
        const auto script = m_apiObjects.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.param1 = Type:Int32()
            end
            function run(IN,OUT)
            end
            )", {}, "script name", m_errorReporting);
        ASSERT_NE(nullptr, script);

        const auto dummyInputScript = m_apiObjects.createLuaScript(R"LUA_SCRIPT(
            function interface(IN)
                IN.param1 = Type:Int32()
            end

            function run(IN,OUT)
            end
            )LUA_SCRIPT", {}, "dummy script", m_errorReporting);
        ASSERT_NE(nullptr, dummyInputScript);

        // link script's output in order to pass outputs validation
        m_apiObjects.getLogicNodeDependencies().link(*script->getOutputs()->getChild(0u)->m_impl, *dummyInputScript->getInputs()->getChild(0u)->m_impl, false, m_errorReporting);

        ValidationResults validationResults;
        m_apiObjects.validateDanglingNodes(validationResults);
        EXPECT_TRUE(validationResults.getWarnings().empty());
    }

    TEST_P(AnApiObjects, ValidatesDanglingNodes_DoesNotProduceWarningIfNodeHasNoOutputs)
    {
        const auto script = m_apiObjects.createLuaScript(R"(
            function interface(IN,OUT)
                IN.param1 = Type:Int32()
            end
            function run(IN,OUT)
            end
            )", {}, "script name", m_errorReporting);
        ASSERT_NE(nullptr, script);

        const auto dummyOutputScript = m_apiObjects.createLuaScript(R"LUA_SCRIPT(
            function interface(IN,OUT)
                OUT.param1 = Type:Int32()
            end

            function run(IN,OUT)
            end
            )LUA_SCRIPT", {}, "dummy script", m_errorReporting);
        ASSERT_NE(nullptr, dummyOutputScript);

        // link script's input in order to pass inputs validation
        m_apiObjects.getLogicNodeDependencies().link(*dummyOutputScript->getOutputs()->getChild(0u)->m_impl, *script->getInputs()->getChild(0u)->m_impl, false, m_errorReporting);

        ValidationResults validationResults;
        m_apiObjects.validateDanglingNodes(validationResults);
        EXPECT_TRUE(validationResults.getWarnings().empty());
    }

    class AnApiObjects_SceneMismatch : public AnApiObjects
    {
    protected:
        RamsesTestSetup m_testSetup;
        ramses::Scene* scene1 {m_testSetup.createScene(ramses::sceneId_t(1))};
        ramses::Scene* scene2 {m_testSetup.createScene(ramses::sceneId_t(2))};
    };

    INSTANTIATE_TEST_SUITE_P(
        AnApiObjects_SceneMismatchTests,
        AnApiObjects_SceneMismatch,
        rlogic::internal::GetFeatureLevelTestValues());

    TEST_P(AnApiObjects_SceneMismatch, recognizesNodeBindingsCarryNodesFromDifferentScenes)
    {
        m_apiObjects.createRamsesNodeBinding(*scene1->createNode("node1"), ERotationType::Euler_XYZ, "binding1");
        RamsesNodeBinding* binding2 = m_apiObjects.createRamsesNodeBinding(*scene2->createNode("node2"), ERotationType::Euler_XYZ, "binding2");

        EXPECT_FALSE(m_apiObjects.checkBindingsReferToSameRamsesScene(m_errorReporting));
        EXPECT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ("Ramses node 'node2' is from scene with id:2 but other objects are from scene with id:1!", m_errorReporting.getErrors()[0].message);
        EXPECT_EQ(binding2, m_errorReporting.getErrors()[0].object);
    }

    TEST_P(AnApiObjects_SceneMismatch, recognizesNodeBindingAndAppearanceBindingAreFromDifferentScenes)
    {
        m_apiObjects.createRamsesNodeBinding(*scene1->createNode("node"), ERotationType::Euler_XYZ, "node binding");
        RamsesAppearanceBinding* appBinding = m_apiObjects.createRamsesAppearanceBinding(RamsesTestSetup::CreateTrivialTestAppearance(*scene2), "app binding");

        EXPECT_FALSE(m_apiObjects.checkBindingsReferToSameRamsesScene(m_errorReporting));
        EXPECT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ("Ramses appearance 'test appearance' is from scene with id:2 but other objects are from scene with id:1!", m_errorReporting.getErrors()[0].message);
        EXPECT_EQ(appBinding, m_errorReporting.getErrors()[0].object);
    }

    TEST_P(AnApiObjects_SceneMismatch, detectsNodeBindingIsFromDifferentScene)
    {
        m_apiObjects.createRamsesNodeBinding(*scene1->createNode("node"), ERotationType::Euler_XYZ, "node binding");
        auto* otherSceneBinding = m_apiObjects.createRamsesNodeBinding(*scene2->createPerspectiveCamera("test camera"), ERotationType::Euler_XYZ, "other binding");

        EXPECT_FALSE(m_apiObjects.checkBindingsReferToSameRamsesScene(m_errorReporting));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ("Ramses node 'test camera' is from scene with id:2 but other objects are from scene with id:1!", m_errorReporting.getErrors()[0].message);
        EXPECT_EQ(otherSceneBinding, m_errorReporting.getErrors()[0].object);
    }

    TEST_P(AnApiObjects_SceneMismatch, detectsAppearanceBindingIsFromDifferentScene)
    {
        m_apiObjects.createRamsesNodeBinding(*scene1->createNode("node"), ERotationType::Euler_XYZ, "node binding");
        auto* otherSceneBinding = m_apiObjects.createRamsesAppearanceBinding(RamsesTestSetup::CreateTrivialTestAppearance(*scene2), "other binding");

        EXPECT_FALSE(m_apiObjects.checkBindingsReferToSameRamsesScene(m_errorReporting));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ("Ramses appearance 'test appearance' is from scene with id:2 but other objects are from scene with id:1!", m_errorReporting.getErrors()[0].message);
        EXPECT_EQ(otherSceneBinding, m_errorReporting.getErrors()[0].object);
    }

    TEST_P(AnApiObjects_SceneMismatch, detectsCameraBindingIsFromDifferentScene)
    {
        m_apiObjects.createRamsesNodeBinding(*scene1->createNode("node"), ERotationType::Euler_XYZ, "node binding");
        auto* otherSceneBinding = m_apiObjects.createRamsesCameraBinding(*scene2->createPerspectiveCamera("camera"), true, "other binding");

        EXPECT_FALSE(m_apiObjects.checkBindingsReferToSameRamsesScene(m_errorReporting));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ("Ramses camera 'camera' is from scene with id:2 but other objects are from scene with id:1!", m_errorReporting.getErrors()[0].message);
        EXPECT_EQ(otherSceneBinding, m_errorReporting.getErrors()[0].object);
    }

    TEST_P(AnApiObjects_SceneMismatch, detectsRenderPassBindingIsFromDifferentScene)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        m_apiObjects.createRamsesNodeBinding(*scene1->createNode("node"), ERotationType::Euler_XYZ, "node binding");
        auto* otherSceneBinding = m_apiObjects.createRamsesRenderPassBinding(*scene2->createRenderPass("render pass"), "other binding");

        EXPECT_FALSE(m_apiObjects.checkBindingsReferToSameRamsesScene(m_errorReporting));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ("Ramses render pass 'render pass' is from scene with id:2 but other objects are from scene with id:1!", m_errorReporting.getErrors()[0].message);
        EXPECT_EQ(otherSceneBinding, m_errorReporting.getErrors()[0].object);
    }

    TEST_P(AnApiObjects_SceneMismatch, detectsRenderGroupBindingIsFromDifferentScene)
    {
        if (GetParam() < EFeatureLevel_03)
            GTEST_SKIP();

        m_apiObjects.createRamsesNodeBinding(*scene1->createNode("node"), ERotationType::Euler_XYZ, "node binding");

        auto rg = scene2->createRenderGroup("render group");
        const auto mesh = scene2->createMeshNode("mesh");
        EXPECT_EQ(ramses::StatusOK, rg->addMeshNode(*mesh));
        RamsesRenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(*mesh));
        const auto* otherSceneBinding = m_apiObjects.createRamsesRenderGroupBinding(*rg, elements, "other binding");

        EXPECT_FALSE(m_apiObjects.checkBindingsReferToSameRamsesScene(m_errorReporting));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ("Ramses render group 'render group' is from scene with id:2 but other objects are from scene with id:1!", m_errorReporting.getErrors()[0].message);
        EXPECT_EQ(otherSceneBinding, m_errorReporting.getErrors()[0].object);
    }

    TEST_P(AnApiObjects_SceneMismatch, detectsMeshNodeBindingIsFromDifferentScene)
    {
        if (GetParam() < EFeatureLevel_05)
            GTEST_SKIP();

        m_apiObjects.createRamsesNodeBinding(*scene1->createNode("node"), ERotationType::Euler_XYZ, "node binding");

        const auto otherSceneBinding = m_apiObjects.createRamsesMeshNodeBinding(*scene2->createMeshNode("mesh"), "mb");

        EXPECT_FALSE(m_apiObjects.checkBindingsReferToSameRamsesScene(m_errorReporting));
        ASSERT_EQ(1u, m_errorReporting.getErrors().size());
        EXPECT_EQ("Ramses mesh node 'mesh' is from scene with id:2 but other objects are from scene with id:1!", m_errorReporting.getErrors()[0].message);
        EXPECT_EQ(otherSceneBinding, m_errorReporting.getErrors()[0].object);
    }

    class AnApiObjects_ImplMapping : public AnApiObjects
    {
    };

    INSTANTIATE_TEST_SUITE_P(
        AnApiObjects_ImplMappingTests,
        AnApiObjects_ImplMapping,
        rlogic::internal::GetFeatureLevelTestValues());

    TEST_P(AnApiObjects_ImplMapping, EmptyWhenCreated)
    {
        EXPECT_TRUE(m_apiObjects.getReverseImplMapping().empty());
    }

    TEST_P(AnApiObjects_ImplMapping, DestroyingScriptDoesNotAffectOtherScript)
    {
        auto script1 = createScript();
        auto script2 = createScript();

        ASSERT_TRUE(m_apiObjects.destroy(*script1, m_errorReporting));

        ASSERT_EQ(1u, m_apiObjects.getReverseImplMapping().size());
        EXPECT_EQ(script2, m_apiObjects.getApiObject(script2->m_impl));
    }

    TEST_P(AnApiObjects_ImplMapping, DestroyingBindingDoesNotAffectScript)
    {
        const LuaScript* script = createScript();
        RamsesNodeBinding* binding = m_apiObjects.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");

        ASSERT_TRUE(m_apiObjects.destroy(*binding, m_errorReporting));

        ASSERT_EQ(1u, m_apiObjects.getReverseImplMapping().size());
        EXPECT_EQ(script, m_apiObjects.getApiObject(script->m_impl));
    }

    class AnApiObjects_Serialization : public AnApiObjects
    {
    };

    INSTANTIATE_TEST_SUITE_P(
        AnApiObjects_SerializationTests,
        AnApiObjects_Serialization,
        rlogic::internal::GetFeatureLevelTestValues());

    TEST_P(AnApiObjects_Serialization, AlwaysCreatesEmptyFlatbuffersContainers_WhenNoObjectsPresent)
    {
        // Create without API objects -> serialize
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize(GetParam());
            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        // Has all containers, size = 0 because no content
        ASSERT_NE(nullptr, serialized.luaScripts());
        ASSERT_EQ(0u, serialized.luaScripts()->size());

        ASSERT_NE(nullptr, serialized.luaInterfaces());
        ASSERT_EQ(0u, serialized.luaInterfaces()->size());

        ASSERT_NE(nullptr, serialized.nodeBindings());
        ASSERT_EQ(0u, serialized.nodeBindings()->size());

        ASSERT_NE(nullptr, serialized.appearanceBindings());
        ASSERT_EQ(0u, serialized.appearanceBindings()->size());

        ASSERT_NE(nullptr, serialized.cameraBindings());
        ASSERT_EQ(0u, serialized.cameraBindings()->size());

        ASSERT_NE(nullptr, serialized.renderPassBindings());
        ASSERT_EQ(0u, serialized.renderPassBindings()->size());

        ASSERT_NE(nullptr, serialized.links());
        ASSERT_EQ(0u, serialized.links()->size());

        EXPECT_EQ(0u, serialized.lastObjectId());
    }

    TEST_P(AnApiObjects_Serialization, CreatesFlatbufferContainer_ForScripts)
    {
        // Create test flatbuffer with only a script
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize(GetParam());
            createScript(toSerialize, m_valid_empty_script);
            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::SourceAndByteCode);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        ASSERT_NE(nullptr, serialized.luaScripts());
        ASSERT_EQ(1u, serialized.luaScripts()->size());
        const rlogic_serialization::LuaScript& serializedScript = *serialized.luaScripts()->Get(0);
        EXPECT_EQ("script", serializedScript.base()->name()->str());
        EXPECT_EQ(1u, serializedScript.base()->id());
        EXPECT_EQ(m_valid_empty_script, serializedScript.luaSourceCode()->str());
        if (GetParam() == EFeatureLevel_01)
        {
            EXPECT_FALSE(serializedScript.luaByteCode());
        }
        else
        {
            EXPECT_TRUE(serializedScript.luaByteCode()->size() > 0);
        }

        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "test", m_errorReporting, GetParam());
        EXPECT_TRUE(deserialized);
    }

    TEST_P(AnApiObjects_Serialization, CreatesFlatbufferContainer_ForInterfaces)
    {
        // Create test flatbuffer with only a script
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize(GetParam());
            createInterface(toSerialize);
            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        ASSERT_NE(nullptr, serialized.luaInterfaces());
        ASSERT_EQ(1u, serialized.luaInterfaces()->size());
        const rlogic_serialization::LuaInterface& serializedInterface = *serialized.luaInterfaces()->Get(0);
        EXPECT_EQ("intf", serializedInterface.base()->name()->str());
        EXPECT_EQ(1u, serializedInterface.base()->id());

        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "test", m_errorReporting, GetParam());
        EXPECT_TRUE(deserialized);
    }

    TEST_P(AnApiObjects_Serialization, CreatesFlatbufferContainers_ForBindings)
    {
        // Create test flatbuffer with only a node binding
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize(GetParam());
            toSerialize.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
            toSerialize.createRamsesAppearanceBinding(*m_appearance, "appearance");
            toSerialize.createRamsesCameraBinding(*m_camera, true, "camera");
            if (GetParam() >= EFeatureLevel_02)
                toSerialize.createRamsesRenderPassBinding(*m_renderPass, "rp");
            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        ASSERT_NE(nullptr, serialized.nodeBindings());
        ASSERT_EQ(1u, serialized.nodeBindings()->size());
        const rlogic_serialization::RamsesNodeBinding& serializedNodeBinding = *serialized.nodeBindings()->Get(0);
        EXPECT_EQ("node", serializedNodeBinding.base()->base()->name()->str());
        EXPECT_EQ(1u, serializedNodeBinding.base()->base()->id());

        ASSERT_NE(nullptr, serialized.appearanceBindings());
        ASSERT_EQ(1u, serialized.appearanceBindings()->size());
        const rlogic_serialization::RamsesAppearanceBinding& serializedAppBinding = *serialized.appearanceBindings()->Get(0);
        EXPECT_EQ("appearance", serializedAppBinding.base()->base()->name()->str());
        EXPECT_EQ(2u, serializedAppBinding.base()->base()->id());

        ASSERT_NE(nullptr, serialized.cameraBindings());
        ASSERT_EQ(1u, serialized.cameraBindings()->size());
        const rlogic_serialization::RamsesCameraBinding& serializedCameraBinding = *serialized.cameraBindings()->Get(0);
        EXPECT_EQ("camera", serializedCameraBinding.base()->base()->name()->str());
        EXPECT_EQ(3u, serializedCameraBinding.base()->base()->id());

        if (GetParam() >= EFeatureLevel_02)
        {
            ASSERT_NE(nullptr, serialized.renderPassBindings());
            ASSERT_EQ(1u, serialized.renderPassBindings()->size());
            const rlogic_serialization::RamsesRenderPassBinding& serializedRenderPassBinding = *serialized.renderPassBindings()->Get(0);
            EXPECT_EQ("rp", serializedRenderPassBinding.base()->base()->name()->str());
            EXPECT_EQ(4u, serializedRenderPassBinding.base()->base()->id());
        }
    }

    TEST_P(AnApiObjects_Serialization, CreatesFlatbufferContainers_ForLinks)
    {
        // Create test flatbuffer with a link between script and binding
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize(GetParam());

            const std::string_view scriptWithOutput = R"(
                function interface(IN,OUT)
                    OUT.nested = {
                        anUnusedValue = Type:Float(),
                        rotation = Type:Vec3f()
                    }
                end
                function run(IN,OUT)
                end
            )";

            const LuaScript* script = createScript(toSerialize, scriptWithOutput);
            RamsesNodeBinding* nodeBinding = toSerialize.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
            ASSERT_TRUE(toSerialize.getLogicNodeDependencies().link(
                *script->getOutputs()->getChild("nested")->getChild("rotation")->m_impl,
                *nodeBinding->getInputs()->getChild("rotation")->m_impl,
                false,
                m_errorReporting));
            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        // Asserts both script and binding objects existence
        ASSERT_EQ(1u, serialized.luaScripts()->size());
        ASSERT_EQ(1u, serialized.nodeBindings()->size());
        const rlogic_serialization::LuaScript& script = *serialized.luaScripts()->Get(0);
        const rlogic_serialization::RamsesNodeBinding& binding = *serialized.nodeBindings()->Get(0);

        ASSERT_NE(nullptr, serialized.links());
        ASSERT_EQ(1u, serialized.links()->size());
        const rlogic_serialization::Link& link = *serialized.links()->Get(0);

        EXPECT_EQ(script.rootOutput()->children()->Get(0)->children()->Get(1), link.sourceProperty());
        EXPECT_EQ(binding.base()->rootInput()->children()->Get(size_t(ENodePropertyStaticIndex::Rotation)), link.targetProperty());
    }

    TEST_P(AnApiObjects_Serialization, ReConstructsImplMappingsWhenCreatedFromDeserializedData)
    {
        // Create dummy data and serialize
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize(GetParam());
            createScript(toSerialize, m_valid_empty_script);
            createInterface(toSerialize);
            const auto nodeBinding = toSerialize.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
            const auto appearanceBinding = toSerialize.createRamsesAppearanceBinding(*m_appearance, "appearance");
            toSerialize.createRamsesCameraBinding(*m_camera, true, "camera");
            if (GetParam() >= EFeatureLevel_02)
                toSerialize.createRamsesRenderPassBinding(*m_renderPass, "rp");
            if (GetParam() >= EFeatureLevel_03)
            {
                RamsesRenderGroupBindingElements elements;
                EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
                toSerialize.createRamsesRenderGroupBinding(*m_renderGroup, elements, "rg");
            }
            if (GetParam() >= EFeatureLevel_04)
                createSkinBinding(*nodeBinding, *appearanceBinding, toSerialize);
            if (GetParam() >= EFeatureLevel_05)
                toSerialize.createRamsesMeshNodeBinding(*m_meshNode, "mb");

            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        EXPECT_CALL(m_resolverMock, findRamsesNodeInScene(::testing::Eq("node"), m_node->getSceneObjectId())).WillOnce(::testing::Return(m_node));
        EXPECT_CALL(m_resolverMock, findRamsesAppearanceInScene(::testing::Eq("appearance"), m_appearance->getSceneObjectId())).WillOnce(::testing::Return(m_appearance));
        EXPECT_CALL(m_resolverMock, findRamsesCameraInScene(::testing::Eq("camera"), m_camera->getSceneObjectId())).WillOnce(::testing::Return(m_camera));
        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_CALL(m_resolverMock, findRamsesRenderPassInScene(::testing::Eq("rp"), m_renderPass->getSceneObjectId())).WillOnce(::testing::Return(m_renderPass));
        }
        if (GetParam() >= EFeatureLevel_03)
        {
            EXPECT_CALL(m_resolverMock, findRamsesRenderGroupInScene(::testing::Eq("rg"), m_renderGroup->getSceneObjectId())).WillOnce(::testing::Return(m_renderGroup));
            EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("rg"), m_meshNode->getSceneObjectId())).WillOnce(::testing::Return(m_meshNode));
        }
        if (GetParam() >= EFeatureLevel_05)
        {
            EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("mb"), m_meshNode->getSceneObjectId())).WillOnce(::testing::Return(m_meshNode));
        }
        std::unique_ptr<ApiObjects> apiObjectsOptional = ApiObjects::Deserialize(serialized, &m_resolverMock, "", m_errorReporting, GetParam());

        ASSERT_TRUE(apiObjectsOptional);

        ApiObjects& apiObjects = *apiObjectsOptional;

        size_t expectedObjCount = 0u;
        switch (GetParam())
        {
        default:
        case EFeatureLevel_01:
            expectedObjCount = 5u;
            break;
        case EFeatureLevel_02:
            expectedObjCount = 6u;
            break;
        case EFeatureLevel_03:
            expectedObjCount = 7u;
            break;
        case EFeatureLevel_04:
            expectedObjCount = 8u;
            break;
        case EFeatureLevel_05:
            expectedObjCount = 9u;
            break;
        }
        ASSERT_EQ(expectedObjCount, apiObjects.getReverseImplMapping().size());

        LuaScript* script = apiObjects.getApiObjectContainer<LuaScript>()[0];
        EXPECT_EQ(script, apiObjects.getApiObject(script->m_impl));
        EXPECT_EQ(script->getName(), "script");
        EXPECT_EQ(script, &script->m_script.getLogicObject());

        LuaInterface* intf = apiObjects.getApiObjectContainer<LuaInterface>()[0];
        EXPECT_EQ(intf, apiObjects.getApiObject(intf->m_impl));
        EXPECT_EQ(intf->getName(), "intf");
        EXPECT_EQ(intf, &intf->m_interface.getLogicObject());

        RamsesNodeBinding* nodeBinding = apiObjects.getApiObjectContainer<RamsesNodeBinding>()[0];
        EXPECT_EQ(nodeBinding, apiObjects.getApiObject(nodeBinding->m_impl));
        EXPECT_EQ(nodeBinding->getName(), "node");
        EXPECT_EQ(nodeBinding, &nodeBinding->m_nodeBinding.getLogicObject());

        RamsesAppearanceBinding* appBinding = apiObjects.getApiObjectContainer<RamsesAppearanceBinding>()[0];
        EXPECT_EQ(appBinding, apiObjects.getApiObject(appBinding->m_impl));
        EXPECT_EQ(appBinding->getName(), "appearance");
        EXPECT_EQ(appBinding, &appBinding->m_appearanceBinding.getLogicObject());

        RamsesCameraBinding* camBinding = apiObjects.getApiObjectContainer<RamsesCameraBinding>()[0];
        EXPECT_EQ(camBinding, apiObjects.getApiObject(camBinding->m_impl));
        EXPECT_EQ(camBinding->getName(), "camera");
        EXPECT_EQ(camBinding, &camBinding->m_cameraBinding.getLogicObject());

        if (GetParam() >= EFeatureLevel_02)
        {
            RamsesRenderPassBinding* rpBinding = apiObjects.getApiObjectContainer<RamsesRenderPassBinding>()[0];
            EXPECT_EQ(rpBinding, apiObjects.getApiObject(rpBinding->m_impl));
            EXPECT_EQ(rpBinding->getName(), "rp");
            EXPECT_EQ(rpBinding, &rpBinding->m_renderPassBinding.getLogicObject());
        }

        if (GetParam() >= EFeatureLevel_03)
        {
            const auto rgBinding = apiObjects.getApiObjectContainer<RamsesRenderGroupBinding>()[0];
            EXPECT_EQ(rgBinding, apiObjects.getApiObject(rgBinding->m_impl));
            EXPECT_EQ(rgBinding->getName(), "rg");
            EXPECT_EQ(rgBinding, &rgBinding->m_renderGroupBinding.getLogicObject());
        }

        if (GetParam() >= EFeatureLevel_04)
        {
            const auto skin = apiObjects.getApiObjectContainer<SkinBinding>()[0];
            EXPECT_EQ(skin, apiObjects.getApiObject(skin->m_impl));
            EXPECT_EQ(skin->getName(), "skin");
            EXPECT_EQ(skin, &skin->m_skinBinding.getLogicObject());
        }

        if (GetParam() >= EFeatureLevel_05)
        {
            const auto meshBinding = apiObjects.getApiObjectContainer<RamsesMeshNodeBinding>()[0];
            EXPECT_EQ(meshBinding, apiObjects.getApiObject(meshBinding->m_impl));
            EXPECT_EQ(meshBinding->getName(), "mb");
            EXPECT_EQ(meshBinding, &meshBinding->m_meshNodeBinding.getLogicObject());
        }
    }

    TEST_P(AnApiObjects_Serialization, ObjectsCreatedAfterLoadingReceiveUniqueId)
    {
        ApiObjects beforeSaving(GetParam());

        // Create dummy data and serialize
        flatbuffers::FlatBufferBuilder builder;
        {
            createScript(beforeSaving, m_valid_empty_script);
            createScript(beforeSaving, m_valid_empty_script);
            createScript(beforeSaving, m_valid_empty_script);

            ApiObjects::Serialize(beforeSaving, builder, ELuaSavingMode::ByteCodeOnly);
        }

        auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        EXPECT_EQ(3u, serialized.lastObjectId());

        std::unique_ptr<ApiObjects> afterLoadingObjects = ApiObjects::Deserialize(serialized, &m_resolverMock, "", m_errorReporting, GetParam());

        auto* newScript = createScript(*afterLoadingObjects, m_valid_empty_script);
        // new script's ID does not overlap with one of the IDs of the objects before saving
        EXPECT_EQ(nullptr, beforeSaving.getApiObjectById(newScript->getId()));
    }

    TEST_P(AnApiObjects_Serialization, ReConstructsLinksWhenCreatedFromDeserializedData)
    {
        // Create dummy data and serialize
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize(GetParam());

            const std::string_view scriptForLinks = R"(
                function interface(IN,OUT)
                    IN.integer = Type:Int32()
                    OUT.nested = {
                        unused = Type:Float(),
                        integer = Type:Int32()
                    }
                end
                function run(IN,OUT)
                end
            )";

            auto script1 = createScript(toSerialize, scriptForLinks);
            auto script2 = createScript(toSerialize, scriptForLinks);
            ASSERT_TRUE(toSerialize.getLogicNodeDependencies().link(
                *script1->getOutputs()->getChild("nested")->getChild("integer")->m_impl,
                *script2->getInputs()->getChild("integer")->m_impl,
                false,
                m_errorReporting));

            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        std::unique_ptr<ApiObjects> apiObjectsOptional = ApiObjects::Deserialize(serialized, &m_resolverMock, "", m_errorReporting, GetParam());

        ASSERT_TRUE(apiObjectsOptional);

        ApiObjects& apiObjects = *apiObjectsOptional;

        LuaScript* script1 = apiObjects.getApiObjectContainer<LuaScript>()[0];
        ASSERT_TRUE(script1);

        LuaScript* script2 = apiObjects.getApiObjectContainer<LuaScript>()[1];
        ASSERT_TRUE(script2);

        EXPECT_TRUE(apiObjects.getLogicNodeDependencies().isLinked(script1->m_impl));
        EXPECT_TRUE(apiObjects.getLogicNodeDependencies().isLinked(script2->m_impl));

        const PropertyImpl* script1Output = script1->getOutputs()->getChild("nested")->getChild("integer")->m_impl.get();
        const PropertyImpl* script2Input = script2->getInputs()->getChild("integer")->m_impl.get();
        EXPECT_EQ(script1Output, script2Input->getIncomingLink().property);
        EXPECT_FALSE(script2Input->getIncomingLink().isWeakLink);

        ASSERT_EQ(1u, script1Output->getOutgoingLinks().size());
        EXPECT_EQ(script1Output->getOutgoingLinks()[0].property, script2Input);
        EXPECT_FALSE(script1Output->getOutgoingLinks()[0].isWeakLink);
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenLuaModulesContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                0, // no modules container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
                );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing Lua modules container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenScriptsContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                0, // no scripts container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing Lua scripts container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenInterfacesContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                0, // no interfaces container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing Lua interfaces container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenNodeBindingsContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                0, // no node bindings container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing node bindings container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenAppearanceBindingsContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                0, // no appearance bindings container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing appearance bindings container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenCameraBindingsContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                0, // no camera bindings container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing camera bindings container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenRenderPassBindingsContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                0u, // no render pass bindings container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_FALSE(deserialized);
            ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
            EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing renderpass bindings container!");
        }
        else
        {
            EXPECT_TRUE(deserialized);
        }
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenLinksContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                0, // no links container
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing links container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenDataArrayContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                0, // no data array container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing data arrays container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenAnimationNodeContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                0, // no animation nodes container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing animation nodes container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenTimerNodeContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                0, // no timer nodes container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing timer nodes container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenAnchorPointContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                0u, // no anchor points container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_FALSE(deserialized);
            ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
            EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing anchor points container!");
        }
        else
        {
            EXPECT_TRUE(deserialized);
        }
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenRenderGroupBindingContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                0u, // no render group bindings container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        if (GetParam() >= EFeatureLevel_03)
        {
            EXPECT_FALSE(deserialized);
            ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
            EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing rendergroup bindings container!");
        }
        else
        {
            EXPECT_TRUE(deserialized);
        }
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenSkinBindingContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                0u, // no skin bindings container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        if (GetParam() >= EFeatureLevel_04)
        {
            EXPECT_FALSE(deserialized);
            ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
            EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing skin bindings container!");
        }
        else
        {
            EXPECT_TRUE(deserialized);
        }
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenMeshNodeBindingContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                0u // no mesh node bindings container
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        if (GetParam() >= EFeatureLevel_05)
        {
            EXPECT_FALSE(deserialized);
            ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
            EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading from serialized data: missing meshnode bindings container!");
        }
        else
        {
            EXPECT_TRUE(deserialized);
        }
    }

    TEST_P(AnApiObjects_Serialization, ReportsErrorWhenScriptCouldNotBeDeserialized)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{ m_testUtils.serializeTestScriptWithError() }),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 2u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LogicObject base from serialized data: missing base table!");
        EXPECT_EQ(m_errorReporting.getErrors()[1].message, "Fatal error during loading of LuaScript from serialized data: missing name and/or ID!");
    }

    TEST_P(AnApiObjects_Serialization, ReportsErrorWhenInterfaceCouldNotBeDeserialized)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{m_testUtils.serializeTestInterfaceWithError()}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaInterface from serialized data: empty name!");
    }

    TEST_P(AnApiObjects_Serialization, ReportsErrorWhenModuleCouldNotBeDeserialized)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{ m_testUtils.serializeTestModule(true) }), // module has errors
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 2u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LogicObject base from serialized data: missing name!");
        EXPECT_EQ(m_errorReporting.getErrors()[1].message, "Fatal error during loading of LuaModule from serialized data: missing name and/or ID!");
    }

    TEST_P(AnApiObjects_Serialization, ReportsErrorWhenRenderPassBindingCouldNotBeDeserialized)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{ m_testUtils.serializeTestRenderPassBindingWithError() }),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_FALSE(deserialized);
            ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
            EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderPassBinding from serialized data: missing base class info!");
        }
        else
        {
            EXPECT_TRUE(deserialized);
        }
    }

    TEST_P(AnApiObjects_Serialization, ReportsErrorWhenRenderGroupBindingCouldNotBeDeserialized)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>>{ m_testUtils.serializeTestRenderGroupBindingWithError() }),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "unit test", m_errorReporting, GetParam());

        if (GetParam() >= EFeatureLevel_03)
        {
            EXPECT_FALSE(deserialized);
            ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
            EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of RamsesRenderGroupBinding from serialized data: missing base class info!");
        }
        else
        {
            EXPECT_TRUE(deserialized);
        }
    }

    TEST_P(AnApiObjects_Serialization, FillsLogicObjectAndOwnedContainerOnDeserialization)
    {
        // Create dummy data and serialize
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize{ GetParam() };
            toSerialize.createLuaModule(m_moduleSrc, {}, "module", m_errorReporting);
            createScript(toSerialize, m_valid_empty_script);
            createInterface(toSerialize);
            const auto node = toSerialize.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
            const auto appearance = toSerialize.createRamsesAppearanceBinding(*m_appearance, "appearance");
            const auto camera = toSerialize.createRamsesCameraBinding(*m_camera, true, "camera");
            auto dataArray = toSerialize.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "data");
            AnimationNodeConfig config;
            config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
            toSerialize.createAnimationNode(*config.m_impl, "animNode");
            toSerialize.createTimerNode("timerNode");
            if (GetParam() >= EFeatureLevel_02)
            {
                toSerialize.createRamsesRenderPassBinding(*m_renderPass, "rp");
                toSerialize.createAnchorPoint(node->m_nodeBinding, camera->m_cameraBinding, "anchor");
            }
            if (GetParam() >= EFeatureLevel_03)
            {
                RamsesRenderGroupBindingElements elements;
                EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
                toSerialize.createRamsesRenderGroupBinding(*m_renderGroup, elements, "rg");
            }
            if (GetParam() >= EFeatureLevel_04)
                createSkinBinding(*node, *appearance, toSerialize);
            if (GetParam() >= EFeatureLevel_05)
                toSerialize.createRamsesMeshNodeBinding(*m_meshNode, "mb");

            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        EXPECT_CALL(m_resolverMock, findRamsesNodeInScene(::testing::Eq("node"), m_node->getSceneObjectId())).WillOnce(::testing::Return(m_node));
        EXPECT_CALL(m_resolverMock, findRamsesAppearanceInScene(::testing::Eq("appearance"), m_appearance->getSceneObjectId())).WillOnce(::testing::Return(m_appearance));
        EXPECT_CALL(m_resolverMock, findRamsesCameraInScene(::testing::Eq("camera"), m_camera->getSceneObjectId())).WillOnce(::testing::Return(m_camera));
        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_CALL(m_resolverMock, findRamsesRenderPassInScene(::testing::Eq("rp"), m_renderPass->getSceneObjectId())).WillOnce(::testing::Return(m_renderPass));
        }
        if (GetParam() >= EFeatureLevel_03)
        {
            EXPECT_CALL(m_resolverMock, findRamsesRenderGroupInScene(::testing::Eq("rg"), m_renderGroup->getSceneObjectId())).WillOnce(::testing::Return(m_renderGroup));
            EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("rg"), m_meshNode->getSceneObjectId())).WillOnce(::testing::Return(m_meshNode));
        }
        if (GetParam() >= EFeatureLevel_05)
        {
            EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("mb"), m_meshNode->getSceneObjectId())).WillOnce(::testing::Return(m_meshNode));
        }
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(serialized, &m_resolverMock, "", m_errorReporting, GetParam());

        ASSERT_TRUE(deserialized);

        ApiObjects& apiObjects = *deserialized;

        const ApiObjectContainer<LogicObject>& logicObjects = apiObjects.getApiObjectContainer<LogicObject>();
        const ApiObjectOwningContainer& ownedObjects = apiObjects.getApiObjectOwningContainer();

        std::vector<LogicObject*> expected;
        if (GetParam() < EFeatureLevel_02)
        {
            expected = std::vector<LogicObject*>{
                apiObjects.getApiObjectContainer<LuaModule>()[0],
                apiObjects.getApiObjectContainer<LuaScript>()[0],
                apiObjects.getApiObjectContainer<LuaInterface>()[0],
                apiObjects.getApiObjectContainer<RamsesNodeBinding>()[0],
                apiObjects.getApiObjectContainer<RamsesAppearanceBinding>()[0],
                apiObjects.getApiObjectContainer<RamsesCameraBinding>()[0],
                apiObjects.getApiObjectContainer<DataArray>()[0],
                apiObjects.getApiObjectContainer<AnimationNode>()[0],
                apiObjects.getApiObjectContainer<TimerNode>()[0]
            };
        }
        else
        {
            expected = std::vector<LogicObject*>{
                apiObjects.getApiObjectContainer<LuaModule>()[0],
                apiObjects.getApiObjectContainer<LuaScript>()[0],
                apiObjects.getApiObjectContainer<LuaInterface>()[0],
                apiObjects.getApiObjectContainer<RamsesNodeBinding>()[0],
                apiObjects.getApiObjectContainer<RamsesAppearanceBinding>()[0],
                apiObjects.getApiObjectContainer<RamsesCameraBinding>()[0],
                apiObjects.getApiObjectContainer<RamsesRenderPassBinding>()[0],
                apiObjects.getApiObjectContainer<DataArray>()[0],
                apiObjects.getApiObjectContainer<AnimationNode>()[0],
                apiObjects.getApiObjectContainer<TimerNode>()[0],
                apiObjects.getApiObjectContainer<AnchorPoint>()[0]
            };

            if (GetParam() >= EFeatureLevel_03)
                expected.push_back(apiObjects.getApiObjectContainer<RamsesRenderGroupBinding>()[0]);

            if (GetParam() >= EFeatureLevel_04)
                expected.push_back(apiObjects.getApiObjectContainer<SkinBinding>()[0]);
            if (GetParam() >= EFeatureLevel_05)
                expected.push_back(apiObjects.getApiObjectContainer<RamsesMeshNodeBinding>()[0]);
        }

        ASSERT_EQ(expected.size(), logicObjects.size());
        ASSERT_EQ(logicObjects.size(), ownedObjects.size());
        for (size_t i = 0; i < expected.size(); ++i)
        {
            EXPECT_EQ(logicObjects[i], expected[i]);
            EXPECT_EQ(ownedObjects[i].get(), expected[i]);
            EXPECT_EQ(logicObjects[i], &logicObjects[i]->m_impl->getLogicObject());
        }
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithoutContent)
    {
        ApiObjects toSerialize{ GetParam() };
        EXPECT_EQ(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
        EXPECT_EQ(toSerialize.getSerializedSize<LogicObject>(), m_emptySerializedSizeTotal);

        EXPECT_EQ(toSerialize.getSerializedSize<AnchorPoint>(), 0u);
        EXPECT_EQ(toSerialize.getSerializedSize<AnimationNode>(), 0u);
        EXPECT_EQ(toSerialize.getSerializedSize<LuaInterface>(), 0u);
        EXPECT_EQ(toSerialize.getSerializedSize<LuaModule>(), 0u);
        EXPECT_EQ(toSerialize.getSerializedSize<LuaScript>(), 0u);
        EXPECT_EQ(toSerialize.getSerializedSize<RamsesAppearanceBinding>(), 0u);
        EXPECT_EQ(toSerialize.getSerializedSize<RamsesCameraBinding>(), 0u);
        EXPECT_EQ(toSerialize.getSerializedSize<RamsesNodeBinding>(), 0u);
        EXPECT_EQ(toSerialize.getSerializedSize<RamsesRenderPassBinding>(), 0u);
        EXPECT_EQ(toSerialize.getSerializedSize<RamsesRenderGroupBinding>(), 0u);
        EXPECT_EQ(toSerialize.getSerializedSize<RamsesMeshNodeBinding>(), 0u);
        EXPECT_EQ(toSerialize.getSerializedSize<TimerNode>(), 0u);
        EXPECT_EQ(toSerialize.getSerializedSize<SkinBinding>(), 0u);
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithInterface)
    {
        ApiObjects toSerialize{ GetParam() };
        createInterface(toSerialize);
        EXPECT_EQ(toSerialize.getSerializedSize<LuaInterface>(), 112u);
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }

    TEST_P(AnApiObjects_Serialization, DISABLED_ChecksSerializedSizeWithModule)
    {
        ApiObjects toSerialize{ GetParam() };
        toSerialize.createLuaModule(R"(
            local mymath = {}
            function mymath.add(a,b)
                return a+b
            end
            mymath.PI=3.1415
            return mymath
        )", {}, "module", m_errorReporting);

        auto result = toSerialize.getSerializedSize<LuaModule>();
        if (EFeatureLevel::EFeatureLevel_01 == GetParam())
        {
            EXPECT_EQ(result, 260u);
        }
        else
        {
            EXPECT_EQ(result, 366u);
        }
    }

    TEST_P(AnApiObjects_Serialization, DISABLED_ChecksSerializedSizeWithEmptyScript)
    {
        ApiObjects toSerialize{ GetParam() };
        createScript(toSerialize, m_valid_empty_script);
        auto result = toSerialize.getSerializedSize<LuaScript>();
        if (EFeatureLevel::EFeatureLevel_01 == GetParam())
        {
            EXPECT_EQ(result, 312u);
        }
        else
        {
            EXPECT_EQ(result, 528u);
        }
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }

    TEST_P(AnApiObjects_Serialization, DISABLED_ChecksSerializedSizeWithScript)
    {
        ApiObjects toSerialize{ GetParam() };
        (void)toSerialize.createLuaScript(R"(
            function interface(IN,OUT)
                IN.a = Type:Float()
                IN.b = Type:Float()
                OUT.value = Type:Float()
            end

            function run(IN,OUT)
                OUT.value = IN.a + IN.b
            end
        )", {}, "script", m_errorReporting);
        auto result = toSerialize.getSerializedSize<LuaScript>();
        if (EFeatureLevel::EFeatureLevel_01 == GetParam())
        {
            EXPECT_EQ(result, 616u);
        }
        else
        {
            EXPECT_EQ(result, 912u);
        }
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }

    TEST_P(AnApiObjects_Serialization, DISABLED_ChecksSerializedSizeWithScriptAndModuleDependency)
    {
        LuaConfigImpl config{};
        ApiObjects toSerialize{ GetParam() };
        const auto module = toSerialize.createLuaModule(R"(
            local mymath = {}
            function mymath.add(a,b)
                return a+b
            end
            mymath.PI=3.1415
            return mymath
        )", config, "module", m_errorReporting);
        config.addDependency("mymath", *module);
        (void)toSerialize.createLuaScript(R"(
            modules("mymath")

            function interface(IN,OUT)
                OUT.v = Type:Int32()
                OUT.pi = Type:Float()
            end

            function run(IN,OUT)
                OUT.v = mymath.add(1,2)
                OUT.pi = mymath.PI
            end
        )", config, "script", m_errorReporting);

        auto result = toSerialize.getSerializedSize<LuaScript>();
        if (EFeatureLevel::EFeatureLevel_01 == GetParam())
        {
            EXPECT_EQ(result, 624u);
        }
        else
        {
            EXPECT_EQ(result, 1000u);
        }
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithNodeBinding)
    {
        ApiObjects toSerialize{ GetParam() };
        toSerialize.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
        auto result = toSerialize.getSerializedSize<RamsesNodeBinding>();
        if (EFeatureLevel::EFeatureLevel_01 == GetParam())
        {
            EXPECT_EQ(result, 400u);
        }
        else
        {
            EXPECT_EQ(result, 440u);
        }
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithAppearanceBinding)
    {
        ApiObjects toSerialize{ GetParam() };
        toSerialize.createRamsesAppearanceBinding(*m_appearance, "appearance");
        EXPECT_EQ(toSerialize.getSerializedSize<RamsesAppearanceBinding>(), 256u);
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithCameraBinding)
    {
        ApiObjects toSerialize{ GetParam() };
        toSerialize.createRamsesCameraBinding(*m_camera, true, "camera");
        EXPECT_EQ(toSerialize.getSerializedSize<RamsesCameraBinding>(), 728u);
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithRenderPassBinding)
    {
        if (GetParam() < EFeatureLevel_02)
        {
            GTEST_SKIP();
        }
        ApiObjects toSerialize{ GetParam() };
        toSerialize.createRamsesRenderPassBinding(*m_renderPass, "renderpass");
        EXPECT_EQ(toSerialize.getSerializedSize<RamsesRenderPassBinding>(), 376u);
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithRenderGroupBinding)
    {
        if (GetParam() < EFeatureLevel_03)
        {
            GTEST_SKIP();
        }
        ApiObjects toSerialize{ GetParam() };
        RamsesRenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(*m_meshNode));
        toSerialize.createRamsesRenderGroupBinding(*m_renderGroup, elements, "rg");
        EXPECT_EQ(toSerialize.getSerializedSize<RamsesRenderGroupBinding>(), 336u);
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithMeshNodeBinding)
    {
        if (GetParam() < EFeatureLevel_05)
        {
            GTEST_SKIP();
        }
        ApiObjects toSerialize{ GetParam() };
        toSerialize.createRamsesMeshNodeBinding(*m_meshNode, "mb");
        EXPECT_EQ(toSerialize.getSerializedSize<RamsesMeshNodeBinding>(), 376u);
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithLinearAnimation)
    {
        // Since there is a template specialization for animations collecting data arrays
        // we test with different data arrays and reusing one single data array.

        // Test animation channel with different data arrays
        {
            ApiObjects toSerialize{ GetParam() };

            auto dataArray1 = toSerialize.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data1");
            auto dataArray2 = toSerialize.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data2");
            ASSERT_NE(nullptr, dataArray1);
            ASSERT_NE(nullptr, dataArray2);

            AnimationNodeConfig config;
            config.addChannel({ "channel", dataArray1, dataArray2, EInterpolationType::Linear });
            toSerialize.createAnimationNode(*config.m_impl, "animation");
            EXPECT_EQ(toSerialize.getSerializedSize<AnimationNode>(), 378u);
            EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
        }
        // Test animation channel with the same data arrays
        {
            ApiObjects toSerialize{ GetParam() };

            auto data = toSerialize.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
            ASSERT_NE(nullptr, data);

            AnimationNodeConfig config;
            config.addChannel({ "channel", data, data, EInterpolationType::Linear });
            toSerialize.createAnimationNode(*config.m_impl, "animation");
            EXPECT_EQ(toSerialize.getSerializedSize<AnimationNode>(), 378u);
            EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
        }
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithCubicAnimation)
    {
        // Since there is a template specialization for animations collecting data arrays
        // we test with different data arrays and reusing one single data array.

        // Test animation channel with different data arrays
        {
            ApiObjects toSerialize{ GetParam() };

            auto dataArray1 = toSerialize.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data1");
            auto dataArray2 = toSerialize.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data2");
            auto dataArray3 = toSerialize.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data3");
            auto dataArray4 = toSerialize.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data4");
            ASSERT_NE(nullptr, dataArray1);
            ASSERT_NE(nullptr, dataArray2);
            ASSERT_NE(nullptr, dataArray3);
            ASSERT_NE(nullptr, dataArray4);

            AnimationNodeConfig config;
            config.addChannel({ "channel", dataArray1, dataArray2, EInterpolationType::Cubic, dataArray3, dataArray4 });
            toSerialize.createAnimationNode(*config.m_impl, "animation");
            EXPECT_EQ(toSerialize.getSerializedSize<AnimationNode>(), 378u);
            EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
        }
        // Test animation channel with the same data arrays
        {
            ApiObjects toSerialize{ GetParam() };

            auto data = toSerialize.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
            ASSERT_NE(nullptr, data);

            AnimationNodeConfig config;
            config.addChannel({ "channel", data, data, EInterpolationType::Cubic, data, data });
            toSerialize.createAnimationNode(*config.m_impl, "animation");
            EXPECT_EQ(toSerialize.getSerializedSize<AnimationNode>(), 378u);
            EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
        }
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithDataArray)
    {
        ApiObjects toSerialize{ GetParam() };
        toSerialize.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        EXPECT_EQ(toSerialize.getSerializedSize<DataArray>(), 98u);
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithTimer)
    {
        ApiObjects toSerialize{ GetParam() };
        toSerialize.createTimerNode("timer");
        EXPECT_EQ(toSerialize.getSerializedSize<TimerNode>(), 290u);
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithAnchorPoint)
    {
        if (GetParam() < EFeatureLevel_02)
        {
            GTEST_SKIP();
        }
        ApiObjects toSerialize{ GetParam() };
        const auto node = toSerialize.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
        const auto camera = toSerialize.createRamsesCameraBinding(*m_camera, true, "camera");
        ASSERT_TRUE(node && camera);
        toSerialize.createAnchorPoint(node->m_nodeBinding, camera->m_cameraBinding, "timer");
        EXPECT_EQ(toSerialize.getSerializedSize<AnchorPoint>(), 248u);
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }

    TEST_P(AnApiObjects_Serialization, ChecksSerializedSizeWithSkinBinding)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        ApiObjects toSerialize{ GetParam() };
        createSkinBinding(toSerialize);
        EXPECT_EQ(toSerialize.getSerializedSize<SkinBinding>(), 184u);
        EXPECT_GT(toSerialize.getTotalSerializedSize(), m_emptySerializedSizeTotal);
    }
}
