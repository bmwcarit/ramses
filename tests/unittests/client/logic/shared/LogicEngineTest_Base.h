//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "RamsesTestUtils.h"
#include "WithTempDirectory.h"

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LuaModule.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/RenderGroupBindingElements.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AnchorPoint.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/ramses-utils.h"

namespace ramses::internal
{
    class ALogicEngineBase
    {
    public:
        explicit ALogicEngineBase(EFeatureLevel featureLevel = EFeatureLevel_Latest)
            : m_ramses{ featureLevel }
        {
            // make ramses camera valid, needed for anchor points
            m_camera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 1.f);
            m_renderGroup->addMeshNode(*m_meshNode);
            m_saveFileConfigNoValidation.setValidationEnabled(false);
        }

        virtual ~ALogicEngineBase() = default;

        LogicEngine& getLogicEngine()
        {
            return *m_logicEngine;
        }

        static LuaConfig CreateDeps(const std::vector<std::pair<std::string_view, const LuaModule*>>& dependencies)
        {
            LuaConfig config;
            for (const auto& [alias, module] : dependencies)
            {
                config.addDependency(alias, *module);
            }

            return config;
        }

        static LuaConfig WithStdModules(std::initializer_list<EStandardModule> modules)
        {
            LuaConfig config;
            for (auto m : modules)
            {
                config.addStandardModuleDependency(m);
            }
            return config;
        }

        bool saveToFile(std::string_view filename, const SaveFileConfig& config = {})
        {
            return m_scene->saveToFile(filename, config);
        }

        bool saveToFileWithoutValidation(std::string_view filename)
        {
            SaveFileConfig configNoValidation;
            configNoValidation.setValidationEnabled(false);
            return m_scene->saveToFile(filename, configNoValidation);
        }

        virtual bool recreateFromFile(std::string_view filename)
        {
            const auto nodeId = m_node->getSceneObjectId();
            const auto cameraId = m_camera->getSceneObjectId();
            const auto appearanceId = m_appearance->getSceneObjectId();
            const auto renderPassId = m_renderPass->getSceneObjectId();
            const auto renderGroupId = m_renderGroup->getSceneObjectId();
            const auto meshNodeId = m_meshNode->getSceneObjectId();
            const auto logicEngineId = m_logicEngine->getSceneObjectId();

            m_ramses.destroyScene(*m_scene);
            m_scene = m_ramses.getClient().loadSceneFromFile(filename);
            if (!m_scene)
                throw std::runtime_error("scene not loaded");
            m_node        = m_scene->findObject<Node>(nodeId);
            m_camera      = m_scene->findObject<OrthographicCamera>(cameraId);
            m_appearance  = m_scene->findObject<Appearance>(appearanceId);
            m_renderPass  = m_scene->findObject<ramses::RenderPass>(renderPassId);
            m_renderGroup = m_scene->findObject<ramses::RenderGroup>(renderGroupId);
            m_meshNode    = m_scene->findObject<MeshNode>(meshNodeId);
            m_logicEngine = m_scene->findObject<LogicEngine>(logicEngineId);
            return m_logicEngine != nullptr;
        }

        void expectNoError()
        {
            auto issue = m_ramses.getFramework().getLastError();
            EXPECT_FALSE(issue.has_value()) << issue->message;
        }

        void expectError(std::string_view expectedMsg, const RamsesObject* expectedObj = nullptr)
        {
            const auto issue = m_ramses.getFramework().getLastError();
            ASSERT_TRUE(issue.has_value());
            EXPECT_EQ(expectedMsg, issue->message);
            EXPECT_EQ(expectedObj, issue->object);
        }

        void expectErrorSubstring(std::string_view expectedSubStr, const RamsesObject* expectedObj = nullptr)
        {
            const auto issue = m_ramses.getFramework().getLastError();
            ASSERT_TRUE(issue.has_value());
            EXPECT_THAT(issue->message, ::testing::HasSubstr(expectedSubStr));
            EXPECT_EQ(expectedObj, issue->object);
        }

        std::string getLastErrorMessage()
        {
            const auto issue = m_ramses.getFramework().getLastError();
            EXPECT_TRUE(issue.has_value());
            return issue.value_or(Issue{}).message;
        }

    protected:
        std::unique_ptr<WithTempDirectory> m_withTempDirectory; // must be destroyed after all loaded scenes released the filesystem
        SaveFileConfig m_saveFileConfigNoValidation;
        RamsesTestSetup m_ramses;
        ramses::Scene* m_scene { m_ramses.createScene() };

        ramses::Node* m_node { m_scene->createNode() };
        ramses::OrthographicCamera* m_camera { m_scene->createOrthographicCamera() };
        ramses::Appearance* m_appearance { &RamsesTestSetup::CreateTrivialTestAppearance(*m_scene) };
        ramses::RenderPass* m_renderPass { m_scene->createRenderPass() };
        ramses::RenderGroup* m_renderGroup { m_scene->createRenderGroup() };
        ramses::MeshNode* m_meshNode { m_scene->createMeshNode("meshNode") };

        LogicEngine* m_logicEngine{ m_scene->createLogicEngine("testLogic") };

        const std::string_view m_valid_empty_script = R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
            end
        )";

        const std::string_view m_invalid_empty_script = R"(
        )";

        const std::string_view m_moduleSourceCode = R"(
            local mymath = {}
            function mymath.add(a,b)
                print(a+b)
            end
            return mymath
        )";

        const std::string_view m_interfaceSourceCode = R"(
            function interface(inout_params)
                inout_params.param_vec3f = Type:Vec3f()
            end
        )";

        [[nodiscard]] LogicEngine* recreate()
        {
            const sceneId_t sceneId = m_scene->getSceneId();

            m_ramses.destroyScene(*m_scene);
            m_scene = m_ramses.createScene(sceneId);
            m_node = m_scene->createNode();
            m_camera = m_scene->createOrthographicCamera();
            m_appearance = &RamsesTestSetup::CreateTrivialTestAppearance(*m_scene);
            m_renderPass = m_scene->createRenderPass();
            m_renderGroup = m_scene->createRenderGroup();
            m_meshNode = m_scene->createMeshNode();

            return m_scene->createLogicEngine();
        }

        RenderGroupBinding* createRenderGroupBinding(LogicEngine& logicEngine)
        {
            RenderGroupBindingElements elements;
            EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
            return logicEngine.createRenderGroupBinding(*m_renderGroup, elements, "renderGroupBinding");
        }

        RenderGroupBinding* createRenderGroupBinding()
        {
            return createRenderGroupBinding(*m_logicEngine);
        }

        static SkinBinding* createSkinBinding(const NodeBinding& nodeBinding, AppearanceBinding& appearanceBinding, LogicEngine& logicEngine)
        {
            const auto optUniform = appearanceBinding.getRamsesAppearance().getEffect().findUniformInput("jointMat");
            EXPECT_TRUE(optUniform.has_value());
            assert(optUniform != std::nullopt);
            return logicEngine.createSkinBinding({ &nodeBinding }, { matrix44f{ 0.f } }, appearanceBinding, *optUniform, "skin");
        }

        SkinBinding* createSkinBinding(LogicEngine& logicEngine)
        {
            const auto nodeBinding = logicEngine.createNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodeForSkin");
            auto appearanceBinding = logicEngine.createAppearanceBinding(*m_appearance, "appearanceForSkin");
            return createSkinBinding(*nodeBinding, *appearanceBinding, logicEngine);
        }

        void withTempDirectory()
        {
            m_withTempDirectory = std::make_unique<WithTempDirectory>();
        }

        size_t m_emptySerializedSizeTotal{164u};
    };

    class ALogicEngine : public ALogicEngineBase, public ::testing::Test
    {
    public:
        explicit ALogicEngine(EFeatureLevel featureLevel = EFeatureLevel_Latest)
            : ALogicEngineBase{ featureLevel }
        {
        }
    };
}
