//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/LuaModule.h"
#include "ramses/client/logic/AnimationNodeConfig.h"
#include "ramses/client/logic/RenderBufferBinding.h"

#include "ramses/client/DataObject.h"

#include "impl/logic/LogicNodeImpl.h"

#include "FeatureLevelTestValues.h"

#include <fmt/format.h>
#include <fstream>

namespace ramses::internal
{
    // There are more specific "create/destroy" tests in ApiObjects unit tests!
    class ALogicEngine_Factory : public ALogicEngineBase, public ::testing::TestWithParam<ramses::EFeatureLevel>
    {
    public:
        ALogicEngine_Factory() : ALogicEngineBase{ GetParam() }
        {
            withTempDirectory();
        }
    };

    RAMSES_INSTANTIATE_LATEST_FEATURELEVEL_ONLY_TEST_SUITE(ALogicEngine_Factory);

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingEmptyScript)
    {
        const LuaScript* script = m_logicEngine->createLuaScript("");
        ASSERT_EQ(nullptr, script);
        EXPECT_EQ(getLastErrorMessage(), "[] No 'run' function defined!");
    }

    TEST_P(ALogicEngine_Factory, CreatesScriptFromValidLuaWithoutErrors)
    {
        const LuaScript* script = m_logicEngine->createLuaScript(m_valid_empty_script);
        ASSERT_TRUE(nullptr != script);
        expectNoError();
    }

    TEST_P(ALogicEngine_Factory, DestroysScriptWithoutErrors)
    {
        LuaScript* script = m_logicEngine->createLuaScript(m_valid_empty_script);
        ASSERT_TRUE(script);
        ASSERT_TRUE(m_logicEngine->destroy(*script));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingScriptFromAnotherEngineInstance)
    {
        auto& otherLogicEngine = *m_scene->createLogicEngine();
        auto script = otherLogicEngine.createLuaScript(m_valid_empty_script);
        ASSERT_TRUE(script);
        ASSERT_FALSE(m_logicEngine->destroy(*script));
        EXPECT_EQ(getLastErrorMessage(), "Failed to destroy object ' [LogicObject ScnObjId=11]', cannot find it in this LogicEngine instance.");
    }

    TEST_P(ALogicEngine_Factory, CreatesLuaModule)
    {
        const auto module = m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "mymodule");
        ASSERT_NE(nullptr, module);
        expectNoError();

        EXPECT_EQ(module, m_logicEngine->findObject<LuaModule>("mymodule"));
        ASSERT_EQ(1u, m_logicEngine->getCollection<LuaModule>().size());
        EXPECT_EQ(module, *m_logicEngine->getCollection<LuaModule>().cbegin());

        const auto* constLogicEngine = m_logicEngine;
        EXPECT_EQ(module, constLogicEngine->findObject<LuaModule>("mymodule"));
    }

    TEST_P(ALogicEngine_Factory, AllowsCreatingLuaModuleWithEmptyName)
    {
        EXPECT_NE(nullptr, m_logicEngine->createLuaModule(m_moduleSourceCode, {}, ""));
        expectNoError();
    }

    TEST_P(ALogicEngine_Factory, AllowsCreatingLuaModuleWithNameContainingNonAlphanumericChars)
    {
        EXPECT_NE(nullptr, m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "!@#$"));
        expectNoError();
    }

    TEST_P(ALogicEngine_Factory, AllowsCreatingLuaModuleWithDupliciteNameEvenIfSourceDiffers)
    {
        ASSERT_TRUE(m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "mymodule"));
        // same name and same source is OK
        EXPECT_TRUE(m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "mymodule"));

        // same name and different source is also OK
        EXPECT_TRUE(m_logicEngine->createLuaModule("return {}", {}, "mymodule"));
    }

    TEST_P(ALogicEngine_Factory, CanDestroyLuaModule)
    {
        LuaModule* module = m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "mymodule");
        ASSERT_NE(nullptr, module);
        EXPECT_TRUE(m_logicEngine->destroy(*module));
        expectNoError();
        EXPECT_FALSE(m_logicEngine->findObject<LuaModule>("mymodule"));
    }

    TEST_P(ALogicEngine_Factory, FailsToDestroyLuaModuleIfFromOtherLogicInstance)
    {
        auto& otherLogic = *m_scene->createLogicEngine();
        LuaModule* module = otherLogic.createLuaModule(m_moduleSourceCode);
        ASSERT_NE(nullptr, module);

        EXPECT_FALSE(m_logicEngine->destroy(*module));
        EXPECT_EQ(getLastErrorMessage(), "Failed to destroy object ' [LogicObject ScnObjId=11]', cannot find it in this LogicEngine instance.");
    }

    TEST_P(ALogicEngine_Factory, FailsToDestroyLuaModuleIfUsedInLuaScript)
    {
        LuaModule* module = m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "mymodule");
        ASSERT_NE(nullptr, module);

        constexpr std::string_view valid_empty_script = R"(
            modules("mymodule")
            function interface(IN,OUT)
            end
            function run(IN,OUT)
            end
        )";
        EXPECT_TRUE(m_logicEngine->createLuaScript(valid_empty_script, CreateDeps({ { "mymodule", module } }), "script"));

        EXPECT_FALSE(m_logicEngine->destroy(*module));
        EXPECT_EQ(getLastErrorMessage(), "Failed to destroy LuaModule 'mymodule', it is used in LuaScript 'script'");
    }

    TEST_P(ALogicEngine_Factory, CanDestroyModuleAfterItIsNotUsedAnymore)
    {
        LuaModule* module = m_logicEngine->createLuaModule(m_moduleSourceCode);
        ASSERT_NE(nullptr, module);

        constexpr std::string_view valid_empty_script = R"(
            modules("mymodule")
            function interface(IN,OUT)
            end
            function run(IN,OUT)
            end
        )";
        auto script = m_logicEngine->createLuaScript(valid_empty_script, CreateDeps({ { "mymodule", module } }));
        ASSERT_NE(nullptr, script);
        EXPECT_FALSE(m_logicEngine->destroy(*module));

        EXPECT_TRUE(m_logicEngine->destroy(*script));
        EXPECT_TRUE(m_logicEngine->destroy(*module));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingLuaScriptUsingModuleFromAnotherLogicInstance)
    {
        auto& other = *m_scene->createLogicEngine();
        const auto module = other.createLuaModule(m_moduleSourceCode);
        ASSERT_NE(nullptr, module);

        EXPECT_EQ(nullptr, m_logicEngine->createLuaScript(m_valid_empty_script, CreateDeps({ { "name", module } })));
        EXPECT_EQ(getLastErrorMessage(),
            "Failed to map Lua module 'name'! It was created on a different instance of LogicEngine.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingLuaModuleUsingModuleFromAnotherLogicInstance)
    {
        auto& other = *m_scene->createLogicEngine();
        const auto module = other.createLuaModule(m_moduleSourceCode);
        ASSERT_NE(nullptr, module);

        LuaConfig config;
        config.addDependency("name", *module);
        EXPECT_EQ(nullptr, m_logicEngine->createLuaModule(m_valid_empty_script, config));
        EXPECT_EQ(getLastErrorMessage(),
            "Failed to map Lua module 'name'! It was created on a different instance of LogicEngine.");
    }

    TEST_P(ALogicEngine_Factory, FailsToCreateNodeBindingIfFromOtherScene)
    {
        auto& logicEngineFromOtherScene{ *m_ramses.createScene(sceneId_t{ 666u })->createLogicEngine() };

        EXPECT_EQ(nullptr, logicEngineFromOtherScene.createNodeBinding(*m_node));
        EXPECT_EQ(getLastErrorMessage(), "Failed to create NodeBinding, object is from sceneId=1 but LogicEngine is from sceneId=666");
    }

    TEST_P(ALogicEngine_Factory, FailsToCreateAppearanceBindingIfFromOtherScene)
    {
        auto& logicEngineFromOtherScene{ *m_ramses.createScene(sceneId_t{ 666u })->createLogicEngine() };

        EXPECT_EQ(nullptr, logicEngineFromOtherScene.createAppearanceBinding(*m_appearance));
        EXPECT_EQ(getLastErrorMessage(), "Failed to create AppearanceBinding, object is from sceneId=1 but LogicEngine is from sceneId=666");
    }

    TEST_P(ALogicEngine_Factory, FailsToCreateCameraBindingIfFromOtherScene)
    {
        auto& logicEngineFromOtherScene{ *m_ramses.createScene(sceneId_t{ 666u })->createLogicEngine() };

        EXPECT_EQ(nullptr, logicEngineFromOtherScene.createCameraBinding(*m_camera));
        EXPECT_EQ(getLastErrorMessage(), "Failed to create CameraBinding, object is from sceneId=1 but LogicEngine is from sceneId=666");

        EXPECT_EQ(nullptr, logicEngineFromOtherScene.createCameraBindingWithFrustumPlanes(*m_camera));
        EXPECT_EQ(getLastErrorMessage(), "Failed to create CameraBinding, object is from sceneId=1 but LogicEngine is from sceneId=666");
    }

    TEST_P(ALogicEngine_Factory, FailsToCreateRenderPassBindingIfFromOtherScene)
    {
        auto& logicEngineFromOtherScene{ *m_ramses.createScene(sceneId_t{ 666u })->createLogicEngine() };

        EXPECT_EQ(nullptr, logicEngineFromOtherScene.createRenderPassBinding(*m_renderPass));
        EXPECT_EQ(getLastErrorMessage(), "Failed to create RenderPassBinding, object is from sceneId=1 but LogicEngine is from sceneId=666");
    }

    TEST_P(ALogicEngine_Factory, FailsToCreateRenderGroupBindingIfFromOtherScene)
    {
        auto& logicEngineFromOtherScene{ *m_ramses.createScene(sceneId_t{ 666u })->createLogicEngine() };

        EXPECT_EQ(nullptr, createRenderGroupBinding(logicEngineFromOtherScene));
        EXPECT_EQ(getLastErrorMessage(), "Failed to create RenderGroupBinding, object is from sceneId=1 but LogicEngine is from sceneId=666");
    }

    TEST_P(ALogicEngine_Factory, FailsToCreateRenderGroupBindingIfElementsFromOtherScene)
    {
        auto& logicEngineFromOtherScene{ *m_ramses.createScene(sceneId_t{ 666u })->createLogicEngine() };

        RenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
        auto rg = logicEngineFromOtherScene.getScene().createRenderGroup();
        // RG is from same scene as logic but elements are not
        EXPECT_EQ(nullptr, logicEngineFromOtherScene.createRenderGroupBinding(*rg, elements));

        EXPECT_EQ(getLastErrorMessage(), "Failed to create RenderGroupBinding, elements are from sceneId=1 but LogicEngine is from sceneId=666");
    }

    TEST_P(ALogicEngine_Factory, FailsToCreateMeshNodeBindingIfFromOtherScene)
    {
        auto& logicEngineFromOtherScene{ *m_ramses.createScene(sceneId_t{ 666u })->createLogicEngine() };

        EXPECT_EQ(nullptr, logicEngineFromOtherScene.createMeshNodeBinding(*m_meshNode));
        EXPECT_EQ(getLastErrorMessage(), "Failed to create MeshNodeBinding, object is from sceneId=1 but LogicEngine is from sceneId=666");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingNodeBindingFromAnotherEngineInstance)
    {
        auto& otherLogicEngine = *m_scene->createLogicEngine();

        auto nodeBinding = otherLogicEngine.createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
        ASSERT_TRUE(nodeBinding);
        ASSERT_FALSE(m_logicEngine->destroy(*nodeBinding));
        EXPECT_EQ(getLastErrorMessage(), "Failed to destroy object 'NodeBinding [LogicObject ScnObjId=11]', cannot find it in this LogicEngine instance.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingAppearanceBindingFromAnotherEngineInstance)
    {
        auto& otherLogicEngine = *m_scene->createLogicEngine();
        auto binding = otherLogicEngine.createAppearanceBinding(*m_appearance, "AppearanceBinding");
        ASSERT_TRUE(binding);
        ASSERT_FALSE(m_logicEngine->destroy(*binding));
        EXPECT_EQ(getLastErrorMessage(), "Failed to destroy object 'AppearanceBinding [LogicObject ScnObjId=11]', cannot find it in this LogicEngine instance.");
    }

    TEST_P(ALogicEngine_Factory, DestroysCameraBindingWithoutErrors)
    {
        auto binding = m_logicEngine->createCameraBinding(*m_camera, "CameraBinding");
        ASSERT_TRUE(binding);
        ASSERT_TRUE(m_logicEngine->destroy(*binding));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingCameraBindingFromAnotherEngineInstance)
    {
        auto& otherLogicEngine = *m_scene->createLogicEngine();
        auto binding = otherLogicEngine.createCameraBinding(*m_camera, "CameraBinding");
        ASSERT_TRUE(binding);
        ASSERT_FALSE(m_logicEngine->destroy(*binding));
        EXPECT_EQ(getLastErrorMessage(), "Failed to destroy object 'CameraBinding [LogicObject ScnObjId=11]', cannot find it in this LogicEngine instance.");
    }

    TEST_P(ALogicEngine_Factory, DestroysRenderPassBindingWithoutErrors)
    {
        auto binding = m_logicEngine->createRenderPassBinding(*m_renderPass, "rp");
        ASSERT_TRUE(binding);
        ASSERT_TRUE(m_logicEngine->destroy(*binding));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingRenderPassBindingFromAnotherEngineInstance)
    {
        auto& otherLogicEngine = *m_scene->createLogicEngine();
        auto binding = otherLogicEngine.createRenderPassBinding(*m_renderPass, "rp");
        ASSERT_TRUE(binding);
        ASSERT_FALSE(m_logicEngine->destroy(*binding));
        EXPECT_EQ(getLastErrorMessage(), "Failed to destroy object 'rp [LogicObject ScnObjId=11]', cannot find it in this LogicEngine instance.");
    }

    TEST_P(ALogicEngine_Factory, DestroysRenderGroupBindingWithoutErrors)
    {
        auto binding = createRenderGroupBinding();
        ASSERT_TRUE(binding);
        ASSERT_TRUE(m_logicEngine->destroy(*binding));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingRenderGroupBindingFromAnotherEngineInstance)
    {
        auto binding = createRenderGroupBinding();
        ASSERT_TRUE(binding);

        auto& otherLogicEngine = *m_scene->createLogicEngine();
        ASSERT_FALSE(otherLogicEngine.destroy(*binding));
        EXPECT_EQ(getLastErrorMessage(), "Failed to destroy object 'renderGroupBinding [LogicObject ScnObjId=10]', cannot find it in this LogicEngine instance.");
    }

    TEST_P(ALogicEngine_Factory, DestroysMeshNodeBindingWithoutErrors)
    {
        auto binding = m_logicEngine->createMeshNodeBinding(*m_meshNode);
        ASSERT_TRUE(binding);
        ASSERT_TRUE(m_logicEngine->destroy(*binding));
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorsWhenDestroyingMeshNodeBindingFromAnotherEngineInstance)
    {
        auto binding = m_logicEngine->createMeshNodeBinding(*m_meshNode);
        ASSERT_TRUE(binding);

        auto& otherLogicEngine = *m_scene->createLogicEngine();
        ASSERT_FALSE(otherLogicEngine.destroy(*binding));
        EXPECT_EQ(getLastErrorMessage(), "Failed to destroy object ' [LogicObject ScnObjId=10]', cannot find it in this LogicEngine instance.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingAnchorPointAndNodeOrCameraFromAnotherInstance)
    {
        const auto nodeBinding = m_logicEngine->createNodeBinding(*m_node);
        const auto cameraBinding = m_logicEngine->createCameraBinding(*m_camera);

        auto& otherEngine = *m_scene->createLogicEngine();
        const auto nodeBindingOther = otherEngine.createNodeBinding(*m_node);
        const auto cameraBindingOther = otherEngine.createCameraBinding(*m_camera);

        EXPECT_EQ(nullptr, m_logicEngine->createAnchorPoint(*nodeBindingOther, *cameraBinding, "anchor"));
        EXPECT_EQ(getLastErrorMessage(), "Failed to create AnchorPoint 'anchor': provided Ramses node binding and/or camera binding were not found in this logic instance.");

        EXPECT_EQ(nullptr, m_logicEngine->createAnchorPoint(*nodeBinding, *cameraBindingOther, "anchor"));
        EXPECT_EQ(getLastErrorMessage(), "Failed to create AnchorPoint 'anchor': provided Ramses node binding and/or camera binding were not found in this logic instance.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndNodeOrAppearanceFromAnotherInstance)
    {
        const auto nodeBinding = m_logicEngine->createNodeBinding(*m_node);
        const auto appearanceBinding = m_logicEngine->createAppearanceBinding(*m_appearance);

        auto& otherEngine = *m_scene->createLogicEngine();
        const auto nodeBindingOther = otherEngine.createNodeBinding(*m_node);
        const auto appearanceBindingOther = otherEngine.createAppearanceBinding(*m_appearance);

        EXPECT_EQ(nullptr, createSkinBinding(*nodeBindingOther, *appearanceBinding, *m_logicEngine));
        EXPECT_EQ(getLastErrorMessage(), "Failed to create SkinBinding 'skin': one or more of the provided Ramses node bindings was not found in this logic instance.");

        EXPECT_EQ(nullptr, createSkinBinding(*nodeBinding, *appearanceBindingOther, *m_logicEngine));
        EXPECT_EQ(getLastErrorMessage(), "Failed to create SkinBinding 'skin': provided Ramses appearance binding was not found in this logic instance.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndNodesEmptyOrNull)
    {
        const auto nodeBinding = m_logicEngine->createNodeBinding(*m_node);
        auto appearanceBinding = m_logicEngine->createAppearanceBinding(*m_appearance);
        const auto optUniform = m_appearance->getEffect().findUniformInput("jointMat");
        ASSERT_TRUE(optUniform.has_value());

        EXPECT_FALSE(m_logicEngine->createSkinBinding({}, {}, *appearanceBinding, *optUniform, "skin"));
        EXPECT_EQ(getLastErrorMessage(), "Cannot create SkinBinding, no or null joint node bindings provided.");

        EXPECT_FALSE(m_logicEngine->createSkinBinding({ nodeBinding, nullptr }, { matrix44f{ 0.f }, matrix44f{ 0.f } }, *appearanceBinding, *optUniform, "skin"));
        EXPECT_EQ(getLastErrorMessage(), "Cannot create SkinBinding, no or null joint node bindings provided.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndNodesCountDifferentFromMatricesCount)
    {
        const auto nodeBinding = m_logicEngine->createNodeBinding(*m_node);
        auto appearanceBinding = m_logicEngine->createAppearanceBinding(*m_appearance);

        const auto optUniform = m_appearance->getEffect().findUniformInput("jointMat");
        ASSERT_TRUE(optUniform.has_value());

        EXPECT_FALSE(m_logicEngine->createSkinBinding({ nodeBinding }, { matrix44f{ 0.f }, matrix44f{ 0.f } }, *appearanceBinding, *optUniform, "skin"));
        EXPECT_EQ(getLastErrorMessage(), "Cannot create SkinBinding, number of inverse matrices must match the number of joints.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndUniformInvalidOrFromAnotherEffect)
    {
        const auto nodeBinding = m_logicEngine->createNodeBinding(*m_node);
        auto appearanceBinding = m_logicEngine->createAppearanceBinding(*m_appearance);

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
        const auto optUniform = otherEffect->findUniformInput("someUniform");
        ASSERT_TRUE(optUniform.has_value());

        EXPECT_FALSE(m_logicEngine->createSkinBinding({ nodeBinding }, { matrix44f{ 0.f } }, *appearanceBinding, *optUniform, "skin"));
        EXPECT_EQ(getLastErrorMessage(), "Cannot create SkinBinding, provided uniform input must be pointing to valid uniform of the provided appearance's effect and must not be bound.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndUniformIsBoundInRamses)
    {
        const auto nodeBinding = m_logicEngine->createNodeBinding(*m_node);
        auto appearanceBinding = m_logicEngine->createAppearanceBinding(*m_appearance);

        const auto optUniform = m_appearance->getEffect().findUniformInput("floatUniform");
        ASSERT_TRUE(optUniform.has_value());
        EXPECT_TRUE(m_appearance->bindInput(*optUniform, *m_scene->createDataObject(ramses::EDataType::Float)));

        EXPECT_FALSE(m_logicEngine->createSkinBinding({ nodeBinding }, { matrix44f{ 0.f } }, *appearanceBinding, *optUniform, "skin"));
        EXPECT_EQ(getLastErrorMessage(), "Cannot create SkinBinding, provided uniform input must be pointing to valid uniform of the provided appearance's effect and must not be bound.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndUniformDataTypeWrong)
    {
        const auto nodeBinding = m_logicEngine->createNodeBinding(*m_node);
        auto appearanceBinding = m_logicEngine->createAppearanceBinding(*m_appearance);

        const auto optUniform = m_appearance->getEffect().findUniformInput("floatUniform");
        ASSERT_TRUE(optUniform.has_value());

        EXPECT_FALSE(m_logicEngine->createSkinBinding({ nodeBinding }, { matrix44f{ 0.f } }, *appearanceBinding, *optUniform, "skin"));
        EXPECT_EQ(getLastErrorMessage(), "Cannot create SkinBinding, provided uniform input must be of type array of Matrix4x4 with element count matching number of joints.");
    }

    TEST_P(ALogicEngine_Factory, ProducesErrorWhenCreatingSkinBindingAndUniformPointsToArrayWithMismatchedSize)
    {
        const auto nodeBinding = m_logicEngine->createNodeBinding(*m_node);

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
        const auto optUniform = otherEffect->findUniformInput("someArray");
        ASSERT_TRUE(optUniform.has_value());
        auto appearanceBinding = m_logicEngine->createAppearanceBinding(*m_scene->createAppearance(*otherEffect));

        EXPECT_FALSE(m_logicEngine->createSkinBinding({ nodeBinding }, { matrix44f{ 0.f } }, *appearanceBinding, *optUniform, "skin"));
        EXPECT_EQ(getLastErrorMessage(), "Cannot create SkinBinding, provided uniform input must be of type array of Matrix4x4 with element count matching number of joints.");
    }

    TEST_P(ALogicEngine_Factory, RenamesObjectsAfterCreation)
    {
        auto script = m_logicEngine->createLuaScript(m_valid_empty_script);
        auto nodeBinding = m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
        auto appearanceBinding = m_logicEngine->createAppearanceBinding(*m_appearance, "AppearanceBinding");
        auto cameraBinding = m_logicEngine->createCameraBinding(*m_camera, "CameraBinding");

        script->setName("same name twice");
        nodeBinding->setName("same name twice");
        appearanceBinding->setName("");
        cameraBinding->setName("");

        EXPECT_EQ("same name twice", script->getName());
        EXPECT_EQ("same name twice", nodeBinding->getName());
        EXPECT_EQ("", appearanceBinding->getName());
        EXPECT_EQ("", cameraBinding->getName());

        auto renderPassBinding = m_logicEngine->createRenderPassBinding(*m_renderPass, "rp");
        renderPassBinding->setName("");
        EXPECT_EQ("", renderPassBinding->getName());

        auto renderGroupBinding = createRenderGroupBinding();
        renderGroupBinding->setName("");
        EXPECT_EQ("", renderGroupBinding->getName());
    }

    TEST_P(ALogicEngine_Factory, CanCastObjectsToValidTypes)
    {
        LogicObject* luaModule         = m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "luaModule");
        LogicObject* luaScript         = m_logicEngine->createLuaScript(m_valid_empty_script, {}, "script");
        LogicObject* nodeBinding       = m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "nodebinding");
        LogicObject* appearanceBinding = m_logicEngine->createAppearanceBinding(*m_appearance, "appbinding");
        LogicObject* cameraBinding     = m_logicEngine->createCameraBinding(*m_camera, "camerabinding");
        LogicObject* dataArray         = m_logicEngine->createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray->as<DataArray>(), dataArray->as<DataArray>(), EInterpolationType::Linear });
        LogicObject* animNode          = m_logicEngine->createAnimationNode(config, "animNode");
        LogicObject* renderPassBinding = m_logicEngine->createRenderPassBinding(*m_renderPass, "rp");
        LogicObject* renderGroupBinding = createRenderGroupBinding();
        LogicObject* skin = createSkinBinding(*m_logicEngine);
        LogicObject* meshBinding = m_logicEngine->createMeshNodeBinding(*m_meshNode);
        LogicObject* rbBinding = m_logicEngine->createRenderBufferBinding(*m_renderBuffer);

        EXPECT_TRUE(luaModule->as<LuaModule>());
        EXPECT_TRUE(luaScript->as<LuaScript>());
        EXPECT_TRUE(nodeBinding->as<NodeBinding>());
        EXPECT_TRUE(appearanceBinding->as<AppearanceBinding>());
        EXPECT_TRUE(cameraBinding->as<CameraBinding>());
        EXPECT_TRUE(dataArray->as<DataArray>());
        EXPECT_TRUE(animNode->as<AnimationNode>());
        EXPECT_TRUE(renderPassBinding->as<RenderPassBinding>());
        EXPECT_TRUE(renderGroupBinding->as<RenderGroupBinding>());
        EXPECT_TRUE(skin->as<SkinBinding>());
        EXPECT_TRUE(meshBinding->as<MeshNodeBinding>());
        EXPECT_TRUE(rbBinding->as<RenderBufferBinding>());

        EXPECT_FALSE(luaModule->as<AnimationNode>());
        EXPECT_FALSE(luaScript->as<DataArray>());
        EXPECT_FALSE(nodeBinding->as<CameraBinding>());
        EXPECT_FALSE(appearanceBinding->as<AnimationNode>());
        EXPECT_FALSE(cameraBinding->as<NodeBinding>());
        EXPECT_FALSE(dataArray->as<LuaScript>());
        EXPECT_FALSE(animNode->as<LuaModule>());
        EXPECT_FALSE(renderPassBinding->as<CameraBinding>());
        EXPECT_FALSE(renderGroupBinding->as<CameraBinding>());
        EXPECT_FALSE(skin->as<CameraBinding>());
        EXPECT_FALSE(meshBinding->as<CameraBinding>());
        EXPECT_FALSE(rbBinding->as<CameraBinding>());

        //cast obj -> node -> binding -> appearanceBinding
        auto* nodeCastFromObject = appearanceBinding->as<LogicNode>();
        EXPECT_TRUE(nodeCastFromObject);
        auto* bindingCastFromNode = nodeCastFromObject->as<RamsesBinding>();
        EXPECT_TRUE(bindingCastFromNode);
        auto* appearanceBindingCastFromBinding = bindingCastFromNode->as<AppearanceBinding>();
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
        m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "luaModule");
        m_logicEngine->createLuaScript(m_valid_empty_script, {}, "script");
        m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "nodebinding");
        m_logicEngine->createAppearanceBinding(*m_appearance, "appbinding");
        m_logicEngine->createCameraBinding(*m_camera, "camerabinding");
        const LogicObject* dataArray = m_logicEngine->createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        AnimationNodeConfig config;
        config.addChannel({ "channel", dataArray->as<DataArray>(), dataArray->as<DataArray>(), EInterpolationType::Linear });
        m_logicEngine->createAnimationNode(config, "animNode");
        m_logicEngine->createRenderPassBinding(*m_renderPass, "renderPass");
        createRenderGroupBinding();
        createSkinBinding(*m_logicEngine);
        m_logicEngine->createMeshNodeBinding(*m_meshNode, "meshBinding");
        m_logicEngine->createRenderBufferBinding(*m_renderBuffer, "rbBinding");

        const auto* immutableLogicEngine = m_logicEngine;
        const auto* luaModuleConst         = immutableLogicEngine->findObject<LogicObject>("luaModule");
        const auto* luaScriptConst         = immutableLogicEngine->findObject<LogicObject>("script");
        const auto* nodeBindingConst       = immutableLogicEngine->findObject<LogicObject>("nodebinding");
        const auto* appearanceBindingConst = immutableLogicEngine->findObject<LogicObject>("appbinding");
        const auto* cameraBindingConst     = immutableLogicEngine->findObject<LogicObject>("camerabinding");
        const auto* dataArrayConst         = immutableLogicEngine->findObject<LogicObject>("dataarray");
        const auto* animNodeConst          = immutableLogicEngine->findObject<LogicObject>("animNode");
        const auto* renderPassBindingConst = immutableLogicEngine->findObject<LogicObject>("renderPass");
        const auto* renderGroupBindingConst = immutableLogicEngine->findObject<LogicObject>("renderGroupBinding");
        const auto* skinConst = immutableLogicEngine->findObject<LogicObject>("skin");
        const auto* meshBindingConst = immutableLogicEngine->findObject<LogicObject>("meshBinding");
        const auto* rbBindingConst = immutableLogicEngine->findObject<LogicObject>("rbBinding");

        EXPECT_TRUE(luaModuleConst->as<LuaModule>());
        EXPECT_TRUE(luaScriptConst->as<LuaScript>());
        EXPECT_TRUE(nodeBindingConst->as<NodeBinding>());
        EXPECT_TRUE(appearanceBindingConst->as<AppearanceBinding>());
        EXPECT_TRUE(cameraBindingConst->as<CameraBinding>());
        EXPECT_TRUE(dataArrayConst->as<DataArray>());
        EXPECT_TRUE(animNodeConst->as<AnimationNode>());
        EXPECT_TRUE(renderPassBindingConst->as<RenderPassBinding>());
        EXPECT_TRUE(renderGroupBindingConst->as<RenderGroupBinding>());
        EXPECT_TRUE(skinConst->as<SkinBinding>());
        EXPECT_TRUE(meshBindingConst->as<MeshNodeBinding>());
        EXPECT_TRUE(rbBindingConst->as<RenderBufferBinding>());

        EXPECT_FALSE(luaModuleConst->as<AnimationNode>());
        EXPECT_FALSE(luaScriptConst->as<DataArray>());
        EXPECT_FALSE(nodeBindingConst->as<CameraBinding>());
        EXPECT_FALSE(appearanceBindingConst->as<AnimationNode>());
        EXPECT_FALSE(cameraBindingConst->as<NodeBinding>());
        EXPECT_FALSE(dataArrayConst->as<LuaScript>());
        EXPECT_FALSE(animNodeConst->as<LuaModule>());
        EXPECT_FALSE(renderPassBindingConst->as<LuaInterface>());
        EXPECT_FALSE(renderGroupBindingConst->as<LuaInterface>());
        EXPECT_FALSE(skinConst->as<CameraBinding>());
        EXPECT_FALSE(meshBindingConst->as<CameraBinding>());
        EXPECT_FALSE(rbBindingConst->as<CameraBinding>());

        // cast obj -> node -> binding -> appearanceBinding
        const auto* nodeCastFromObject = appearanceBindingConst->as<LogicNode>();
        EXPECT_TRUE(nodeCastFromObject);
        const auto* bindingCastFromNode = nodeCastFromObject->as<RamsesBinding>();
        EXPECT_TRUE(bindingCastFromNode);
        const auto* appearanceBindingCastFromBinding = bindingCastFromNode->as<AppearanceBinding>();
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
        struct UnknownObjectImpl: LogicNodeImpl
        {
            explicit UnknownObjectImpl(SceneImpl& scene)
                : LogicNodeImpl(scene, "name", sceneObjectId_t{ 1u })
            {
            }

            std::optional<LogicNodeRuntimeError> update() override { return std::nullopt; }
            void createRootProperties() final {}
        };

        struct UnknownObject : LogicNode
        {
            explicit UnknownObject(std::unique_ptr<UnknownObjectImpl> impl)
                : LogicNode(std::move(impl))
            {
            }
        };

        UnknownObject unknownObject(std::make_unique<UnknownObjectImpl>(m_scene->impl()));
        EXPECT_FALSE(m_logicEngine->destroy(unknownObject));
        EXPECT_EQ(getLastErrorMessage(), "Tried to destroy object 'name' with unknown type");
    }
}
