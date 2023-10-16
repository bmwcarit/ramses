//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

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
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/UniformInput.h"

#include "RamsesTestUtils.h"
#include "FeatureLevelTestValues.h"

namespace ramses::internal
{
    class ALogicEngine_SerializedSize : public ALogicEngineBase, public ::testing::TestWithParam<ramses::EFeatureLevel>
    {
    protected:
        ALogicEngine_SerializedSize()
            : ALogicEngineBase{ GetParam() }
        {
        }

        const std::string_view m_valid_empty_interface = R"(
            function interface(IN,OUT)
            end
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
            auto script = m_logicEngine->createLuaScript(source, {}, "script");
            EXPECT_NE(nullptr, script);
            return script;
        }

        LuaInterface* createInterface()
        {
            auto intf = m_logicEngine->createLuaInterface(m_valid_empty_interface, "intf");
            EXPECT_NE(nullptr, intf);
            return intf;
        }

        SkinBinding* createSkinBinding()
        {
            const auto node = m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "nodeForSkin");
            const auto appearance = m_logicEngine->createAppearanceBinding(*m_appearance, "appearanceForSkin");
            const auto optUniform = appearance->getRamsesAppearance().getEffect().findUniformInput("jointMat");
            EXPECT_TRUE(optUniform.has_value());
            assert(optUniform != std::nullopt);
            return m_logicEngine->createSkinBinding({ node }, { matrix44f{ 0.f } }, *appearance, *optUniform, "skin");
        }
    };

    static constexpr size_t EmptySerializedSizeTotal{ 164u };

    INSTANTIATE_TEST_SUITE_P(
        ALogicEngine_SerializedSizeTests,
        ALogicEngine_SerializedSize,
        ramses::internal::GetFeatureLevelTestValues());

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithoutContent)
    {
        EXPECT_EQ(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<LogicObject>(), EmptySerializedSizeTotal);

        EXPECT_EQ(this->m_logicEngine->getSerializedSize<AnchorPoint>(), 0u);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<AnimationNode>(), 0u);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<LuaInterface>(), 0u);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<LuaModule>(), 0u);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<LuaScript>(), 0u);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<AppearanceBinding>(), 0u);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<CameraBinding>(), 0u);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<NodeBinding>(), 0u);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<RenderPassBinding>(), 0u);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<RenderGroupBinding>(), 0u);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<MeshNodeBinding>(), 0u);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<TimerNode>(), 0u);
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<SkinBinding>(), 0u);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithInterface)
    {
        createInterface();
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<LuaInterface>(), 112u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithModule)
    {
        this->m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "module");
        const auto result = this->m_logicEngine->getSerializedSize<LuaModule>(ELuaSavingMode::SourceCodeOnly);
        EXPECT_EQ(result, 232u);
    }

    // TODO figure out how to deal with difference in exported size on different platforms
    // byte code has different size on 32bit arch (currently known)
    TEST_P(ALogicEngine_SerializedSize, DISABLED_ChecksSerializedSizeWithModule_withByteCode)
    {
        this->m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "module");

        auto result = this->m_logicEngine->getSerializedSize<LuaModule>(ELuaSavingMode::ByteCodeOnly);
        EXPECT_EQ(result, 366u);

        result = this->m_logicEngine->getSerializedSize<LuaModule>(ELuaSavingMode::SourceAndByteCode);
        EXPECT_EQ(result, 550u);

        // default is source and bytecode
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<LuaModule>(),
            this->m_logicEngine->getSerializedSize<LuaModule>(ELuaSavingMode::SourceAndByteCode));
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithEmptyScript)
    {
        createScript(m_valid_empty_script);
        const auto result = this->m_logicEngine->getSerializedSize<LuaScript>(ELuaSavingMode::SourceCodeOnly);
        EXPECT_EQ(result, 312u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(ELuaSavingMode::SourceCodeOnly), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithScript)
    {
        this->m_logicEngine->createLuaScript(m_script_source_code, {}, "script");
        const auto result = this->m_logicEngine->getSerializedSize<LuaScript>(ELuaSavingMode::SourceCodeOnly);
        EXPECT_EQ(result, 616u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(ELuaSavingMode::SourceCodeOnly), EmptySerializedSizeTotal);
    }

    // TODO figure out how to deal with difference in exported size on different platforms
    // byte code has different size on 32bit arch (currently known)
    TEST_P(ALogicEngine_SerializedSize, DISABLED_ChecksSerializedSizeWithScript_withByteCode)
    {
        this->m_logicEngine->createLuaScript(m_script_source_code, {}, "script");

        auto result = this->m_logicEngine->getSerializedSize<LuaScript>(ELuaSavingMode::ByteCodeOnly);
        EXPECT_EQ(result, 912u);

        result = this->m_logicEngine->getSerializedSize<LuaScript>(ELuaSavingMode::SourceAndByteCode);
        EXPECT_EQ(result, 1184u);

        // default is source and bytecode
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<LuaScript>(),
            this->m_logicEngine->getSerializedSize<LuaScript>(ELuaSavingMode::SourceAndByteCode));
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithScriptAndModuleDependency)
    {
        LuaConfig config;
        const auto module = this->m_logicEngine->createLuaModule(m_moduleSourceCode, config, "module");
        config.addDependency("mymath", *module);
        this->m_logicEngine->createLuaScript(m_script_with_module_source_code, config, "script");

        const auto result = this->m_logicEngine->getSerializedSize<LuaScript>(ELuaSavingMode::SourceCodeOnly);
        EXPECT_EQ(result, 624u);
    }

    // TODO figure out how to deal with difference in exported size on different platforms
    // byte code has different size on 32bit arch (currently known)
    TEST_P(ALogicEngine_SerializedSize, DISABLED_ChecksSerializedSizeWithScriptAndModuleDependency_withByteCode)
    {
        LuaConfig config;
        const auto module = this->m_logicEngine->createLuaModule(m_moduleSourceCode, config, "module");
        config.addDependency("mymath", *module);
        this->m_logicEngine->createLuaScript(m_script_with_module_source_code, config, "script");

        auto result = this->m_logicEngine->getSerializedSize<LuaScript>(ELuaSavingMode::ByteCodeOnly);
        EXPECT_EQ(result, 1000u);

        result = this->m_logicEngine->getSerializedSize<LuaScript>(ELuaSavingMode::SourceAndByteCode);
        EXPECT_EQ(result, 1304u);

        // default is source and bytecode
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<LuaScript>(),
            this->m_logicEngine->getSerializedSize<LuaScript>(ELuaSavingMode::SourceAndByteCode));
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithNodeBinding)
    {
        this->m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "node");
        const auto result = this->m_logicEngine->getSerializedSize<NodeBinding>();
        EXPECT_EQ(result, 440u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithAppearanceBinding)
    {
        this->m_logicEngine->createAppearanceBinding(*m_appearance, "appearance");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<AppearanceBinding>(), 256u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithCameraBinding)
    {
        this->m_logicEngine->createCameraBinding(*m_camera, "camera");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<CameraBinding>(), 728u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithCameraBindingWithFrustumPlanes)
    {
        this->m_logicEngine->createCameraBindingWithFrustumPlanes(*m_camera, "camera");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<CameraBinding>(), 728u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithRenderPassBinding)
    {
        this->m_logicEngine->createRenderPassBinding(*m_renderPass, "renderpass");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<RenderPassBinding>(), 376u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithRenderGroupBinding)
    {
        RenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(*m_meshNode));
        this->m_logicEngine->createRenderGroupBinding(*m_renderGroup, elements, "rg");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<RenderGroupBinding>(), 336u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithMeshNodeBinding)
    {
        this->m_logicEngine->createMeshNodeBinding(*m_meshNode, "mb");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<MeshNodeBinding>(), 376u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    // Since there is a template specialization for animations collecting data arrays
    // we test with different data arrays and reusing one single data array.
    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithLinearAnimation_differentArrays)
    {
        // Test animation channel with different data arrays
        auto dataArray1 = this->m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data1");
        auto dataArray2 = this->m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data2");
        ASSERT_NE(nullptr, dataArray1);
        ASSERT_NE(nullptr, dataArray2);

        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray1, dataArray2, EInterpolationType::Linear });
        this->m_logicEngine->createAnimationNode(config, "animation");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<AnimationNode>(), 378u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithLinearAnimation_sameArray)
    {
        // Test animation channel with the same data arrays
        auto data = this->m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        ASSERT_NE(nullptr, data);

        AnimationNodeConfig config;
        config.addChannel({ "channel", data, data, EInterpolationType::Linear });
        this->m_logicEngine->createAnimationNode(config, "animation");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<AnimationNode>(), 378u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    // Since there is a template specialization for animations collecting data arrays
    // we test with different data arrays and reusing one single data array.
    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithCubicAnimation_differentArrays)
    {
        // Test animation channel with different data arrays
        auto dataArray1 = this->m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data1");
        auto dataArray2 = this->m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data2");
        auto dataArray3 = this->m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data3");
        auto dataArray4 = this->m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data4");
        ASSERT_NE(nullptr, dataArray1);
        ASSERT_NE(nullptr, dataArray2);
        ASSERT_NE(nullptr, dataArray3);
        ASSERT_NE(nullptr, dataArray4);

        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray1, dataArray2, EInterpolationType::Cubic, dataArray3, dataArray4 });
        this->m_logicEngine->createAnimationNode(config, "animation");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<AnimationNode>(), 378u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithCubicAnimation_sameArray)
    {
        // Test animation channel with the same data arrays
        auto data = this->m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        ASSERT_NE(nullptr, data);

        AnimationNodeConfig config;
        config.addChannel({ "channel", data, data, EInterpolationType::Cubic, data, data });
        this->m_logicEngine->createAnimationNode(config, "animation");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<AnimationNode>(), 378u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithDataArray)
    {
        this->m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "data");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<DataArray>(), 98u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithTimer)
    {
        this->m_logicEngine->createTimerNode("timer");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<TimerNode>(), 290u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithAnchorPoint)
    {
        const auto node = this->m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "node");
        const auto camera = this->m_logicEngine->createCameraBinding(*m_camera, "camera");
        ASSERT_TRUE(node && camera);
        this->m_logicEngine->createAnchorPoint(*node, *camera, "timer");
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<AnchorPoint>(), 248u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }

    TEST_P(ALogicEngine_SerializedSize, ChecksSerializedSizeWithSkinBinding)
    {
        createSkinBinding();
        EXPECT_EQ(this->m_logicEngine->getSerializedSize<SkinBinding>(), 184u);
        EXPECT_GT(this->m_logicEngine->getTotalSerializedSize(), EmptySerializedSizeTotal);
    }
}
