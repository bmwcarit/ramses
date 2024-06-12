//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"

#include "internal/logic/flatbuffers/generated/LogicEngineGen.h"

#include "internal/logic/ApiObjects.h"
#include "internal/logic/SolState.h"
#include "impl/ErrorReporting.h"

#include "impl/logic/LogicEngineImpl.h"
#include "impl/logic/PropertyImpl.h"
#include "impl/logic/LuaModuleImpl.h"
#include "impl/logic/LuaScriptImpl.h"
#include "impl/logic/LuaInterfaceImpl.h"
#include "impl/logic/NodeBindingImpl.h"
#include "impl/logic/AppearanceBindingImpl.h"
#include "impl/logic/CameraBindingImpl.h"
#include "impl/logic/RenderPassBindingImpl.h"
#include "impl/logic/RenderGroupBindingImpl.h"
#include "impl/logic/MeshNodeBindingImpl.h"
#include "impl/logic/DataArrayImpl.h"
#include "impl/logic/AnchorPointImpl.h"
#include "impl/logic/AnimationNodeImpl.h"
#include "impl/logic/SkinBindingImpl.h"
#include "impl/logic/TimerNodeImpl.h"
#include "impl/logic/RenderBufferBindingImpl.h"

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/LuaModule.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/RenderGroupBindingElements.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/AnimationNodeConfig.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AnchorPoint.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/RenderBufferBinding.h"
#include "ramses/client/Scene.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderBuffer.h"
#include "RamsesTestUtils.h"
#include "LogTestUtils.h"
#include "SerializationTestUtils.h"
#include "RamsesObjectResolverMock.h"
#include "FeatureLevelTestValues.h"

namespace ramses::internal
{
    class AnApiObjects : public ::testing::TestWithParam<EFeatureLevel>
    {
    protected:
        AnApiObjects()
        {
            m_renderGroup->addMeshNode(*m_meshNode);
        }

        RamsesTestSetup m_ramses;
        ramses::Scene* m_scene = { m_ramses.createScene() };
        ApiObjects m_apiObjects{ GetParam(), m_scene->impl() };

        ErrorReporting m_errorReporting;
        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        SerializationTestUtils m_testUtils{ m_flatBufferBuilder };
        ::testing::StrictMock<RamsesObjectResolverMock> m_resolverMock;

        ramses::Node* m_node = { m_scene->createNode() };
        ramses::PerspectiveCamera* m_camera = { m_scene->createPerspectiveCamera() };
        ramses::Appearance* m_appearance = { &RamsesTestSetup::CreateTrivialTestAppearance(*m_scene) };
        ramses::RenderPass* m_renderPass = { m_scene->createRenderPass() };
        ramses::RenderGroup* m_renderGroup = { m_scene->createRenderGroup() };
        ramses::MeshNode* m_meshNode = { m_scene->createMeshNode("meshNode") };
        ramses::RenderBuffer* m_renderBuffer = { m_scene->createRenderBuffer(1u, 2u, ERenderBufferFormat::R16F, ERenderBufferAccessMode::ReadWrite, 3u, "renderBuffer") };

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
            auto intf = apiObjects.createLuaInterface(m_valid_empty_interface, {}, "intf", m_errorReporting);
            EXPECT_NE(nullptr, intf);
            return intf;
        }

        AnchorPoint* createAnchorPoint()
        {
            auto* node = m_apiObjects.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
            auto* camera = m_apiObjects.createCameraBinding(*m_camera, true, "camera");
            EXPECT_TRUE(node && camera);
            return m_apiObjects.createAnchorPoint(node->impl(), camera->impl(), "anchor");
        }

        RenderGroupBinding* createRenderGroupBinding()
        {
            RenderGroupBindingElements elements;
            EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
            return m_apiObjects.createRenderGroupBinding(*m_renderGroup, elements, "renderGroupBinding");
        }

        static SkinBinding* createSkinBinding(const NodeBinding& joint, AppearanceBinding& appearance, ApiObjects& apiObjects)
        {
            const auto optUniform = appearance.getRamsesAppearance().getEffect().findUniformInput("jointMat");
            EXPECT_TRUE(optUniform.has_value());
            assert(optUniform != std::nullopt);
            return apiObjects.createSkinBinding({ &joint.impl() }, { matrix44f{ 0.f } }, appearance.impl(), *optUniform, "skin");
        }

        SkinBinding* createSkinBinding(ApiObjects& apiObjects)
        {
            const auto* node       = apiObjects.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodeForSkin");
            auto*       appearance = apiObjects.createAppearanceBinding(*m_appearance, "appearanceForSkin");
            return createSkinBinding(*node, *appearance, apiObjects);
        }

        const LogicObject* getApiObjectById(uint64_t id)
        {
            const auto& objs = m_apiObjects.getApiObjectContainer<LogicObject>();
            const auto it = std::find_if(objs.cbegin(), objs.cend(), [id](const auto o) { return o->getSceneObjectId().getValue() == id; });
            const auto* obj = (it == objs.cend() ? nullptr : *it);

            const auto& objsOwn = m_apiObjects.getApiObjectOwningContainer();
            const auto it2 = std::find_if(objsOwn.cbegin(), objsOwn.cend(), [id](const auto& o) { return o->getSceneObjectId().getValue() == id; });
            const auto* objOwn = (it2 == objsOwn.cend() ? nullptr : it2->get());

            EXPECT_EQ(obj, objOwn);
            return obj;
        }

        // Silence logs, unless explicitly enabled, to reduce spam and speed up tests
        ScopedLogContextLevel m_silenceLogs{ CONTEXT_CLIENT, ELogLevel::Off };

        size_t m_emptySerializedSizeTotal{164u};
    };

    RAMSES_INSTANTIATE_LATEST_FEATURELEVEL_ONLY_TEST_SUITE(AnApiObjects);

    TEST_P(AnApiObjects, CreatesScriptFromValidLuaWithoutErrors)
    {
        const LuaScript* script = createScript();
        EXPECT_FALSE(m_errorReporting.getError().has_value());
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
        ApiObjects otherInstance{ GetParam(), m_scene->impl() };
        LuaScript* script = createScript(otherInstance, m_valid_empty_script);
        EXPECT_EQ(script, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(script, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*script, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy object 'script [LogicObject ScnObjId=9]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(m_errorReporting.getError()->object, script);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(script, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(script, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesInterfaceFromValidLuaWithoutErrors)
    {
        const LuaInterface* intf = createInterface();
        EXPECT_FALSE(m_errorReporting.getError().has_value());
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
        ApiObjects otherInstance{ GetParam(), m_scene->impl() };
        LuaInterface* intf = createInterface(otherInstance);
        EXPECT_EQ(intf, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(intf, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*intf, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy object 'intf [LogicObject ScnObjId=9]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(m_errorReporting.getError()->object, intf);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(intf, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(intf, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesLuaModule)
    {
        auto module = m_apiObjects.createLuaModule(m_moduleSrc, {}, "module", m_errorReporting);
        EXPECT_NE(nullptr, module);

        EXPECT_FALSE(m_errorReporting.getError().has_value());
        ASSERT_EQ(1u, m_apiObjects.getApiObjectContainer<LuaModule>().size());
        EXPECT_EQ(1u, m_apiObjects.getApiObjectContainer<LogicObject>().size());
        EXPECT_EQ(1u, m_apiObjects.getApiObjectOwningContainer().size());
        EXPECT_EQ(module, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(module, m_apiObjects.getApiObjectContainer<LogicObject>().back());
        EXPECT_EQ(module, m_apiObjects.getApiObjectContainer<LuaModule>().front());
    }

    TEST_P(AnApiObjects, CreatesNodeBindingWithoutErrors)
    {
        NodeBinding* nodeBinding = m_apiObjects.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        EXPECT_NE(nullptr, nodeBinding);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_EQ(nodeBinding, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(nodeBinding, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysNodeBindingWithoutErrors)
    {
        NodeBinding* nodeBinding = m_apiObjects.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        ASSERT_NE(nullptr, nodeBinding);
        m_apiObjects.destroy(*nodeBinding, m_errorReporting);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingNodeBindingFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam(), m_scene->impl() };
        NodeBinding* nodeBinding = otherInstance.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        ASSERT_TRUE(nodeBinding);
        EXPECT_EQ(nodeBinding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(nodeBinding, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*nodeBinding, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy object 'NodeBinding [LogicObject ScnObjId=9]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(m_errorReporting.getError()->object, nodeBinding);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(nodeBinding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(nodeBinding, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesCameraBindingWithoutErrors)
    {
        CameraBinding* cameraBinding = m_apiObjects.createCameraBinding(*m_camera, false, "CameraBinding");
        EXPECT_NE(nullptr, cameraBinding);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_EQ(cameraBinding, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(cameraBinding, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysCameraBindingWithoutErrors)
    {
        CameraBinding* cameraBinding = m_apiObjects.createCameraBinding(*m_camera, true, "CameraBinding");
        ASSERT_NE(nullptr, cameraBinding);
        m_apiObjects.destroy(*cameraBinding, m_errorReporting);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingCameraBindingFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam(), m_scene->impl() };
        CameraBinding* cameraBinding = otherInstance.createCameraBinding(*m_camera, true, "CameraBinding");
        ASSERT_TRUE(cameraBinding);
        EXPECT_EQ(cameraBinding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(cameraBinding, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*cameraBinding, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy object 'CameraBinding [LogicObject ScnObjId=9]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(m_errorReporting.getError()->object, cameraBinding);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(cameraBinding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(cameraBinding, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesRenderPassBindingWithoutErrors)
    {
        RenderPassBinding* binding = m_apiObjects.createRenderPassBinding(*m_renderPass, "RenderPassBinding");
        EXPECT_NE(nullptr, binding);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_EQ(binding, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysRenderPassBindingWithoutErrors)
    {
        RenderPassBinding* binding = m_apiObjects.createRenderPassBinding(*m_renderPass, "RenderPassBinding");
        ASSERT_NE(nullptr, binding);
        m_apiObjects.destroy(*binding, m_errorReporting);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingRenderPassBindingFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam(), m_scene->impl() };
        RenderPassBinding* binding = otherInstance.createRenderPassBinding(*m_renderPass, "RenderPassBinding");
        ASSERT_TRUE(binding);
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*binding, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy object 'RenderPassBinding [LogicObject ScnObjId=9]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(m_errorReporting.getError()->object, binding);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesRenderGroupBindingWithoutErrors)
    {
        const auto binding = createRenderGroupBinding();
        EXPECT_NE(nullptr, binding);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_EQ(binding, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysRenderGroupBindingWithoutErrors)
    {
        auto binding = createRenderGroupBinding();
        ASSERT_NE(nullptr, binding);
        m_apiObjects.destroy(*binding, m_errorReporting);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingRenderGroupBindingFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam(), m_scene->impl() };
        RenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(*m_meshNode));
        auto binding = otherInstance.createRenderGroupBinding(*m_renderGroup, elements, "RenderGroupBinding");
        ASSERT_TRUE(binding);
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*binding, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy object 'RenderGroupBinding [LogicObject ScnObjId=9]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(m_errorReporting.getError()->object, binding);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesMeshNodeBindingWithoutErrors)
    {
        const auto binding = m_apiObjects.createMeshNodeBinding(*m_meshNode, "mb");
        EXPECT_NE(nullptr, binding);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_EQ(binding, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysMeshNodeBindingWithoutErrors)
    {
        auto binding = m_apiObjects.createMeshNodeBinding(*m_meshNode, "mb");
        ASSERT_NE(nullptr, binding);
        m_apiObjects.destroy(*binding, m_errorReporting);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingMeshNodeBindingFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam(), m_scene->impl() };
        auto binding = otherInstance.createMeshNodeBinding(*m_meshNode, "mb");
        ASSERT_TRUE(binding);
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
        EXPECT_FALSE(m_apiObjects.destroy(*binding, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy object 'mb [LogicObject ScnObjId=9]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(m_errorReporting.getError()->object, binding);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesAppearanceBindingWithoutErrors)
    {
        AppearanceBinding* binding = m_apiObjects.createAppearanceBinding(*m_appearance, "AppearanceBinding");
        EXPECT_NE(nullptr, binding);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_EQ(binding, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysAppearanceBindingWithoutErrors)
    {
        AppearanceBinding* binding = m_apiObjects.createAppearanceBinding(*m_appearance, "AppearanceBinding");
        ASSERT_TRUE(binding);
        ASSERT_TRUE(m_apiObjects.destroy(*binding, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingAppearanceBindingFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam(), m_scene->impl() };
        AppearanceBinding* binding = otherInstance.createAppearanceBinding(*m_appearance, "AppearanceBinding");
        ASSERT_TRUE(binding);
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*binding, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy object 'AppearanceBinding [LogicObject ScnObjId=9]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(m_errorReporting.getError()->object, binding);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(binding, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(binding, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesDataArray)
    {
        const std::vector<float> data{ 1.f, 2.f, 3.f };
        auto dataArray = m_apiObjects.createDataArray(data, "data");
        EXPECT_NE(nullptr, dataArray);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
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
        EXPECT_FALSE(m_errorReporting.getError().has_value());
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
        auto animNode = m_apiObjects.createAnimationNode(config.impl(), "animNode");

        EXPECT_FALSE(m_apiObjects.destroy(*dataArray1, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy data array 'data1', it is used in animation node 'animNode' channel 'channel1'");
        EXPECT_EQ(m_errorReporting.getError()->object, dataArray1);
        m_errorReporting.reset();

        EXPECT_FALSE(m_apiObjects.destroy(*dataArray2, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy data array 'data2', it is used in animation node 'animNode' channel 'channel1'");
        EXPECT_EQ(m_errorReporting.getError()->object, dataArray2);
        m_errorReporting.reset();

        EXPECT_FALSE(m_apiObjects.destroy(*dataArray3, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy data array 'data3', it is used in animation node 'animNode' channel 'channel2'");
        EXPECT_EQ(m_errorReporting.getError()->object, dataArray3);
        m_errorReporting.reset();

        EXPECT_FALSE(m_apiObjects.destroy(*dataArray4,  m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy data array 'data4', it is used in animation node 'animNode' channel 'channel2'");
        EXPECT_EQ(m_errorReporting.getError()->object, dataArray4);
        m_errorReporting.reset();

        // succeeds after destroying animation node
        EXPECT_TRUE(m_apiObjects.destroy(*animNode, m_errorReporting));
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_TRUE(m_apiObjects.destroy(*dataArray1, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*dataArray2, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*dataArray3, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*dataArray4, m_errorReporting));
        EXPECT_FALSE(m_errorReporting.getError().has_value());
    }

    TEST_P(AnApiObjects, FailsToDestroyDataArrayFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam(), m_scene->impl() };
        auto dataArray = otherInstance.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        ASSERT_NE(nullptr, dataArray);
        EXPECT_EQ(dataArray, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(dataArray, otherInstance.getApiObjectContainer<LogicObject>().back());
        EXPECT_FALSE(m_apiObjects.destroy(*dataArray, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy object 'data [LogicObject ScnObjId=9]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(m_errorReporting.getError()->object, dataArray);

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
        auto animNode = m_apiObjects.createAnimationNode(config.impl(), "animNode");
        EXPECT_FALSE(m_errorReporting.getError().has_value());
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
        auto animNode = m_apiObjects.createAnimationNode(config.impl(), "animNode");
        EXPECT_TRUE(m_apiObjects.destroy(*animNode, m_errorReporting));
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<AnimationNode>().empty());
        // did not affect data array
        EXPECT_TRUE(!m_apiObjects.getApiObjectContainer<DataArray>().empty());
        EXPECT_EQ(1u, m_apiObjects.getApiObjectOwningContainer().size());
        EXPECT_EQ(1u, m_apiObjects.getApiObjectContainer<LogicObject>().size());
    }

    TEST_P(AnApiObjects, FailsToDestroyAnimationNodeFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam(), m_scene->impl() };
        auto dataArray = otherInstance.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        ASSERT_NE(nullptr, dataArray);
        AnimationNodeConfig config;
        EXPECT_TRUE(config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear }));
        auto animNode = otherInstance.createAnimationNode(config.impl(), "animNode");
        EXPECT_EQ(animNode, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(animNode, otherInstance.getApiObjectContainer<LogicObject>().back());
        EXPECT_FALSE(m_apiObjects.destroy(*animNode, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy object 'animNode [LogicObject ScnObjId=10]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(m_errorReporting.getError()->object, animNode);

        // Did not affect existence in otherInstance!
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<AnimationNode>().empty());
        EXPECT_EQ(animNode, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(animNode, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, CreatesTimerNode)
    {
        auto timerNode = m_apiObjects.createTimerNode("timerNode");
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        ASSERT_EQ(1u, m_apiObjects.getApiObjectOwningContainer().size());
        EXPECT_EQ(timerNode, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_THAT(m_apiObjects.getApiObjectContainer<LogicObject>(), ::testing::ElementsAre(timerNode));
        EXPECT_THAT(m_apiObjects.getApiObjectContainer<TimerNode>(), ::testing::ElementsAre(timerNode));
    }

    TEST_P(AnApiObjects, DestroysTimerNode)
    {
        auto timerNode = m_apiObjects.createTimerNode("timerNode");
        EXPECT_TRUE(m_apiObjects.destroy(*timerNode, m_errorReporting));
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<TimerNode>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
    }

    TEST_P(AnApiObjects, FailsToDestroyTimerNodeFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam(), m_scene->impl() };
        auto timerNode = otherInstance.createTimerNode("timerNode");
        EXPECT_FALSE(m_apiObjects.destroy(*timerNode, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy object 'timerNode [LogicObject ScnObjId=9]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(m_errorReporting.getError()->object, timerNode);

        // Did not affect existence in otherInstance!
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<TimerNode>().empty());
        EXPECT_EQ(timerNode, otherInstance.getApiObjectContainer<TimerNode>().front());
        EXPECT_EQ(timerNode, otherInstance.getApiObjectOwningContainer().front().get());
        EXPECT_EQ(timerNode, otherInstance.getApiObjectContainer<LogicObject>().front());
    }

    TEST_P(AnApiObjects, CreatesAnchorPointWithoutErrors)
    {
        AnchorPoint* anchor = createAnchorPoint();
        EXPECT_NE(nullptr, anchor);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_EQ(anchor, m_apiObjects.getApiObjectContainer<AnchorPoint>().front());
        EXPECT_EQ(anchor, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(anchor, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysAnchorPointWithoutErrors)
    {
        AnchorPoint* anchor = createAnchorPoint();
        ASSERT_NE(nullptr, anchor);
        m_apiObjects.destroy(*anchor, m_errorReporting);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<AnchorPoint>().empty());
        EXPECT_NE(anchor, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_NE(anchor, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingAnchorPointFromAnotherClassInstance)
    {
        ApiObjects otherInstance{ GetParam(), m_scene->impl() };
        auto* node = otherInstance.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
        auto* camera = otherInstance.createCameraBinding(*m_camera, true, "camera");
        AnchorPoint* anchor = otherInstance.createAnchorPoint(node->impl(), camera->impl(), "anchor");
        ASSERT_TRUE(anchor);
        EXPECT_EQ(anchor, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(anchor, otherInstance.getApiObjectContainer<LogicObject>().back());
        ASSERT_FALSE(m_apiObjects.destroy(*anchor, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy object 'anchor [LogicObject ScnObjId=11]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(m_errorReporting.getError()->object, anchor);

        // Did not affect existence in otherInstance!
        EXPECT_EQ(anchor, otherInstance.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(anchor, otherInstance.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, FailsToDestroyNodeOrCameraBindingIfUsedInAnchorPoint)
    {
        auto* node = m_apiObjects.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
        auto* camera = m_apiObjects.createCameraBinding(*m_camera, true, "camera");
        AnchorPoint* anchor = m_apiObjects.createAnchorPoint(node->impl(), camera->impl(), "anchor");

        EXPECT_FALSE(m_apiObjects.destroy(*node, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy Ramses node binding 'node', it is used in anchor point 'anchor'");
        EXPECT_EQ(m_errorReporting.getError()->object, node);
        m_errorReporting.reset();

        EXPECT_FALSE(m_apiObjects.destroy(*camera, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy Ramses camera binding 'camera', it is used in anchor point 'anchor'");
        EXPECT_EQ(m_errorReporting.getError()->object, camera);
        m_errorReporting.reset();

        // succeeds after destroying anchor point
        EXPECT_TRUE(m_apiObjects.destroy(*anchor, m_errorReporting));
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_TRUE(m_apiObjects.destroy(*node, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*camera, m_errorReporting));
        EXPECT_FALSE(m_errorReporting.getError().has_value());
    }

    TEST_P(AnApiObjects, CreatesSkinBindingWithoutErrors)
    {
        auto skin = createSkinBinding(m_apiObjects);
        ASSERT_NE(nullptr, skin);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_EQ(skin, m_apiObjects.getApiObjectContainer<SkinBinding>().front());
        EXPECT_EQ(skin, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_EQ(skin, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, DestroysSkinBindingWithoutErrors)
    {
        auto skin = createSkinBinding(m_apiObjects);
        ASSERT_NE(nullptr, skin);
        m_apiObjects.destroy(*skin, m_errorReporting);
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<SkinBinding>().empty());
        EXPECT_NE(skin, m_apiObjects.getApiObjectOwningContainer().back().get());
        EXPECT_NE(skin, m_apiObjects.getApiObjectContainer<LogicObject>().back());
    }

    TEST_P(AnApiObjects, ProducesErrorsWhenDestroyingSkinBindingFromAnotherClassInstance)
    {
        auto skin = createSkinBinding(m_apiObjects);
        ASSERT_NE(nullptr, skin);

        ApiObjects otherInstance{ GetParam(), m_scene->impl() };

        ErrorReporting  errorReporting;
        EXPECT_FALSE(otherInstance.destroy(*skin, errorReporting));
        ASSERT_TRUE(errorReporting.getError().has_value());
        EXPECT_EQ(errorReporting.getError()->message, "Failed to destroy object 'skin [LogicObject ScnObjId=11]', cannot find it in this LogicEngine instance.");
        EXPECT_EQ(errorReporting.getError()->object, skin);
    }

    TEST_P(AnApiObjects, FailsToDestroyNodeOrAppearanceBindingIfUsedInSkinBinding)
    {
        auto skin = createSkinBinding(m_apiObjects);
        ASSERT_NE(nullptr, skin);

        const auto& nodes = m_apiObjects.getApiObjectContainer<NodeBinding>();
        const auto it = std::find_if(nodes.cbegin(), nodes.cend(), [](const auto& n) { return n->getName() == "nodeForSkin"; });
        ASSERT_TRUE(it != nodes.cend());
        const auto nodeUsedInSkin = *it;

        const auto& appearances = m_apiObjects.getApiObjectContainer<AppearanceBinding>();
        const auto it2 = std::find_if(appearances.cbegin(), appearances.cend(), [](const auto& a) { return a->getName() == "appearanceForSkin"; });
        ASSERT_TRUE(it2 != appearances.cend());
        const auto appearanceUsedInSkin = *it2;

        EXPECT_FALSE(m_apiObjects.destroy(*nodeUsedInSkin, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy Ramses node binding 'nodeForSkin', it is used in skin binding 'skin'");
        EXPECT_EQ(m_errorReporting.getError()->object, nodeUsedInSkin);
        m_errorReporting.reset();

        EXPECT_FALSE(m_apiObjects.destroy(*appearanceUsedInSkin, m_errorReporting));
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Failed to destroy Ramses appearance binding 'appearanceForSkin', it is used in skin binding 'skin'");
        EXPECT_EQ(m_errorReporting.getError()->object, appearanceUsedInSkin);
        m_errorReporting.reset();

        // succeeds after destroying skin binding
        EXPECT_TRUE(m_apiObjects.destroy(*skin, m_errorReporting));
        EXPECT_FALSE(m_errorReporting.getError().has_value());
        EXPECT_TRUE(m_apiObjects.destroy(*nodeUsedInSkin, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*appearanceUsedInSkin, m_errorReporting));
        EXPECT_FALSE(m_errorReporting.getError().has_value());
    }

    TEST_P(AnApiObjects, ProvidesEmptyCollections_WhenNothingWasCreated)
    {
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LuaScript>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<NodeBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<AppearanceBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<CameraBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<RenderPassBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<RenderGroupBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<MeshNodeBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<DataArray>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<AnimationNode>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<TimerNode>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<AnchorPoint>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<SkinBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<RenderBufferBinding>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectContainer<LogicObject>().empty());
        EXPECT_TRUE(m_apiObjects.getApiObjectOwningContainer().empty());

        const ApiObjects& apiObjectsConst = m_apiObjects;
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<LuaScript>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<NodeBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<AppearanceBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<CameraBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<RenderPassBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<RenderGroupBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<MeshNodeBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<DataArray>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<AnimationNode>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<TimerNode>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<AnchorPoint>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<SkinBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<RenderBufferBinding>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectContainer<LogicObject>().empty());
        EXPECT_TRUE(apiObjectsConst.getApiObjectOwningContainer().empty());
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
        NodeBinding* binding = m_apiObjects.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        ApiObjectContainer<NodeBinding>& nodes = m_apiObjects.getApiObjectContainer<NodeBinding>();
        EXPECT_THAT(nodes, ::testing::ElementsAre(binding));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyAppearanceBindingsCollection_WhenAppearanceBindingsWereCreated)
    {
        AppearanceBinding* binding = m_apiObjects.createAppearanceBinding(*m_appearance, "");
        ApiObjectContainer<AppearanceBinding>& appearances = m_apiObjects.getApiObjectContainer<AppearanceBinding>();
        EXPECT_THAT(appearances, ::testing::ElementsAre(binding));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyCameraBindingsCollection_WhenCameraBindingsWereCreated)
    {
        CameraBinding* binding = m_apiObjects.createCameraBinding(*m_camera, true, "");
        ApiObjectContainer<CameraBinding>& cameras = m_apiObjects.getApiObjectContainer<CameraBinding>();
        EXPECT_THAT(cameras, ::testing::ElementsAre(binding));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyRenderPassBindingsCollection_WhenRenderPassBindingsWereCreated)
    {
        RenderPassBinding* binding = m_apiObjects.createRenderPassBinding(*m_renderPass, "");
        ApiObjectContainer<RenderPassBinding>& renderPasses = m_apiObjects.getApiObjectContainer<RenderPassBinding>();
        EXPECT_THAT(renderPasses, ::testing::ElementsAre(binding));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyRenderGroupBindingsCollection_WhenRenderGroupBindingsWereCreated)
    {
        const auto* binding = createRenderGroupBinding();
        ApiObjectContainer<RenderGroupBinding>& renderGroups = m_apiObjects.getApiObjectContainer<RenderGroupBinding>();
        EXPECT_THAT(renderGroups, ::testing::ElementsAre(binding));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyRenderBufferBindingsCollection_WhenRenderBufferBindingsWereCreated)
    {
        const auto* binding = m_apiObjects.createRenderBufferBinding(*m_renderBuffer, "rb");
        ApiObjectContainer<RenderBufferBinding>& bindings = m_apiObjects.getApiObjectContainer<RenderBufferBinding>();
        EXPECT_THAT(bindings, ::testing::ElementsAre(binding));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptyAnchorPointsCollection_WhenAnchorPointsWereCreated)
    {
        AnchorPoint* anchor = createAnchorPoint();
        const auto& anchors = m_apiObjects.getApiObjectContainer<AnchorPoint>();
        EXPECT_THAT(anchors, ::testing::ElementsAre(anchor));
    }

    TEST_P(AnApiObjects, ProvidesNonEmptySkinBindingsCollection_WhenSkinBindingsWereCreated)
    {
        const auto* skin = createSkinBinding(m_apiObjects);
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
        auto* nodeBinding       = m_apiObjects.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        auto* appearanceBinding = m_apiObjects.createAppearanceBinding(*m_appearance, "");
        auto* cameraBinding     = m_apiObjects.createCameraBinding(*m_camera, true, "");
        auto* dataArray         = m_apiObjects.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "data");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
        auto* animationNode     = m_apiObjects.createAnimationNode(config.impl(), "animNode");
        auto* timerNode         = m_apiObjects.createTimerNode("timerNode");
        auto* renderPassBinding = m_apiObjects.createRenderPassBinding(*m_renderPass, "");
        auto* anchor = m_apiObjects.createAnchorPoint(nodeBinding->impl(), cameraBinding->impl(), "anchor");
        auto* renderGroupBinding = createRenderGroupBinding();
        auto* skin = createSkinBinding(*nodeBinding, *appearanceBinding, m_apiObjects);
        auto* meshBinding = m_apiObjects.createMeshNodeBinding(*m_meshNode, "mb");

        // feature level 01 always present
        std::vector<LogicObject*> expectedObjects{ luaModule, luaScript, luaInterface, nodeBinding, appearanceBinding, cameraBinding, dataArray, animationNode,
            timerNode, renderPassBinding, anchor, renderGroupBinding, skin, meshBinding };

        std::vector<LogicObject*> ownedLogicObjectsRawPointers;
        std::transform(ownedObjects.cbegin(), ownedObjects.cend(), std::back_inserter(ownedLogicObjectsRawPointers), [](auto& obj) { return obj.get(); });
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
        const auto* nodeBinding       = m_apiObjects.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        auto*       appearanceBinding = m_apiObjects.createAppearanceBinding(*m_appearance, "");
        const auto* cameraBinding     = m_apiObjects.createCameraBinding(*m_camera, true, "");
        const auto* dataArray         = m_apiObjects.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "data");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
        const auto* animationNode     = m_apiObjects.createAnimationNode(config.impl(), "animNode");
        const auto* timerNode         = m_apiObjects.createTimerNode("timerNode");
        const auto* renderPassBinding = m_apiObjects.createRenderPassBinding(*m_renderPass, "");
        const auto* anchor = createAnchorPoint();
        const auto* renderGroupBinding = createRenderGroupBinding();
        const auto* skin = createSkinBinding(*nodeBinding, *appearanceBinding, m_apiObjects);
        const auto* meshBinding = m_apiObjects.createMeshNodeBinding(*m_meshNode, "mb");

        std::unordered_set<sceneObjectId_t> ids =
        {
            luaModule->getSceneObjectId(),
            luaScript->getSceneObjectId(),
            luaInterface->getSceneObjectId(),
            nodeBinding->getSceneObjectId(),
            appearanceBinding->getSceneObjectId(),
            cameraBinding->getSceneObjectId(),
            dataArray->getSceneObjectId(),
            animationNode->getSceneObjectId(),
            timerNode->getSceneObjectId(),
            renderPassBinding->getSceneObjectId(),
            anchor->getSceneObjectId(),
            renderGroupBinding->getSceneObjectId(),
            skin->getSceneObjectId(),
            meshBinding->getSceneObjectId()
        };
        EXPECT_EQ(14u, ids.size());
    }

    TEST_P(AnApiObjects, logicObjectIdsAreRemovedFromIdMappingWhenObjectIsDestroyed)
    {
        const auto* luaModule         = m_apiObjects.createLuaModule(m_moduleSrc, {}, "module", m_errorReporting);
        auto* luaScript               = createScript(m_apiObjects, m_valid_empty_script);
        const auto* nodeBinding       = m_apiObjects.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        auto* appearanceBinding       = m_apiObjects.createAppearanceBinding(*m_appearance, "");
        const auto* cameraBinding     = m_apiObjects.createCameraBinding(*m_camera, true, "");
        const auto* dataArray         = m_apiObjects.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "data");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
        auto* animationNode           = m_apiObjects.createAnimationNode(config.impl(), "animNode");
        const auto* timerNode         = m_apiObjects.createTimerNode("timerNode");
        const auto* luaInterface      = createInterface();
        const RenderPassBinding* renderPassBinding = m_apiObjects.createRenderPassBinding(*m_renderPass, "");
        const AnchorPoint* anchor = createAnchorPoint();
        const RenderGroupBinding* renderGroupBinding = createRenderGroupBinding();
        const SkinBinding* skinBinding = createSkinBinding(m_apiObjects);
        const MeshNodeBinding* meshBinding = m_apiObjects.createMeshNodeBinding(*m_meshNode, "mb");
        const auto* rbBinding = m_apiObjects.createRenderBufferBinding(*m_renderBuffer, "rb");

        EXPECT_EQ(getApiObjectById(9u), luaModule);
        EXPECT_EQ(getApiObjectById(10u), luaScript);
        EXPECT_EQ(getApiObjectById(11u), nodeBinding);
        EXPECT_EQ(getApiObjectById(12u), appearanceBinding);
        EXPECT_EQ(getApiObjectById(13u), cameraBinding);
        EXPECT_EQ(getApiObjectById(14u), dataArray);
        EXPECT_EQ(getApiObjectById(15u), animationNode);
        EXPECT_EQ(getApiObjectById(16u), timerNode);
        EXPECT_EQ(getApiObjectById(17u), luaInterface);
        EXPECT_EQ(getApiObjectById(18u), renderPassBinding);
        EXPECT_EQ(getApiObjectById(21u), anchor);
        EXPECT_EQ(getApiObjectById(22u), renderGroupBinding);
        EXPECT_EQ(getApiObjectById(25u), skinBinding);
        EXPECT_EQ(getApiObjectById(26u), meshBinding);
        EXPECT_EQ(getApiObjectById(27u), rbBinding);

        EXPECT_TRUE(m_apiObjects.destroy(*luaScript, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*appearanceBinding, m_errorReporting));
        EXPECT_TRUE(m_apiObjects.destroy(*animationNode, m_errorReporting));

        EXPECT_EQ(getApiObjectById(9u), luaModule);
        EXPECT_EQ(getApiObjectById(10u), nullptr);
        EXPECT_EQ(getApiObjectById(11u), nodeBinding);
        EXPECT_EQ(getApiObjectById(12u), nullptr);
        EXPECT_EQ(getApiObjectById(13u), cameraBinding);
        EXPECT_EQ(getApiObjectById(14u), dataArray);
        EXPECT_EQ(getApiObjectById(15u), nullptr);
        EXPECT_EQ(getApiObjectById(16u), timerNode);
        EXPECT_EQ(getApiObjectById(17u), luaInterface);
        EXPECT_EQ(getApiObjectById(18u), renderPassBinding);
        EXPECT_EQ(getApiObjectById(21u), anchor);
        EXPECT_EQ(getApiObjectById(22u), renderGroupBinding);
        EXPECT_EQ(getApiObjectById(25u), skinBinding);
        EXPECT_EQ(getApiObjectById(26u), meshBinding);
        EXPECT_EQ(getApiObjectById(27u), rbBinding);
    }

    TEST_P(AnApiObjects, logicObjectsGenerateIdentificationStringWithUserId)
    {
        auto* luaModule = m_apiObjects.createLuaModule(m_moduleSrc, {}, "module", m_errorReporting);
        auto* luaScript = createScript(m_apiObjects, m_valid_empty_script);
        auto* nodeBinding = m_apiObjects.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodeBinding");
        auto* appearanceBinding = m_apiObjects.createAppearanceBinding(*m_appearance, "appearanceBinding");
        auto* cameraBinding = m_apiObjects.createCameraBinding(*m_camera, true, "cameraBinding");
        RenderPassBinding* renderPassBinding = m_apiObjects.createRenderPassBinding(*m_renderPass, "renderPassBinding");
        auto* dataArray = m_apiObjects.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataArray");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
        auto* animationNode = m_apiObjects.createAnimationNode(config.impl(), "animNode");
        auto* timerNode = m_apiObjects.createTimerNode("timerNode");
        auto* luaInterface = createInterface();
        AnchorPoint* anchor = createAnchorPoint();
        RenderGroupBinding* renderGroupBinding = createRenderGroupBinding();
        SkinBinding* skin = createSkinBinding(m_apiObjects);
        MeshNodeBinding* meshBinding = m_apiObjects.createMeshNodeBinding(*m_meshNode, "mb");
        auto* rbBinding = m_apiObjects.createRenderBufferBinding(*m_renderBuffer, "rb");

        EXPECT_TRUE(luaModule->setUserId(1u, 2u));
        EXPECT_TRUE(luaScript->setUserId(3u, 4u));
        EXPECT_TRUE(nodeBinding->setUserId(5u, 6u));
        EXPECT_TRUE(appearanceBinding->setUserId(7u, 8u));
        EXPECT_TRUE(cameraBinding->setUserId(9u, 10u));
        EXPECT_TRUE(renderPassBinding->setUserId(11u, 12u));
        EXPECT_TRUE(dataArray->setUserId(13u, 14u));
        EXPECT_TRUE(animationNode->setUserId(15u, 16u));
        EXPECT_TRUE(timerNode->setUserId(17u, 18u));
        EXPECT_TRUE(luaInterface->setUserId(19u, 20u));
        EXPECT_TRUE(anchor->setUserId(21u, 22u));
        EXPECT_TRUE(renderGroupBinding->setUserId(23u, 24u));
        EXPECT_TRUE(skin->setUserId(25u, 26u));
        EXPECT_TRUE(meshBinding->setUserId(27u, 28u));
        EXPECT_TRUE(rbBinding->setUserId(29u, 30u));

        EXPECT_EQ(luaModule->impl().getIdentificationString(), "module [LogicObject UserId=00000000000000010000000000000002 ScnObjId=9]");
        EXPECT_EQ(luaScript->impl().getIdentificationString(), "script [LogicObject UserId=00000000000000030000000000000004 ScnObjId=10]");
        EXPECT_EQ(nodeBinding->impl().getIdentificationString(), "nodeBinding [LogicObject UserId=00000000000000050000000000000006 ScnObjId=11]");
        EXPECT_EQ(appearanceBinding->impl().getIdentificationString(), "appearanceBinding [LogicObject UserId=00000000000000070000000000000008 ScnObjId=12]");
        EXPECT_EQ(cameraBinding->impl().getIdentificationString(), "cameraBinding [LogicObject UserId=0000000000000009000000000000000A ScnObjId=13]");
        EXPECT_EQ(renderPassBinding->impl().getIdentificationString(), "renderPassBinding [LogicObject UserId=000000000000000B000000000000000C ScnObjId=14]");
        EXPECT_EQ(dataArray->impl().getIdentificationString(), "dataArray [LogicObject UserId=000000000000000D000000000000000E ScnObjId=15]");
        EXPECT_EQ(animationNode->impl().getIdentificationString(), "animNode [LogicObject UserId=000000000000000F0000000000000010 ScnObjId=16]");
        EXPECT_EQ(timerNode->impl().getIdentificationString(), "timerNode [LogicObject UserId=00000000000000110000000000000012 ScnObjId=17]");
        EXPECT_EQ(luaInterface->impl().getIdentificationString(), "intf [LogicObject UserId=00000000000000130000000000000014 ScnObjId=18]");
        EXPECT_EQ(anchor->impl().getIdentificationString(), "anchor [LogicObject UserId=00000000000000150000000000000016 ScnObjId=21]");
        EXPECT_EQ(renderGroupBinding->impl().getIdentificationString(), "renderGroupBinding [LogicObject UserId=00000000000000170000000000000018 ScnObjId=22]");
        EXPECT_EQ(skin->impl().getIdentificationString(), "skin [LogicObject UserId=0000000000000019000000000000001A ScnObjId=25]");
        EXPECT_EQ(meshBinding->impl().getIdentificationString(), "mb [LogicObject UserId=000000000000001B000000000000001C ScnObjId=26]");
        EXPECT_EQ(rbBinding->impl().getIdentificationString(), "rb [LogicObject UserId=000000000000001D000000000000001E ScnObjId=27]");
    }

    TEST_P(AnApiObjects, ValidatesThatAllLuaInterfaceOutputsAreLinked_GeneratesWarningsIfOutputsNotLinked)
    {
        LuaInterface* intf = m_apiObjects.createLuaInterface(R"(
            function interface(IN,OUT)

                IN.param1 = Type:Int32()
                IN.param2 = {a=Type:Float(), b=Type:Int32()}

            end
        )", {}, "intf name", m_errorReporting);
        ASSERT_NE(nullptr, intf);

        ValidationReportImpl validationResults;
        m_apiObjects.validateInterfaces(validationResults);
        EXPECT_EQ(3u, validationResults.getIssues().size());
        EXPECT_THAT(validationResults.getIssues(),
            ::testing::Each(::testing::Field(&Issue::message, ::testing::HasSubstr("Interface [intf name] has unlinked output"))));
        EXPECT_THAT(validationResults.getIssues(), ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_P(AnApiObjects, ValidatesThatAllLuaInterfaceOutputsAreLinked_DoesNotGenerateWarningsIfAllOutputsLinked)
    {
        LuaInterface* intf = m_apiObjects.createLuaInterface(R"(
            function interface(IN,OUT)

                IN.param1 = Type:Int32()
                IN.param2 = {a=Type:Float(), b=Type:Int32()}

            end
        )", {}, "intf name", m_errorReporting);

        LuaScript* inputsScript = m_apiObjects.createLuaScript(R"LUA_SCRIPT(
        function interface(IN,OUT)

            IN.param1 = Type:Int32()
            IN.param21 = Type:Float()
            IN.param22 = Type:Int32()

        end

        function run(IN,OUT)
        end
        )LUA_SCRIPT", {}, "inputs script", m_errorReporting);

        auto* output1  = intf->getOutputs()->getChild(0);
        auto* output21 = intf->getOutputs()->getChild(1)->getChild(0);
        auto* output22 = intf->getOutputs()->getChild(1)->getChild(1);

        m_apiObjects.getLogicNodeDependencies().link(output1->impl(), inputsScript->getInputs()->getChild(0)->impl(), false, m_errorReporting);
        m_apiObjects.getLogicNodeDependencies().link(output21->impl(), inputsScript->getInputs()->getChild(1)->impl(), false, m_errorReporting);
        m_apiObjects.getLogicNodeDependencies().link(output22->impl(), inputsScript->getInputs()->getChild(2)->impl(), false, m_errorReporting);

        ValidationReportImpl validationResults;
        m_apiObjects.validateInterfaces(validationResults);
        EXPECT_FALSE(validationResults.hasIssue());
    }

    TEST_P(AnApiObjects, ValidatesThatLuaInterfacesNamesAreUnique)
    {
        // single interface -> no warning
        const auto* intf1 = createInterface();
        {
            ValidationReportImpl validationResults;
            m_apiObjects.validateInterfaces(validationResults);
            EXPECT_FALSE(validationResults.hasIssue());
        }

        // two interfaces with same name -> error
        auto intf2 = createInterface();
        {
            ValidationReportImpl validationResults;
            m_apiObjects.validateInterfaces(validationResults);
            ASSERT_EQ(1u, validationResults.getIssues().size());
            EXPECT_EQ(validationResults.getIssues().front().message, "Interface [intf] does not have a unique name");
            EXPECT_EQ(validationResults.getIssues().front().object, intf1);
            EXPECT_EQ(validationResults.getIssues().front().type, EIssueType::Error);
        }

        // rename conflicting intf -> no warning
        intf2->setName("otherIntf");
        {
            ValidationReportImpl validationResults;
            m_apiObjects.validateInterfaces(validationResults);
            EXPECT_FALSE(validationResults.hasIssue());
        }

        // another interface with same name -> error
        auto intf3 = createInterface();
        intf3->setName("otherIntf");
        {
            ValidationReportImpl validationResults;
            m_apiObjects.validateInterfaces(validationResults);
            ASSERT_EQ(1u, validationResults.getIssues().size());
            EXPECT_EQ(validationResults.getIssues().front().message, "Interface [otherIntf] does not have a unique name");
            EXPECT_EQ(validationResults.getIssues().front().object, intf2);
            EXPECT_EQ(validationResults.getIssues().front().type, EIssueType::Error);
        }
    }

    TEST_P(AnApiObjects, ValidatesDanglingNodes_ProducesWarningIfNodeHasNoIncomingOrOutgoingLinks)
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

        ValidationReportImpl validationResults;
        m_apiObjects.validateDanglingNodes(validationResults);
        EXPECT_EQ(2u, validationResults.getIssues().size());
        EXPECT_THAT(validationResults.getIssues()[0].message, ::testing::HasSubstr("Node [script name] has no outgoing links"));
        EXPECT_THAT(validationResults.getIssues()[1].message, ::testing::HasSubstr("Node [script name] has no incoming links"));
        EXPECT_THAT(validationResults.getIssues(), ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_P(AnApiObjects, ValidatesDanglingNodes_DoesNotProduceWarningIfNodeHasNoInputs)
    {
        auto* script = m_apiObjects.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.param1 = Type:Int32()
            end
            function run(IN,OUT)
            end
            )", {}, "script name", m_errorReporting);
        ASSERT_NE(nullptr, script);

        auto* dummyInputScript = m_apiObjects.createLuaScript(R"LUA_SCRIPT(
            function interface(IN)
                IN.param1 = Type:Int32()
            end

            function run(IN,OUT)
            end
            )LUA_SCRIPT", {}, "dummy script", m_errorReporting);
        ASSERT_NE(nullptr, dummyInputScript);

        // link script's output in order to pass outputs validation
        m_apiObjects.getLogicNodeDependencies().link(script->getOutputs()->getChild(0u)->impl(), dummyInputScript->getInputs()->getChild(0u)->impl(), false, m_errorReporting);

        ValidationReportImpl validationResults;
        m_apiObjects.validateDanglingNodes(validationResults);
        EXPECT_FALSE(validationResults.hasIssue());
    }

    TEST_P(AnApiObjects, ValidatesDanglingNodes_DoesNotProduceWarningIfNodeHasNoOutputs)
    {
        auto* script = m_apiObjects.createLuaScript(R"(
            function interface(IN,OUT)
                IN.param1 = Type:Int32()
            end
            function run(IN,OUT)
            end
            )", {}, "script name", m_errorReporting);
        ASSERT_NE(nullptr, script);

        auto* dummyOutputScript = m_apiObjects.createLuaScript(R"LUA_SCRIPT(
            function interface(IN,OUT)
                OUT.param1 = Type:Int32()
            end

            function run(IN,OUT)
            end
            )LUA_SCRIPT", {}, "dummy script", m_errorReporting);
        ASSERT_NE(nullptr, dummyOutputScript);

        // link script's input in order to pass inputs validation
        m_apiObjects.getLogicNodeDependencies().link(dummyOutputScript->getOutputs()->getChild(0u)->impl(), script->getInputs()->getChild(0u)->impl(), false, m_errorReporting);

        ValidationReportImpl validationResults;
        m_apiObjects.validateDanglingNodes(validationResults);
        EXPECT_FALSE(validationResults.hasIssue());
    }

    class AnApiObjects_Serialization : public AnApiObjects
    {
    };

    RAMSES_INSTANTIATE_LATEST_FEATURELEVEL_ONLY_TEST_SUITE(AnApiObjects_Serialization);

    TEST_P(AnApiObjects_Serialization, AlwaysCreatesEmptyFlatbuffersContainers_WhenNoObjectsPresent)
    {
        // Create without API objects -> serialize
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize(GetParam(), m_scene->impl());
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
    }

    TEST_P(AnApiObjects_Serialization, CreatesFlatbufferContainer_ForScripts)
    {
        // Create test flatbuffer with only a script
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize{ GetParam(), m_scene->impl() };
            createScript(toSerialize, m_valid_empty_script);
            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::SourceAndByteCode);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        ASSERT_NE(nullptr, serialized.luaScripts());
        ASSERT_EQ(1u, serialized.luaScripts()->size());
        const rlogic_serialization::LuaScript& serializedScript = *serialized.luaScripts()->Get(0);
        EXPECT_EQ("script", serializedScript.base()->name()->str());
        EXPECT_EQ(9u, serializedScript.base()->id());
        EXPECT_EQ(m_valid_empty_script, serializedScript.luaSourceCode()->str());
        EXPECT_TRUE(serializedScript.luaByteCode()->size() > 0);

        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "test", m_errorReporting, GetParam());
        EXPECT_TRUE(deserialized);
    }

    TEST_P(AnApiObjects_Serialization, CreatesFlatbufferContainer_ForInterfaces)
    {
        // Create test flatbuffer with only a script
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize{ GetParam(), m_scene->impl() };
            createInterface(toSerialize);
            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        ASSERT_NE(nullptr, serialized.luaInterfaces());
        ASSERT_EQ(1u, serialized.luaInterfaces()->size());
        const rlogic_serialization::LuaInterface& serializedInterface = *serialized.luaInterfaces()->Get(0);
        EXPECT_EQ("intf", serializedInterface.base()->name()->str());
        EXPECT_EQ(9u, serializedInterface.base()->id());

        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "test", m_errorReporting, GetParam());
        EXPECT_TRUE(deserialized);
    }

    TEST_P(AnApiObjects_Serialization, CreatesFlatbufferContainers_ForBindings)
    {
        // Create test flatbuffer with only a node binding
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize{ GetParam(), m_scene->impl() };
            toSerialize.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
            toSerialize.createAppearanceBinding(*m_appearance, "appearance");
            toSerialize.createCameraBinding(*m_camera, true, "camera");
            toSerialize.createRenderPassBinding(*m_renderPass, "rp");
            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        ASSERT_NE(nullptr, serialized.nodeBindings());
        ASSERT_EQ(1u, serialized.nodeBindings()->size());
        const rlogic_serialization::NodeBinding& serializedNodeBinding = *serialized.nodeBindings()->Get(0);
        EXPECT_EQ("node", serializedNodeBinding.base()->base()->name()->str());
        EXPECT_EQ(9u, serializedNodeBinding.base()->base()->id());

        ASSERT_NE(nullptr, serialized.appearanceBindings());
        ASSERT_EQ(1u, serialized.appearanceBindings()->size());
        const rlogic_serialization::AppearanceBinding& serializedAppBinding = *serialized.appearanceBindings()->Get(0);
        EXPECT_EQ("appearance", serializedAppBinding.base()->base()->name()->str());
        EXPECT_EQ(10u, serializedAppBinding.base()->base()->id());

        ASSERT_NE(nullptr, serialized.cameraBindings());
        ASSERT_EQ(1u, serialized.cameraBindings()->size());
        const rlogic_serialization::CameraBinding& serializedCameraBinding = *serialized.cameraBindings()->Get(0);
        EXPECT_EQ("camera", serializedCameraBinding.base()->base()->name()->str());
        EXPECT_EQ(11u, serializedCameraBinding.base()->base()->id());

        ASSERT_NE(nullptr, serialized.renderPassBindings());
        ASSERT_EQ(1u, serialized.renderPassBindings()->size());
        const rlogic_serialization::RenderPassBinding& serializedRenderPassBinding = *serialized.renderPassBindings()->Get(0);
        EXPECT_EQ("rp", serializedRenderPassBinding.base()->base()->name()->str());
        EXPECT_EQ(12u, serializedRenderPassBinding.base()->base()->id());
    }

    TEST_P(AnApiObjects_Serialization, CreatesFlatbufferContainers_ForLinks)
    {
        // Create test flatbuffer with a link between script and binding
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize{ GetParam(), m_scene->impl() };

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

            LuaScript* script = createScript(toSerialize, scriptWithOutput);
            NodeBinding* nodeBinding = toSerialize.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
            ASSERT_TRUE(toSerialize.getLogicNodeDependencies().link(
                script->getOutputs()->getChild("nested")->getChild("rotation")->impl(),
                nodeBinding->getInputs()->getChild("rotation")->impl(),
                false,
                m_errorReporting));
            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        // Asserts both script and binding objects existence
        ASSERT_EQ(1u, serialized.luaScripts()->size());
        ASSERT_EQ(1u, serialized.nodeBindings()->size());
        const rlogic_serialization::LuaScript& script = *serialized.luaScripts()->Get(0);
        const rlogic_serialization::NodeBinding& binding = *serialized.nodeBindings()->Get(0);

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
            ApiObjects toSerialize{ GetParam(), m_scene->impl() };
            createScript(toSerialize, m_valid_empty_script);
            createInterface(toSerialize);
            const auto* nodeBinding = toSerialize.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
            auto* appearanceBinding = toSerialize.createAppearanceBinding(*m_appearance, "appearance");
            toSerialize.createCameraBinding(*m_camera, true, "camera");
            toSerialize.createRenderPassBinding(*m_renderPass, "rp");
            RenderGroupBindingElements elements;
            EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
            toSerialize.createRenderGroupBinding(*m_renderGroup, elements, "rg");
            createSkinBinding(*nodeBinding, *appearanceBinding, toSerialize);
            toSerialize.createMeshNodeBinding(*m_meshNode, "mb");
            toSerialize.createRenderBufferBinding(*m_renderBuffer, "rb");

            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        EXPECT_CALL(m_resolverMock, findRamsesNodeInScene(::testing::Eq("node"), m_node->getSceneObjectId())).WillOnce(::testing::Return(m_node));
        EXPECT_CALL(m_resolverMock, findRamsesAppearanceInScene(::testing::Eq("appearance"), m_appearance->getSceneObjectId())).WillOnce(::testing::Return(m_appearance));
        EXPECT_CALL(m_resolverMock, findRamsesCameraInScene(::testing::Eq("camera"), m_camera->getSceneObjectId())).WillOnce(::testing::Return(m_camera));
        EXPECT_CALL(m_resolverMock, findRamsesRenderPassInScene(::testing::Eq("rp"), m_renderPass->getSceneObjectId())).WillOnce(::testing::Return(m_renderPass));
        EXPECT_CALL(m_resolverMock, findRamsesRenderGroupInScene(::testing::Eq("rg"), m_renderGroup->getSceneObjectId())).WillOnce(::testing::Return(m_renderGroup));
        EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("rg"), m_meshNode->getSceneObjectId())).WillOnce(::testing::Return(m_meshNode));
        EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("mb"), m_meshNode->getSceneObjectId())).WillOnce(::testing::Return(m_meshNode));
        EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("rb"), m_renderBuffer->getSceneObjectId())).WillOnce(::testing::Return(m_renderBuffer));
        std::unique_ptr<ApiObjects> apiObjectsOptional = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "", m_errorReporting, GetParam());

        ASSERT_TRUE(apiObjectsOptional);

        ApiObjects& apiObjects = *apiObjectsOptional;

        ASSERT_EQ(10u, apiObjects.getApiObjectOwningContainer().size());

        LuaScript* script = apiObjects.getApiObjectContainer<LuaScript>()[0];
        EXPECT_EQ(script->getName(), "script");
        EXPECT_EQ(script, &script->impl().getLogicObject());

        LuaInterface* intf = apiObjects.getApiObjectContainer<LuaInterface>()[0];
        EXPECT_EQ(intf->getName(), "intf");
        EXPECT_EQ(intf, &intf->impl().getLogicObject());

        NodeBinding* nodeBinding = apiObjects.getApiObjectContainer<NodeBinding>()[0];
        EXPECT_EQ(nodeBinding->getName(), "node");
        EXPECT_EQ(nodeBinding, &nodeBinding->impl().getLogicObject());

        AppearanceBinding* appBinding = apiObjects.getApiObjectContainer<AppearanceBinding>()[0];
        EXPECT_EQ(appBinding->getName(), "appearance");
        EXPECT_EQ(appBinding, &appBinding->impl().getLogicObject());

        CameraBinding* camBinding = apiObjects.getApiObjectContainer<CameraBinding>()[0];
        EXPECT_EQ(camBinding->getName(), "camera");
        EXPECT_EQ(camBinding, &camBinding->impl().getLogicObject());

        RenderPassBinding* rpBinding = apiObjects.getApiObjectContainer<RenderPassBinding>()[0];
        EXPECT_EQ(rpBinding->getName(), "rp");
        EXPECT_EQ(rpBinding, &rpBinding->impl().getLogicObject());

        const auto* rgBinding = apiObjects.getApiObjectContainer<RenderGroupBinding>()[0];
        EXPECT_EQ(rgBinding->getName(), "rg");
        EXPECT_EQ(rgBinding, &rgBinding->impl().getLogicObject());

        const auto* skin = apiObjects.getApiObjectContainer<SkinBinding>()[0];
        EXPECT_EQ(skin->getName(), "skin");
        EXPECT_EQ(skin, &skin->impl().getLogicObject());

        const auto* meshBinding = apiObjects.getApiObjectContainer<MeshNodeBinding>()[0];
        EXPECT_EQ(meshBinding->getName(), "mb");
        EXPECT_EQ(meshBinding, &meshBinding->impl().getLogicObject());

        const auto* rbBinding = apiObjects.getApiObjectContainer<RenderBufferBinding>()[0];
        EXPECT_EQ(rbBinding->getName(), "rb");
        EXPECT_EQ(rbBinding, &rbBinding->impl().getLogicObject());
    }

    TEST_P(AnApiObjects_Serialization, ObjectsCreatedAfterLoadingReceiveUniqueId)
    {
        std::unordered_set<sceneObjectId_t> ids;

        // Create dummy data and serialize
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects beforeSaving{ GetParam(), m_scene->impl() };
            ids.insert(createScript(beforeSaving, m_valid_empty_script)->getSceneObjectId());
            ids.insert(createScript(beforeSaving, m_valid_empty_script)->getSceneObjectId());
            ids.insert(createScript(beforeSaving, m_valid_empty_script)->getSceneObjectId());
            EXPECT_EQ(3u, ids.size());

            ApiObjects::Serialize(beforeSaving, builder, ELuaSavingMode::ByteCodeOnly);
        }

        auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        std::unique_ptr<ApiObjects> afterLoadingObjects = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "", m_errorReporting, GetParam());

        auto* newScript = createScript(*afterLoadingObjects, m_valid_empty_script);
        // new script's ID does not overlap with one of the IDs of the objects before saving
        EXPECT_EQ(ids.cend(), ids.find(newScript->getSceneObjectId()));
    }

    TEST_P(AnApiObjects_Serialization, ReConstructsLinksWhenCreatedFromDeserializedData)
    {
        // Create dummy data and serialize
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize{ GetParam(), m_scene->impl() };

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
                script1->getOutputs()->getChild("nested")->getChild("integer")->impl(),
                script2->getInputs()->getChild("integer")->impl(),
                false,
                m_errorReporting));

            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        std::unique_ptr<ApiObjects> apiObjectsOptional = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "", m_errorReporting, GetParam());

        ASSERT_TRUE(apiObjectsOptional);

        ApiObjects& apiObjects = *apiObjectsOptional;

        LuaScript* script1 = apiObjects.getApiObjectContainer<LuaScript>()[0];
        ASSERT_TRUE(script1);

        LuaScript* script2 = apiObjects.getApiObjectContainer<LuaScript>()[1];
        ASSERT_TRUE(script2);

        EXPECT_TRUE(apiObjects.getLogicNodeDependencies().isLinked(script1->impl()));
        EXPECT_TRUE(apiObjects.getLogicNodeDependencies().isLinked(script2->impl()));

        PropertyImpl* script1Output = &script1->getOutputs()->getChild("nested")->getChild("integer")->impl();
        PropertyImpl* script2Input = &script2->getInputs()->getChild("integer")->impl();
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
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing Lua modules container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenScriptsContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                0, // no scripts container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing Lua scripts container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenInterfacesContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                0, // no interfaces container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing Lua interfaces container!");
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
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing node bindings container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenAppearanceBindingsContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                0, // no appearance bindings container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing appearance bindings container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenCameraBindingsContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                0, // no camera bindings container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing camera bindings container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenRenderPassBindingsContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                0u, // no render pass bindings container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing renderpass bindings container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenLinksContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                0, // no links container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing links container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenDataArrayContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                0, // no data array container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing data arrays container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenAnimationNodeContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                0, // no animation nodes container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing animation nodes container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenTimerNodeContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                0, // no timer nodes container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing timer nodes container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenAnchorPointContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                0u, // no anchor points container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing anchor points container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenRenderGroupBindingContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                0u, // no render group bindings container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing rendergroup bindings container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenSkinBindingContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                0u, // no skin bindings container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing skin bindings container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenMeshNodeBindingContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                0u, // no mesh node bindings container
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing meshnode bindings container!");
    }

    TEST_P(AnApiObjects_Serialization, ErrorWhenRenderBufferBindingContainerMissing)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                0u // no render buffer bindings container
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading from serialized data: missing render buffer bindings container!");
    }

    TEST_P(AnApiObjects_Serialization, ReportsErrorWhenScriptCouldNotBeDeserialized)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{ m_testUtils.serializeTestScriptWithError() }),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of LuaScript from serialized data: missing name and/or ID!");
    }

    TEST_P(AnApiObjects_Serialization, ReportsErrorWhenInterfaceCouldNotBeDeserialized)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{m_testUtils.serializeTestInterfaceWithError()}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of LuaInterface from serialized data: empty name!");
    }

    TEST_P(AnApiObjects_Serialization, ReportsErrorWhenModuleCouldNotBeDeserialized)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{ m_testUtils.serializeTestModule(true) }), // module has errors
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of LuaModule from serialized data: missing name and/or ID!");
    }

    TEST_P(AnApiObjects_Serialization, ReportsErrorWhenRenderPassBindingCouldNotBeDeserialized)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{ m_testUtils.serializeTestRenderPassBindingWithError() }),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderPassBinding from serialized data: missing base class info!");
    }

    TEST_P(AnApiObjects_Serialization, ReportsErrorWhenRenderGroupBindingCouldNotBeDeserialized)
    {
        {
            auto apiObjects = rlogic_serialization::CreateApiObjects(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::Link>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>>{ m_testUtils.serializeTestRenderGroupBindingWithError() }),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>>{})
            );
            m_flatBufferBuilder.Finish(apiObjects);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "unit test", m_errorReporting, GetParam());

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of RenderGroupBinding from serialized data: missing base class info!");
    }

    TEST_P(AnApiObjects_Serialization, FillsLogicObjectAndOwnedContainerOnDeserialization)
    {
        // Create dummy data and serialize
        flatbuffers::FlatBufferBuilder builder;
        {
            ApiObjects toSerialize{ GetParam(), m_scene->impl() };
            toSerialize.createLuaModule(m_moduleSrc, {}, "module", m_errorReporting);
            createScript(toSerialize, m_valid_empty_script);
            createInterface(toSerialize);
            auto* node = toSerialize.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "node");
            auto* appearance = toSerialize.createAppearanceBinding(*m_appearance, "appearance");
            auto* camera = toSerialize.createCameraBinding(*m_camera, true, "camera");
            auto dataArray = toSerialize.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "data");
            AnimationNodeConfig config;
            config.addChannel({ "channel", dataArray, dataArray, EInterpolationType::Linear });
            toSerialize.createAnimationNode(config.impl(), "animNode");
            toSerialize.createTimerNode("timerNode");
            toSerialize.createRenderPassBinding(*m_renderPass, "rp");
            toSerialize.createAnchorPoint(node->impl(), camera->impl(), "anchor");
            RenderGroupBindingElements elements;
            EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
            toSerialize.createRenderGroupBinding(*m_renderGroup, elements, "rg");
            createSkinBinding(*node, *appearance, toSerialize);
            toSerialize.createMeshNodeBinding(*m_meshNode, "mb");
            toSerialize.createRenderBufferBinding(*m_renderBuffer, "rb");

            ApiObjects::Serialize(toSerialize, builder, ELuaSavingMode::ByteCodeOnly);
        }

        auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::ApiObjects>(builder.GetBufferPointer());

        EXPECT_CALL(m_resolverMock, findRamsesNodeInScene(::testing::Eq("node"), m_node->getSceneObjectId())).WillOnce(::testing::Return(m_node));
        EXPECT_CALL(m_resolverMock, findRamsesAppearanceInScene(::testing::Eq("appearance"), m_appearance->getSceneObjectId())).WillOnce(::testing::Return(m_appearance));
        EXPECT_CALL(m_resolverMock, findRamsesCameraInScene(::testing::Eq("camera"), m_camera->getSceneObjectId())).WillOnce(::testing::Return(m_camera));
        EXPECT_CALL(m_resolverMock, findRamsesRenderPassInScene(::testing::Eq("rp"), m_renderPass->getSceneObjectId())).WillOnce(::testing::Return(m_renderPass));
        EXPECT_CALL(m_resolverMock, findRamsesRenderGroupInScene(::testing::Eq("rg"), m_renderGroup->getSceneObjectId())).WillOnce(::testing::Return(m_renderGroup));
        EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("rg"), m_meshNode->getSceneObjectId())).WillOnce(::testing::Return(m_meshNode));
        EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("mb"), m_meshNode->getSceneObjectId())).WillOnce(::testing::Return(m_meshNode));
        EXPECT_CALL(m_resolverMock, findRamsesSceneObjectInScene(::testing::Eq("rb"), m_renderBuffer->getSceneObjectId())).WillOnce(::testing::Return(m_renderBuffer));
        std::unique_ptr<ApiObjects> deserialized = ApiObjects::Deserialize(m_scene->impl(), serialized, m_resolverMock, "", m_errorReporting, GetParam());

        ASSERT_TRUE(deserialized);

        ApiObjects& apiObjects = *deserialized;

        const ApiObjectContainer<LogicObject>& logicObjects = apiObjects.getApiObjectContainer<LogicObject>();
        const ApiObjectOwningContainer& ownedObjects = apiObjects.getApiObjectOwningContainer();

        std::vector<LogicObject*> expected;
        expected = std::vector<LogicObject*>{
            apiObjects.getApiObjectContainer<LuaModule>()[0],
            apiObjects.getApiObjectContainer<LuaScript>()[0],
            apiObjects.getApiObjectContainer<LuaInterface>()[0],
            apiObjects.getApiObjectContainer<NodeBinding>()[0],
            apiObjects.getApiObjectContainer<AppearanceBinding>()[0],
            apiObjects.getApiObjectContainer<CameraBinding>()[0],
            apiObjects.getApiObjectContainer<RenderPassBinding>()[0],
            apiObjects.getApiObjectContainer<DataArray>()[0],
            apiObjects.getApiObjectContainer<AnimationNode>()[0],
            apiObjects.getApiObjectContainer<TimerNode>()[0],
            apiObjects.getApiObjectContainer<AnchorPoint>()[0],
            apiObjects.getApiObjectContainer<RenderGroupBinding>()[0],
            apiObjects.getApiObjectContainer<SkinBinding>()[0],
            apiObjects.getApiObjectContainer<MeshNodeBinding>()[0],
            apiObjects.getApiObjectContainer<RenderBufferBinding>()[0]
        };

        ASSERT_EQ(expected.size(), logicObjects.size());
        ASSERT_EQ(logicObjects.size(), ownedObjects.size());
        for (size_t i = 0; i < expected.size(); ++i)
        {
            EXPECT_EQ(logicObjects[i], expected[i]);
            EXPECT_EQ(ownedObjects[i].get(), expected[i]);
            EXPECT_EQ(logicObjects[i], &logicObjects[i]->impl().getLogicObject());
        }
    }
}
