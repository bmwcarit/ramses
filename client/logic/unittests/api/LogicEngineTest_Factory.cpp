//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"
#include "ramses-logic/RamsesRenderPassBinding.h"
#include "ramses-logic/LuaModule.h"
#include "ramses-logic/AnimationNodeConfig.h"

#include "ramses-client-api/DataFloat.h"

#include "impl/LogicNodeImpl.h"

#include "FeatureLevelTestValues.h"
#include "WithTempDirectory.h"

#include <fmt/format.h>
#include <fstream>

namespace rlogic
{
    // There are more specific "create/destroy" tests in ApiObjects unit tests!
    class ALogicEngine_Factory : public ALogicEngineBase, public ::testing::TestWithParam<EFeatureLevel>
    {
    public:
        ALogicEngine_Factory() : ALogicEngineBase{ GetParam() }
        {
        }

    protected:
        WithTempDirectory tempFolder;
    };

    INSTANTIATE_TEST_SUITE_P(
        ALogicEngine_FactoryTests,
        ALogicEngine_Factory,
        rlogic::internal::GetFeatureLevelTestValues());

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingEmptyScript)
    {
        const LuaScript* script = m_logicEngine.createLuaScript("");
        ASSERT_EQ(nullptr, script);
        EXPECT_FALSE(m_logicEngine.getErrors().empty());
    }

    TEST_P(ALogicEngine_Factory, CreatesScriptFromValidLuaWithoutErrors)
    {
        const LuaScript* script = m_logicEngine.createLuaScript(m_valid_empty_script);
        ASSERT_TRUE(nullptr != script);
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
    }

    TEST_P(ALogicEngine_Factory, DestroysScriptWithoutErrors)
    {
        LuaScript* script = m_logicEngine.createLuaScript(m_valid_empty_script);
        ASSERT_TRUE(script);
        ASSERT_TRUE(m_logicEngine.destroy(*script));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingScriptFromAnotherEngineInstance)
    {
        LogicEngine otherLogicEngine;
        auto script = otherLogicEngine.createLuaScript(m_valid_empty_script);
        ASSERT_TRUE(script);
        ASSERT_FALSE(m_logicEngine.destroy(*script));
        EXPECT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Can't find script in logic engine!");
    }

    TEST_P(ALogicEngine_Factory, CreatesLuaModule)
    {
        const auto module = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "mymodule");
        ASSERT_NE(nullptr, module);
        EXPECT_TRUE(m_logicEngine.getErrors().empty());

        EXPECT_EQ(module, m_logicEngine.findByName<LuaModule>("mymodule"));
        ASSERT_EQ(1u, m_logicEngine.getCollection<LuaModule>().size());
        EXPECT_EQ(module, *m_logicEngine.getCollection<LuaModule>().cbegin());

        const auto& constLogicEngine = m_logicEngine;
        EXPECT_EQ(module, constLogicEngine.findByName<LuaModule>("mymodule"));
    }

    TEST_P(ALogicEngine_Factory, AllowsCreatingLuaModuleWithEmptyName)
    {
        EXPECT_NE(nullptr, m_logicEngine.createLuaModule(m_moduleSourceCode, {}, ""));
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
    }

    TEST_P(ALogicEngine_Factory, AllowsCreatingLuaModuleWithNameContainingNonAlphanumericChars)
    {
        EXPECT_NE(nullptr, m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "!@#$"));
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
    }

    TEST_P(ALogicEngine_Factory, AllowsCreatingLuaModuleWithDupliciteNameEvenIfSourceDiffers)
    {
        ASSERT_TRUE(m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "mymodule"));
        // same name and same source is OK
        EXPECT_TRUE(m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "mymodule"));

        // same name and different source is also OK
        EXPECT_TRUE(m_logicEngine.createLuaModule("return {}", {}, "mymodule"));
    }

    TEST_P(ALogicEngine_Factory, CanDestroyLuaModule)
    {
        LuaModule* module = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "mymodule");
        ASSERT_NE(nullptr, module);
        EXPECT_TRUE(m_logicEngine.destroy(*module));
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
        EXPECT_FALSE(m_logicEngine.findByName<LuaModule>("mymodule"));
    }

    TEST_P(ALogicEngine_Factory, FailsToDestroyLuaModuleIfFromOtherLogicInstance)
    {
        LogicEngine otherLogic;
        LuaModule* module = otherLogic.createLuaModule(m_moduleSourceCode);
        ASSERT_NE(nullptr, module);

        EXPECT_FALSE(m_logicEngine.destroy(*module));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_EQ(m_logicEngine.getErrors().front().message, "Can't find Lua module in logic engine!");
    }

    TEST_P(ALogicEngine_Factory, FailsToDestroyLuaModuleIfUsedInLuaScript)
    {
        LuaModule* module = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "mymodule");
        ASSERT_NE(nullptr, module);

        constexpr std::string_view valid_empty_script = R"(
            modules("mymodule")
            function interface(IN,OUT)
            end
            function run(IN,OUT)
            end
        )";
        EXPECT_TRUE(m_logicEngine.createLuaScript(valid_empty_script, CreateDeps({ { "mymodule", module } }), "script"));

        EXPECT_FALSE(m_logicEngine.destroy(*module));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_EQ(m_logicEngine.getErrors().front().message, "Failed to destroy LuaModule 'mymodule', it is used in LuaScript 'script'");
    }

    TEST_P(ALogicEngine_Factory, CanDestroyModuleAfterItIsNotUsedAnymore)
    {
        LuaModule* module = m_logicEngine.createLuaModule(m_moduleSourceCode);
        ASSERT_NE(nullptr, module);

        constexpr std::string_view valid_empty_script = R"(
            modules("mymodule")
            function interface(IN,OUT)
            end
            function run(IN,OUT)
            end
        )";
        auto script = m_logicEngine.createLuaScript(valid_empty_script, CreateDeps({ { "mymodule", module } }));
        ASSERT_NE(nullptr, script);
        EXPECT_FALSE(m_logicEngine.destroy(*module));

        EXPECT_TRUE(m_logicEngine.destroy(*script));
        EXPECT_TRUE(m_logicEngine.destroy(*module));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingLuaScriptUsingModuleFromAnotherLogicInstance)
    {
        LogicEngine other;
        const auto module = other.createLuaModule(m_moduleSourceCode);
        ASSERT_NE(nullptr, module);

        EXPECT_EQ(nullptr, m_logicEngine.createLuaScript(m_valid_empty_script, CreateDeps({ { "name", module } })));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_EQ(m_logicEngine.getErrors().front().message,
            "Failed to map Lua module 'name'! It was created on a different instance of LogicEngine.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingLuaModuleUsingModuleFromAnotherLogicInstance)
    {
        LogicEngine other;
        const auto module = other.createLuaModule(m_moduleSourceCode);
        ASSERT_NE(nullptr, module);

        LuaConfig config;
        config.addDependency("name", *module);
        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule(m_valid_empty_script, config));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_EQ(m_logicEngine.getErrors().front().message,
            "Failed to map Lua module 'name'! It was created on a different instance of LogicEngine.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingRamsesNodeBindingFromAnotherEngineInstance)
    {
        LogicEngine otherLogicEngine;

        auto ramsesNodeBinding = otherLogicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        ASSERT_TRUE(ramsesNodeBinding);
        ASSERT_FALSE(m_logicEngine.destroy(*ramsesNodeBinding));
        EXPECT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Can't find RamsesNodeBinding in logic engine!");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingRamsesAppearanceBindingFromAnotherEngineInstance)
    {
        LogicEngine otherLogicEngine;
        auto binding = otherLogicEngine.createRamsesAppearanceBinding(*m_appearance, "AppearanceBinding");
        ASSERT_TRUE(binding);
        ASSERT_FALSE(m_logicEngine.destroy(*binding));
        EXPECT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Can't find RamsesAppearanceBinding in logic engine!");
    }

    TEST_P(ALogicEngine_Factory, DestroysRamsesCameraBindingWithoutErrors)
    {
        auto binding = m_logicEngine.createRamsesCameraBinding(*m_camera, "CameraBinding");
        ASSERT_TRUE(binding);
        ASSERT_TRUE(m_logicEngine.destroy(*binding));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingRamsesCameraBindingFromAnotherEngineInstance)
    {
        LogicEngine otherLogicEngine;
        auto binding = otherLogicEngine.createRamsesCameraBinding(*m_camera, "CameraBinding");
        ASSERT_TRUE(binding);
        ASSERT_FALSE(m_logicEngine.destroy(*binding));
        EXPECT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Can't find RamsesCameraBinding in logic engine!");
    }

    TEST_P(ALogicEngine_Factory, DestroysRamsesRenderPassBindingWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        auto binding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rp");
        ASSERT_TRUE(binding);
        ASSERT_TRUE(m_logicEngine.destroy(*binding));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingRamsesRenderPassBindingFromAnotherEngineInstance)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        LogicEngine otherLogicEngine{ GetParam() };
        auto binding = otherLogicEngine.createRamsesRenderPassBinding(*m_renderPass, "rp");
        ASSERT_TRUE(binding);
        ASSERT_FALSE(m_logicEngine.destroy(*binding));
        EXPECT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Can't find RamsesRenderPassBinding in logic engine!");
    }

    TEST_P(ALogicEngine_Factory, FailsToCreateRamsesRenderPassBindingOnFeatureLevel01)
    {
        if (GetParam() >= EFeatureLevel_02)
            GTEST_SKIP();

        const auto binding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rp");
        EXPECT_FALSE(binding);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create RamsesRenderPassBinding, feature level 02 or higher is required, feature level in this runtime set to 01.");
    }

    TEST_P(ALogicEngine_Factory, DestroysRamsesRenderGroupBindingWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_03)
            GTEST_SKIP();

        auto binding = createRenderGroupBinding();
        ASSERT_TRUE(binding);
        ASSERT_TRUE(m_logicEngine.destroy(*binding));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingRamsesRenderGroupBindingFromAnotherEngineInstance)
    {
        if (GetParam() < EFeatureLevel_03)
            GTEST_SKIP();

        auto binding = createRenderGroupBinding();
        ASSERT_TRUE(binding);

        LogicEngine otherLogicEngine{ GetParam() };
        ASSERT_FALSE(otherLogicEngine.destroy(*binding));
        EXPECT_EQ(otherLogicEngine.getErrors().size(), 1u);
        EXPECT_EQ(otherLogicEngine.getErrors()[0].message, "Can't find RamsesRenderGroupBinding in logic engine!");
    }

    TEST_P(ALogicEngine_Factory, FailsToCreateRamsesRenderGroupBindingOnFeatureLevelBelow03)
    {
        if (GetParam() >= EFeatureLevel_03)
            GTEST_SKIP();

        const auto binding = createRenderGroupBinding();
        EXPECT_FALSE(binding);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, fmt::format("Cannot create RamsesRenderGroupBinding, feature level 03 or higher is required, feature level in this runtime set to 0{}.", GetParam()));
    }

    TEST_P(ALogicEngine_Factory, DestroysRamsesMeshNodeBindingWithoutErrors)
    {
        if (GetParam() < EFeatureLevel_05)
            GTEST_SKIP();

        auto binding = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode);
        ASSERT_TRUE(binding);
        ASSERT_TRUE(m_logicEngine.destroy(*binding));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingRamsesMeshNodeBindingFromAnotherEngineInstance)
    {
        if (GetParam() < EFeatureLevel_05)
            GTEST_SKIP();

        auto binding = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode);
        ASSERT_TRUE(binding);

        LogicEngine otherLogicEngine{ GetParam() };
        ASSERT_FALSE(otherLogicEngine.destroy(*binding));
        EXPECT_EQ(otherLogicEngine.getErrors().size(), 1u);
        EXPECT_EQ(otherLogicEngine.getErrors()[0].message, "Can't find RamsesMeshNodeBinding in logic engine!");
    }

    TEST_P(ALogicEngine_Factory, FailsToCreateRamsesMeshNodeBindingOnFeatureLevelBelow04)
    {
        if (GetParam() >= EFeatureLevel_05)
            GTEST_SKIP();

        const auto binding = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode);
        EXPECT_FALSE(binding);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, fmt::format("Cannot create RamsesMeshNodeBinding, feature level 05 or higher is required, feature level in this runtime set to 0{}.", GetParam()));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingAnchorPointAndNodeOrCameraFromAnotherInstance)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node);
        const auto cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera);

        LogicEngine otherEngine;
        const auto nodeBindingOther = otherEngine.createRamsesNodeBinding(*m_node);
        const auto cameraBindingOther = otherEngine.createRamsesCameraBinding(*m_camera);

        EXPECT_EQ(nullptr, m_logicEngine.createAnchorPoint(*nodeBindingOther, *cameraBinding, "anchor"));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Failed to create AnchorPoint 'anchor': provided Ramses node binding and/or camera binding were not found in this logic instance.");

        EXPECT_EQ(nullptr, m_logicEngine.createAnchorPoint(*nodeBinding, *cameraBindingOther, "anchor"));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Failed to create AnchorPoint 'anchor': provided Ramses node binding and/or camera binding were not found in this logic instance.");
    }

    TEST_P(ALogicEngine_Factory, FailsToCreateAnchorPointOnFeatureLevel01)
    {
        if (GetParam() >= EFeatureLevel_02)
            GTEST_SKIP();

        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node);
        const auto cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera);

        const auto anchorPoint = m_logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
        EXPECT_FALSE(anchorPoint);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create AnchorPoint, feature level 02 or higher is required, feature level in this runtime set to 01.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndNodeOrAppearanceFromAnotherInstance)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node);
        const auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance);

        LogicEngine otherEngine;
        const auto nodeBindingOther = otherEngine.createRamsesNodeBinding(*m_node);
        const auto appearanceBindingOther = otherEngine.createRamsesAppearanceBinding(*m_appearance);

        EXPECT_EQ(nullptr, createSkinBinding(*nodeBindingOther, *appearanceBinding, m_logicEngine));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Failed to create SkinBinding 'skin': one or more of the provided Ramses node bindings was not found in this logic instance.");

        EXPECT_EQ(nullptr, createSkinBinding(*nodeBinding, *appearanceBindingOther, m_logicEngine));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Failed to create SkinBinding 'skin': provided Ramses appearance binding was not found in this logic instance.");
    }

    TEST_P(ALogicEngine_Factory, FailsToCreateSkinBindingOnFeatureLevelBelow04)
    {
        if (GetParam() >= EFeatureLevel_04)
            GTEST_SKIP();

        EXPECT_FALSE(createSkinBinding(m_logicEngine));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, fmt::format("Cannot create SkinBinding, feature level 04 or higher is required, feature level in this runtime set to 0{}.", GetParam()));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndNodesEmptyOrNull)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node);
        auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance);
        ramses::UniformInput uniform;
        m_appearance->getEffect().findUniformInput("jointMat", uniform);
        EXPECT_TRUE(uniform.isValid());

        EXPECT_FALSE(m_logicEngine.createSkinBinding({}, {}, *appearanceBinding, uniform, "skin"));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create SkinBinding, no or null joint node bindings provided.");

        EXPECT_FALSE(m_logicEngine.createSkinBinding({ nodeBinding, nullptr }, { matrix44f{ 0.f }, matrix44f{ 0.f } }, *appearanceBinding, uniform, "skin"));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create SkinBinding, no or null joint node bindings provided.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndNodesCountDifferentFromMatricesCount)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node);
        auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance);
        ramses::UniformInput uniform;
        m_appearance->getEffect().findUniformInput("jointMat", uniform);
        EXPECT_TRUE(uniform.isValid());

        EXPECT_FALSE(m_logicEngine.createSkinBinding({ nodeBinding }, { matrix44f{ 0.f }, matrix44f{ 0.f } }, *appearanceBinding, uniform, "skin"));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create SkinBinding, number of inverse matrices must match the number of joints.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndUniformInvalidOrFromAnotherEffect)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node);
        auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance);

        // invalid uniform
        ramses::UniformInput uniform;
        EXPECT_FALSE(uniform.isValid());

        EXPECT_FALSE(m_logicEngine.createSkinBinding({ nodeBinding }, { matrix44f{ 0.f } }, *appearanceBinding, uniform, "skin"));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create SkinBinding, provided uniform input must be pointing to valid uniform of the provided appearance's effect and must not be bound.");

        // valid uniform but from other effect
        const std::string_view vertShader = R"(
                #version 100
                uniform highp float someUniform;
                attribute vec3 a_position;
                void main()
                {
                    gl_Position = someUniform * vec4(a_position, 1.0);
                })";
        const std::string_view fragShader = R"(
                #version 100
                void main(void)
                {
                    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
                })";
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShader(vertShader.data());
        effectDesc.setFragmentShader(fragShader.data());
        const auto otherEffect = m_scene->createEffect(effectDesc);
        otherEffect->findUniformInput("someUniform", uniform);
        EXPECT_TRUE(uniform.isValid());

        EXPECT_FALSE(m_logicEngine.createSkinBinding({ nodeBinding }, { matrix44f{ 0.f } }, *appearanceBinding, uniform, "skin"));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create SkinBinding, provided uniform input must be pointing to valid uniform of the provided appearance's effect and must not be bound.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndUniformIsBoundInRamses)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node);
        auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance);

        ramses::UniformInput uniform;
        m_appearance->getEffect().findUniformInput("floatUniform", uniform);
        EXPECT_TRUE(uniform.isValid());
        EXPECT_EQ(ramses::StatusOK, m_appearance->bindInput(uniform, *m_scene->createDataFloat()));

        EXPECT_FALSE(m_logicEngine.createSkinBinding({ nodeBinding }, { matrix44f{ 0.f } }, *appearanceBinding, uniform, "skin"));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create SkinBinding, provided uniform input must be pointing to valid uniform of the provided appearance's effect and must not be bound.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndUniformDataTypeWrong)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node);
        auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance);

        ramses::UniformInput uniform;
        m_appearance->getEffect().findUniformInput("floatUniform", uniform);
        EXPECT_TRUE(uniform.isValid());

        EXPECT_FALSE(m_logicEngine.createSkinBinding({ nodeBinding }, { matrix44f{ 0.f } }, *appearanceBinding, uniform, "skin"));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create SkinBinding, provided uniform input must be of type array of Matrix4x4 with element count matching number of joints.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndUniformPointsToArrayWithMismatchedSize)
    {
        if (GetParam() < EFeatureLevel_04)
            GTEST_SKIP();

        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node);

        const std::string_view vertShader = R"(
                #version 100
                uniform highp mat4 someArray[3];
                attribute vec3 a_position;
                void main()
                {
                    gl_Position = someArray[0] * vec4(a_position, 1.0);
                })";
        const std::string_view fragShader = R"(
                #version 100
                void main(void)
                {
                    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
                })";
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShader(vertShader.data());
        effectDesc.setFragmentShader(fragShader.data());
        const auto otherEffect = m_scene->createEffect(effectDesc);
        ramses::UniformInput uniform;
        otherEffect->findUniformInput("someArray", uniform);
        EXPECT_TRUE(uniform.isValid());
        auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_scene->createAppearance(*otherEffect));

        EXPECT_FALSE(m_logicEngine.createSkinBinding({ nodeBinding }, { matrix44f{ 0.f } }, *appearanceBinding, uniform, "skin"));
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_EQ(m_logicEngine.getErrors()[0].message, "Cannot create SkinBinding, provided uniform input must be of type array of Matrix4x4 with element count matching number of joints.");
    }

    TEST_P(ALogicEngine_Factory, RenamesObjectsAfterCreation)
    {
        auto script = m_logicEngine.createLuaScript(m_valid_empty_script);
        auto ramsesNodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        auto ramsesAppearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "AppearanceBinding");
        auto ramsesCameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "CameraBinding");

        script->setName("same name twice");
        ramsesNodeBinding->setName("same name twice");
        ramsesAppearanceBinding->setName("");
        ramsesCameraBinding->setName("");

        EXPECT_EQ("same name twice", script->getName());
        EXPECT_EQ("same name twice", ramsesNodeBinding->getName());
        EXPECT_EQ("", ramsesAppearanceBinding->getName());
        EXPECT_EQ("", ramsesCameraBinding->getName());

        if (GetParam() >= EFeatureLevel_02)
        {
            auto ramsesRenderPassBinding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rp");
            ramsesRenderPassBinding->setName("");
            EXPECT_EQ("", ramsesRenderPassBinding->getName());
        }

        if (GetParam() >= EFeatureLevel_03)
        {
            auto ramsesRenderGroupBinding = createRenderGroupBinding();
            ramsesRenderGroupBinding->setName("");
            EXPECT_EQ("", ramsesRenderGroupBinding->getName());
        }
    }

    TEST_P(ALogicEngine_Factory, CanCastObjectsToValidTypes)
    {
        LogicObject* luaModule         = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        LogicObject* luaScript         = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        LogicObject* nodeBinding       = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        LogicObject* appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        LogicObject* cameraBinding     = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        LogicObject* dataArray         = m_logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray->as<DataArray>(), dataArray->as<DataArray>(), EInterpolationType::Linear });
        LogicObject* animNode          = m_logicEngine.createAnimationNode(config, "animNode");

        EXPECT_TRUE(luaModule->as<LuaModule>());
        EXPECT_TRUE(luaScript->as<LuaScript>());
        EXPECT_TRUE(nodeBinding->as<RamsesNodeBinding>());
        EXPECT_TRUE(appearanceBinding->as<RamsesAppearanceBinding>());
        EXPECT_TRUE(cameraBinding->as<RamsesCameraBinding>());
        EXPECT_TRUE(dataArray->as<DataArray>());
        EXPECT_TRUE(animNode->as<AnimationNode>());

        EXPECT_FALSE(luaModule->as<AnimationNode>());
        EXPECT_FALSE(luaScript->as<DataArray>());
        EXPECT_FALSE(nodeBinding->as<RamsesCameraBinding>());
        EXPECT_FALSE(appearanceBinding->as<AnimationNode>());
        EXPECT_FALSE(cameraBinding->as<RamsesNodeBinding>());
        EXPECT_FALSE(dataArray->as<LuaScript>());
        EXPECT_FALSE(animNode->as<LuaModule>());

        if (GetParam() >= EFeatureLevel_02)
        {
            LogicObject* renderPassBinding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rp");
            EXPECT_TRUE(renderPassBinding->as<RamsesRenderPassBinding>());
            EXPECT_FALSE(renderPassBinding->as<RamsesCameraBinding>());
        }

        if (GetParam() >= EFeatureLevel_03)
        {
            LogicObject* renderGroupBinding = createRenderGroupBinding();
            EXPECT_TRUE(renderGroupBinding->as<RamsesRenderGroupBinding>());
            EXPECT_FALSE(renderGroupBinding->as<RamsesCameraBinding>());
        }

        if (GetParam() >= EFeatureLevel_04)
        {
            LogicObject* skin = createSkinBinding(m_logicEngine);
            EXPECT_TRUE(skin->as<SkinBinding>());
            EXPECT_FALSE(skin->as<RamsesCameraBinding>());
        }

        if (GetParam() >= EFeatureLevel_05)
        {
            LogicObject* meshBinding = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode);
            EXPECT_TRUE(meshBinding->as<RamsesMeshNodeBinding>());
            EXPECT_FALSE(meshBinding->as<RamsesCameraBinding>());
        }

        //cast obj -> node -> binding -> appearanceBinding
        auto* nodeCastFromObject = appearanceBinding->as<LogicNode>();
        EXPECT_TRUE(nodeCastFromObject);
        auto* bindingCastFromNode = nodeCastFromObject->as<RamsesBinding>();
        EXPECT_TRUE(bindingCastFromNode);
        auto* appearanceBindingCastFromBinding = bindingCastFromNode->as<RamsesAppearanceBinding>();
        EXPECT_TRUE(appearanceBindingCastFromBinding);

        //cast appearanceBinding -> binding -> node -> obj
        EXPECT_TRUE(appearanceBindingCastFromBinding->as<RamsesBinding>());
        EXPECT_TRUE(bindingCastFromNode->as<LogicNode>());
        EXPECT_TRUE(nodeCastFromObject->as<LogicObject>());

        //cast obj -> node -> animationnode
        auto* anNodeCastFromObject = animNode->as<LogicNode>();
        EXPECT_TRUE(anNodeCastFromObject);
        auto* animationCastFromNode = anNodeCastFromObject->as<AnimationNode>();
        EXPECT_TRUE(animationCastFromNode);

        //cast animationnode -> node -> obj
        EXPECT_TRUE(animationCastFromNode->as<LogicNode>());
        EXPECT_TRUE(anNodeCastFromObject->as<LogicObject>());
    }

    TEST_P(ALogicEngine_Factory, CanCastObjectsToValidTypes_Const)
    {
        m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const LogicObject* dataArray = m_logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray->as<DataArray>(), dataArray->as<DataArray>(), EInterpolationType::Linear });
        m_logicEngine.createAnimationNode(config, "animNode");

        const auto& immutableLogicEngine = m_logicEngine;
        const auto* luaModuleConst         = immutableLogicEngine.findByName<LogicObject>("luaModule");
        const auto* luaScriptConst         = immutableLogicEngine.findByName<LogicObject>("script");
        const auto* nodeBindingConst       = immutableLogicEngine.findByName<LogicObject>("nodebinding");
        const auto* appearanceBindingConst = immutableLogicEngine.findByName<LogicObject>("appbinding");
        const auto* cameraBindingConst     = immutableLogicEngine.findByName<LogicObject>("camerabinding");
        const auto* dataArrayConst         = immutableLogicEngine.findByName<LogicObject>("dataarray");
        const auto* animNodeConst          = immutableLogicEngine.findByName<LogicObject>("animNode");

        EXPECT_TRUE(luaModuleConst->as<LuaModule>());
        EXPECT_TRUE(luaScriptConst->as<LuaScript>());
        EXPECT_TRUE(nodeBindingConst->as<RamsesNodeBinding>());
        EXPECT_TRUE(appearanceBindingConst->as<RamsesAppearanceBinding>());
        EXPECT_TRUE(cameraBindingConst->as<RamsesCameraBinding>());
        EXPECT_TRUE(dataArrayConst->as<DataArray>());
        EXPECT_TRUE(animNodeConst->as<AnimationNode>());

        EXPECT_FALSE(luaModuleConst->as<AnimationNode>());
        EXPECT_FALSE(luaScriptConst->as<DataArray>());
        EXPECT_FALSE(nodeBindingConst->as<RamsesCameraBinding>());
        EXPECT_FALSE(appearanceBindingConst->as<AnimationNode>());
        EXPECT_FALSE(cameraBindingConst->as<RamsesNodeBinding>());
        EXPECT_FALSE(dataArrayConst->as<LuaScript>());
        EXPECT_FALSE(animNodeConst->as<LuaModule>());

        if (GetParam() >= EFeatureLevel_02)
        {
            m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "renderPass");
            const auto* renderPassBindingConst = immutableLogicEngine.findByName<LogicObject>("renderPass");
            EXPECT_TRUE(renderPassBindingConst->as<RamsesRenderPassBinding>());
            EXPECT_FALSE(renderPassBindingConst->as<LuaInterface>());
        }

        if (GetParam() >= EFeatureLevel_03)
        {
            createRenderGroupBinding();
            const auto* renderGroupBindingConst = immutableLogicEngine.findByName<LogicObject>("renderGroupBinding");
            EXPECT_TRUE(renderGroupBindingConst->as<RamsesRenderGroupBinding>());
            EXPECT_FALSE(renderGroupBindingConst->as<LuaInterface>());
        }

        if (GetParam() >= EFeatureLevel_04)
        {
            const LogicObject* skin = createSkinBinding(m_logicEngine);
            EXPECT_TRUE(skin->as<SkinBinding>());
            EXPECT_FALSE(skin->as<RamsesCameraBinding>());
        }

        if (GetParam() >= EFeatureLevel_05)
        {
            const LogicObject* meshBinding = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode);
            EXPECT_TRUE(meshBinding->as<RamsesMeshNodeBinding>());
            EXPECT_FALSE(meshBinding->as<RamsesCameraBinding>());
        }

        // cast obj -> node -> binding -> appearanceBinding
        const auto* nodeCastFromObject = appearanceBindingConst->as<LogicNode>();
        EXPECT_TRUE(nodeCastFromObject);
        const auto* bindingCastFromNode = nodeCastFromObject->as<RamsesBinding>();
        EXPECT_TRUE(bindingCastFromNode);
        const auto* appearanceBindingCastFromBinding = bindingCastFromNode->as<RamsesAppearanceBinding>();
        EXPECT_TRUE(appearanceBindingCastFromBinding);

        // cast appearanceBinding -> binding -> node -> obj
        EXPECT_TRUE(appearanceBindingCastFromBinding->as<RamsesBinding>());
        EXPECT_TRUE(bindingCastFromNode->as<LogicNode>());
        EXPECT_TRUE(nodeCastFromObject->as<LogicObject>());

        // cast obj -> node -> animationnode
        const auto* anNodeCastFromObject = animNodeConst->as<LogicNode>();
        EXPECT_TRUE(anNodeCastFromObject);
        const auto* animationCastFromNode = anNodeCastFromObject->as<AnimationNode>();
        EXPECT_TRUE(animationCastFromNode);

        // cast animationnode -> node -> obj
        EXPECT_TRUE(animationCastFromNode->as<LogicNode>());
        EXPECT_TRUE(anNodeCastFromObject->as<LogicObject>());
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorIfWrongObjectTypeIsDestroyed)
    {
        struct UnknownObjectImpl: internal::LogicNodeImpl
        {
            UnknownObjectImpl()
                : LogicNodeImpl("name", 1u)
            {
            }

            std::optional<internal::LogicNodeRuntimeError> update() override { return std::nullopt; }
            void createRootProperties() final {}
        };

        struct UnknownObject : LogicNode
        {
            explicit UnknownObject(std::unique_ptr<UnknownObjectImpl> impl)
                : LogicNode(std::move(impl))
            {
            }
        };

        UnknownObject     unknownObject(std::make_unique<UnknownObjectImpl>());
        EXPECT_FALSE(m_logicEngine.destroy(unknownObject));
        const auto& errors = m_logicEngine.getErrors();
        EXPECT_EQ(1u, errors.size());
        EXPECT_EQ(errors[0].message, "Tried to destroy object 'name' with unknown type");
    }

    TEST_P(ALogicEngine_Factory, CanBeMoved)
    {
        LuaScript* script = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "Script");
        RamsesNodeBinding* ramsesNodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        RamsesAppearanceBinding* appBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "AppearanceBinding");
        RamsesCameraBinding* camBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "CameraBinding");
        const RamsesRenderPassBinding* rpBinding = nullptr;
        if (GetParam() >= EFeatureLevel_02)
            rpBinding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "RenderPass");
        const RamsesRenderGroupBinding* rgBinding = nullptr;
        if (GetParam() >= EFeatureLevel_03)
            rgBinding = createRenderGroupBinding();

        LogicEngine movedLogicEngine(std::move(m_logicEngine));
        EXPECT_EQ(script, movedLogicEngine.findByName<LuaScript>("Script"));
        EXPECT_EQ(ramsesNodeBinding, movedLogicEngine.findByName<RamsesNodeBinding>("NodeBinding"));
        EXPECT_EQ(appBinding, movedLogicEngine.findByName<RamsesAppearanceBinding>("AppearanceBinding"));
        EXPECT_EQ(camBinding, movedLogicEngine.findByName<RamsesCameraBinding>("CameraBinding"));
        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_EQ(rpBinding, movedLogicEngine.findByName<RamsesRenderPassBinding>("RenderPass"));
        }
        if (GetParam() >= EFeatureLevel_03)
        {
            EXPECT_EQ(rgBinding, movedLogicEngine.findByName<RamsesRenderGroupBinding>("renderGroupBinding"));
        }

        movedLogicEngine.update();

        LogicEngine moveAssignedLogicEngine;
        moveAssignedLogicEngine = std::move(movedLogicEngine);

        EXPECT_EQ(script, moveAssignedLogicEngine.findByName<LuaScript>("Script"));
        EXPECT_EQ(ramsesNodeBinding, moveAssignedLogicEngine.findByName<RamsesNodeBinding>("NodeBinding"));
        EXPECT_EQ(appBinding, moveAssignedLogicEngine.findByName<RamsesAppearanceBinding>("AppearanceBinding"));
        EXPECT_EQ(camBinding, moveAssignedLogicEngine.findByName<RamsesCameraBinding>("CameraBinding"));
        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_EQ(rpBinding, moveAssignedLogicEngine.findByName<RamsesRenderPassBinding>("RenderPass"));
        }
        if (GetParam() >= EFeatureLevel_03)
        {
            EXPECT_EQ(rgBinding, moveAssignedLogicEngine.findByName<RamsesRenderGroupBinding>("renderGroupBinding"));
        }

        moveAssignedLogicEngine.update();
    }
}
