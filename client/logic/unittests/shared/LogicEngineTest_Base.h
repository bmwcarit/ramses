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

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaModule.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/LuaInterface.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"
#include "ramses-logic/RamsesRenderPassBinding.h"
#include "ramses-logic/RamsesRenderGroupBinding.h"
#include "ramses-logic/RamsesRenderGroupBindingElements.h"
#include "ramses-logic/RamsesMeshNodeBinding.h"
#include "ramses-logic/DataArray.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/TimerNode.h"
#include "ramses-logic/AnchorPoint.h"
#include "ramses-logic/SkinBinding.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-utils.h"

namespace rlogic
{
    class ALogicEngineBase
    {
    public:
        explicit ALogicEngineBase(EFeatureLevel featureLevel = EFeatureLevel_01)
            : m_logicEngine{ featureLevel }
        {
            // make ramses camera valid, needed for anchor points
            m_camera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 1.f);
            m_renderGroup->addMeshNode(*m_meshNode);
            m_saveFileConfigNoValidation.setValidationEnabled(false);
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

        static bool SaveToFileWithoutValidation(LogicEngine& logicEngine, std::string_view filename)
        {
            SaveFileConfig configNoValidation;
            configNoValidation.setValidationEnabled(false);
            return logicEngine.saveToFile(filename, configNoValidation);
        }

    protected:
        LogicEngine m_logicEngine;
        SaveFileConfig m_saveFileConfigNoValidation;
        RamsesTestSetup m_ramses;
        ramses::Scene* m_scene = { m_ramses.createScene() };
        ramses::Node* m_node = { m_scene->createNode() };
        ramses::OrthographicCamera* m_camera = { m_scene->createOrthographicCamera() };
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

        void recreate()
        {
            const ramses::sceneId_t sceneId = m_scene->getSceneId();

            m_ramses.destroyScene(*m_scene);
            m_scene = m_ramses.createScene(sceneId);
            m_node = m_scene->createNode();
            m_camera = m_scene->createOrthographicCamera();
            m_appearance = &RamsesTestSetup::CreateTrivialTestAppearance(*m_scene);
            m_renderPass = m_scene->createRenderPass();
            m_renderGroup = m_scene->createRenderGroup();
            m_meshNode = m_scene->createMeshNode();
        }

        RamsesRenderGroupBinding* createRenderGroupBinding(LogicEngine& logicEngine)
        {
            RamsesRenderGroupBindingElements elements;
            EXPECT_TRUE(elements.addElement(*m_meshNode, "mesh"));
            return logicEngine.createRamsesRenderGroupBinding(*m_renderGroup, elements, "renderGroupBinding");
        }

        RamsesRenderGroupBinding* createRenderGroupBinding()
        {
            return createRenderGroupBinding(m_logicEngine);
        }

        static SkinBinding* createSkinBinding(const RamsesNodeBinding& nodeBinding, RamsesAppearanceBinding& appearanceBinding, LogicEngine& logicEngine)
        {
            ramses::UniformInput uniform;
            appearanceBinding.getRamsesAppearance().getEffect().findUniformInput("jointMat", uniform);
            EXPECT_TRUE(uniform.isValid());
            return logicEngine.createSkinBinding({ &nodeBinding }, { matrix44f{ 0.f } }, appearanceBinding, uniform, "skin");
        }

        SkinBinding* createSkinBinding(LogicEngine& logicEngine)
        {
            const auto nodeBinding = logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodeForSkin");
            auto appearanceBinding = logicEngine.createRamsesAppearanceBinding(*m_appearance, "appearanceForSkin");
            return createSkinBinding(*nodeBinding, *appearanceBinding, logicEngine);
        }

        size_t m_emptySerializedSizeTotal{164u};
    };

    class ALogicEngine : public ALogicEngineBase, public ::testing::Test
    {
    public:
        explicit ALogicEngine(EFeatureLevel featureLevel = EFeatureLevel_01)
            : ALogicEngineBase{ featureLevel }
        {
        }
    };
}
