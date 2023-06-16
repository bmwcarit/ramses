//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

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
#include "ramses-client-api/UniformInput.h"

#include "RamsesTestUtils.h"
#include "FeatureLevelTestValues.h"

namespace ramses::internal
{
    class ALogicEngine_SerializedSize : public ALogicEngineBase, public ::testing::TestWithParam<ramses::EFeatureLevel>
    {
    protected:
        ALogicEngine_SerializedSize()
        {
            m_renderGroup->addMeshNode(*m_meshNode);
        }

        LogicEngine m_logicEngine{ GetParam() };

        RamsesTestSetup m_ramses;
        ramses::Scene* m_scene = { m_ramses.createScene() };
        ramses::Node* m_node = { m_scene->createNode() };
        ramses::PerspectiveCamera* m_camera = { m_scene->createPerspectiveCamera() };
        ramses::Appearance* m_appearance = { &RamsesTestSetup::CreateTrivialTestAppearance(*m_scene) };
        ramses::RenderPass* m_renderPass = { m_scene->createRenderPass() };
        ramses::RenderGroup* m_renderGroup = { m_scene->createRenderGroup() };
        ramses::MeshNode* m_meshNode = { m_scene->createMeshNode("meshNode") };

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

        const std::string_view m_module_source_code = R"(
            local mymath = {}
            function mymath.add(a,b)
                return a+b
            end
            mymath.PI=3.1415
            return mymath
        )";

        const std::string_view m_script_source_code = R"(
            function interface(IN,OUT)
                IN.a = Type:Float()
                IN.b = Type:Float()
                OUT.value = Type:Float()
            end

            function run(IN,OUT)
                OUT.value = IN.a + IN.b
            end
        )";

        const std::string_view m_script_with_module_source_code = R"(
            modules("mymath")

            function interface(IN,OUT)
                OUT.v = Type:Int32()
                OUT.pi = Type:Float()
            end

            function run(IN,OUT)
                OUT.v = mymath.add(1,2)
                OUT.pi = mymath.PI
            end
        )";

        LuaScript* createScript(std::string_view source)
        {
            auto script = m_logicEngine.createLuaScript(source, {}, "script");
            EXPECT_NE(nullptr, script);
            return script;
        }

        LuaInterface* createInterface()
        {
            auto intf = m_logicEngine.createLuaInterface(m_valid_empty_interface, "intf");
            EXPECT_NE(nullptr, intf);
            return intf;
        }

        SkinBinding* createSkinBinding()
        {
            const auto node = m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "nodeForSkin");
            const auto appearance = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appearanceForSkin");
            ramses::UniformInput uniform;
            appearance->getRamsesAppearance().getEffect().findUniformInput("jointMat", uniform);
            EXPECT_TRUE(uniform.isValid());
            return m_logicEngine.createSkinBinding({ node }, { matrix44f{ 0.f } }, *appearance, uniform, "skin");
        }
    };

    static constexpr size_t EmptySerializedSizeTotal{ 164u };

    INSTANTIATE_TEST_SUITE_P(
        ALogicEngine_SerializedSizeTests,
        ALogicEngine_SerializedSize,
        ramses::internal::GetFeatureLevelTestValues());

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithoutContent)
    {
        EXPECT_EQ(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<LogicObject>(), EmptySerializedSizeTotal);

        EXPECT_EQ(this->m_logicEngine.getSerializedSize<AnchorPoint>(), 0u);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<AnimationNode>(), 0u);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<LuaInterface>(), 0u);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<LuaModule>(), 0u);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<LuaScript>(), 0u);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<RamsesAppearanceBinding>(), 0u);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<RamsesCameraBinding>(), 0u);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<RamsesNodeBinding>(), 0u);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<RamsesRenderPassBinding>(), 0u);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<RamsesRenderGroupBinding>(), 0u);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<RamsesMeshNodeBinding>(), 0u);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<TimerNode>(), 0u);
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<SkinBinding>(), 0u);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithInterface)
    {
        createInterface();
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<LuaInterface>(), 112u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithModule)
    {
        this->m_logicEngine.createLuaModule(m_module_source_code, {}, "module");
        const auto result = this->m_logicEngine.getSerializedSize<LuaModule>(ELuaSavingMode::SourceCodeOnly);
        EXPECT_EQ(result, 260u);
    }

    // TODO figure out how to deal with difference in exported size on different platforms
    // byte code has different size on 32bit arch (currently known)
    TEST_P(ALogicEngine_SerializedSize, DISABLED_ChecksSerializedSizeWithModule_withByteCode)
    {
        this->m_logicEngine.createLuaModule(m_module_source_code, {}, "module");

        auto result = this->m_logicEngine.getSerializedSize<LuaModule>(ELuaSavingMode::ByteCodeOnly);
        EXPECT_EQ(result, 366u);

        result = this->m_logicEngine.getSerializedSize<LuaModule>(ELuaSavingMode::SourceAndByteCode);
        EXPECT_EQ(result, 550u);

        // default is source and bytecode
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<LuaModule>(),
            this->m_logicEngine.getSerializedSize<LuaModule>(ELuaSavingMode::SourceAndByteCode));
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithEmptyScript)
    {
        createScript(m_valid_empty_script);
        const auto result = this->m_logicEngine.getSerializedSize<LuaScript>(ELuaSavingMode::SourceCodeOnly);
        EXPECT_EQ(result, 312u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(ELuaSavingMode::SourceCodeOnly), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithScript)
    {
        this->m_logicEngine.createLuaScript(m_script_source_code, {}, "script");
        const auto result = this->m_logicEngine.getSerializedSize<LuaScript>(ELuaSavingMode::SourceCodeOnly);
        EXPECT_EQ(result, 616u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(ELuaSavingMode::SourceCodeOnly), EmptySerializedSizeTotal);
    }

    // TODO figure out how to deal with difference in exported size on different platforms
    // byte code has different size on 32bit arch (currently known)
    TEST_P(ALogicEngine_SerializedSize, DISABLED_ChecksSerializedSizeWithScript_withByteCode)
    {
        this->m_logicEngine.createLuaScript(m_script_source_code, {}, "script");

        auto result = this->m_logicEngine.getSerializedSize<LuaScript>(ELuaSavingMode::ByteCodeOnly);
        EXPECT_EQ(result, 912u);

        result = this->m_logicEngine.getSerializedSize<LuaScript>(ELuaSavingMode::SourceAndByteCode);
        EXPECT_EQ(result, 1184u);

        // default is source and bytecode
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<LuaScript>(),
            this->m_logicEngine.getSerializedSize<LuaScript>(ELuaSavingMode::SourceAndByteCode));
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithScriptAndModuleDependency)
    {
        LuaConfig config;
        const auto module = this->m_logicEngine.createLuaModule(m_module_source_code, config, "module");
        config.addDependency("mymath", *module);
        this->m_logicEngine.createLuaScript(m_script_with_module_source_code, config, "script");

        const auto result = this->m_logicEngine.getSerializedSize<LuaScript>(ELuaSavingMode::SourceCodeOnly);
        EXPECT_EQ(result, 624u);
    }

    // TODO figure out how to deal with difference in exported size on different platforms
    // byte code has different size on 32bit arch (currently known)
    TEST_P(ALogicEngine_SerializedSize, DISABLED_ChecksSerializedSizeWithScriptAndModuleDependency_withByteCode)
    {
        LuaConfig config;
        const auto module = this->m_logicEngine.createLuaModule(m_module_source_code, config, "module");
        config.addDependency("mymath", *module);
        this->m_logicEngine.createLuaScript(m_script_with_module_source_code, config, "script");

        auto result = this->m_logicEngine.getSerializedSize<LuaScript>(ELuaSavingMode::ByteCodeOnly);
        EXPECT_EQ(result, 1000u);

        result = this->m_logicEngine.getSerializedSize<LuaScript>(ELuaSavingMode::SourceAndByteCode);
        EXPECT_EQ(result, 1304u);

        // default is source and bytecode
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<LuaScript>(),
            this->m_logicEngine.getSerializedSize<LuaScript>(ELuaSavingMode::SourceAndByteCode));
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithNodeBinding)
    {
        this->m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "node");
        const auto result = this->m_logicEngine.getSerializedSize<RamsesNodeBinding>();
        EXPECT_EQ(result, 440u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithAppearanceBinding)
    {
        this->m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appearance");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<RamsesAppearanceBinding>(), 256u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithCameraBinding)
    {
        this->m_logicEngine.createRamsesCameraBinding(*m_camera, "camera");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<RamsesCameraBinding>(), 632u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithCameraBindingWithFrustumPlanes)
    {
        this->m_logicEngine.createRamsesCameraBindingWithFrustumPlanes(*m_camera, "camera");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<RamsesCameraBinding>(), 728u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithRenderPassBinding)
    {
        this->m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderpass");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<RamsesRenderPassBinding>(), 376u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithRenderGroupBinding)
    {
        RamsesRenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(*m_meshNode));
        this->m_logicEngine.createRamsesRenderGroupBinding(*m_renderGroup, elements, "rg");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<RamsesRenderGroupBinding>(), 336u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithMeshNodeBinding)
    {
        this->m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "mb");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<RamsesMeshNodeBinding>(), 376u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    // Since there is a template specialization for animations collecting data arrays
    // we test with different data arrays and reusing one single data array.
    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithLinearAnimation_differentArrays)
    {
        // Test animation channel with different data arrays
        auto dataArray1 = this->m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data1");
        auto dataArray2 = this->m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data2");
        ASSERT_NE(nullptr, dataArray1);
        ASSERT_NE(nullptr, dataArray2);

        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray1, dataArray2, EInterpolationType::Linear });
        this->m_logicEngine.createAnimationNode(config, "animation");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<AnimationNode>(), 378u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithLinearAnimation_sameArray)
    {
        // Test animation channel with the same data arrays
        auto data = this->m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        ASSERT_NE(nullptr, data);

        AnimationNodeConfig config;
        config.addChannel({ "channel", data, data, EInterpolationType::Linear });
        this->m_logicEngine.createAnimationNode(config, "animation");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<AnimationNode>(), 378u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    // Since there is a template specialization for animations collecting data arrays
    // we test with different data arrays and reusing one single data array.
    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithCubicAnimation_differentArrays)
    {
        // Test animation channel with different data arrays
        auto dataArray1 = this->m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data1");
        auto dataArray2 = this->m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data2");
        auto dataArray3 = this->m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data3");
        auto dataArray4 = this->m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data4");
        ASSERT_NE(nullptr, dataArray1);
        ASSERT_NE(nullptr, dataArray2);
        ASSERT_NE(nullptr, dataArray3);
        ASSERT_NE(nullptr, dataArray4);

        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray1, dataArray2, EInterpolationType::Cubic, dataArray3, dataArray4 });
        this->m_logicEngine.createAnimationNode(config, "animation");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<AnimationNode>(), 378u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithCubicAnimation_sameArray)
    {
        // Test animation channel with the same data arrays
        auto data = this->m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        ASSERT_NE(nullptr, data);

        AnimationNodeConfig config;
        config.addChannel({ "channel", data, data, EInterpolationType::Cubic, data, data });
        this->m_logicEngine.createAnimationNode(config, "animation");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<AnimationNode>(), 378u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithDataArray)
    {
        this->m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<DataArray>(), 98u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithTimer)
    {
        this->m_logicEngine.createTimerNode("timer");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<TimerNode>(), 290u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithAnchorPoint)
    {
        const auto node = this->m_logicEngine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "node");
        const auto camera = this->m_logicEngine.createRamsesCameraBinding(*m_camera, "camera");
        ASSERT_TRUE(node && camera);
        this->m_logicEngine.createAnchorPoint(*node, *camera, "timer");
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<AnchorPoint>(), 248u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithSkinBinding)
    {
        createSkinBinding();
        EXPECT_EQ(this->m_logicEngine.getSerializedSize<SkinBinding>(), 184u);
        EXPECT_GT(this->m_logicEngine.getTotalSerializedSize(), EmptySerializedSizeTotal);
    }
}
