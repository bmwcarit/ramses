//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "LogicViewer.h"
#include "RamsesTestUtils.h"
#include "WithTempDirectory.h"
#include <fstream>
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/AnchorPoint.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/ramses-client.h"
#include <fmt/format.h>


namespace ramses::internal
{
    class ALogicViewerBase : public ::testing::Test
    {
    public:

        class MockScreenshot
        {
        public:
            MockScreenshot()
            {
                SetMockScreenshot(this);
            }

            ~MockScreenshot()
            {
                SetMockScreenshot(nullptr);
            }

            MOCK_METHOD(bool, screenshot, (const std::string&));
        };

        ALogicViewerBase()
        {
            m_mockScreenshot = nullptr;
        }

        Result loadLua(std::string_view lua)
        {
            const char* filename = "logicviewertest.lua";
            {
                std::ofstream fileStream(filename, std::ofstream::binary);
                if (!fileStream.is_open())
                {
                    throw std::runtime_error("filestream not open");
                }
                fileStream << lua;
                if (fileStream.bad())
                {
                    throw std::runtime_error("bad filestream");
                }
            }
            auto result = viewer.loadLuaFile(filename);
            if (result.ok())
            {
                result = viewer.update();
            }
            return result;
        }

        static bool doScreenshot(const std::string& filename)
        {
            return (m_mockScreenshot != nullptr) ? m_mockScreenshot->screenshot(filename) : false;
        }

        template<class T>
        T* getNode(std::string_view nodeName)
        {
            auto* node = viewer.getLogic().findObject<T>(nodeName);
            if (node == nullptr)
            {
                throw(std::runtime_error(fmt::format("Node not found: '{}'", nodeName)));
            }
            return node;
        }

        static ramses::Property* GetInput(ramses::LogicNode* node, std::string_view propertyName)
        {
            auto* property = node->getInputs()->getChild(propertyName);
            if (property == nullptr)
            {
                throw(std::runtime_error(fmt::format("Input property '{}' not found in node '{}'", propertyName, node->getName())));
            }
            return property;
        }

        template<class T>
        ramses::Property* getInput(std::string_view nodeName, std::string_view propertyName)
        {
            return GetInput(getNode<T>(nodeName), propertyName);
        }

        static const ramses::Property* GetOutput(const ramses::LogicNode* node, std::string_view propertyName)
        {
            const auto* property = node->getOutputs()->getChild(propertyName);
            if (property == nullptr)
            {
                throw(std::runtime_error(fmt::format("Output property '{}' not found in node '{}'", propertyName, node->getName())));
            }
            return property;
        }

        template<class T>
        const ramses::Property* getOutput(std::string_view nodeName, std::string_view propertyName)
        {
            return GetOutput(getNode<T>(nodeName), propertyName);
        }

        static void SetMockScreenshot(MockScreenshot* mock)
        {
            m_mockScreenshot = mock;
        }

    protected:
        WithTempDirectory m_withTempDirectory;
        RamsesTestSetup m_ramses;
        ramses::Scene* m_scene = { m_ramses.createScene() };
        ramses::Node* m_node = { m_scene->createNode() };
        ramses::OrthographicCamera* m_camera = { m_scene->createOrthographicCamera() };
        ramses::Appearance* m_appearance = { &RamsesTestSetup::CreateTrivialTestAppearance(*m_scene) };
        ramses::RenderPass* m_renderPass = { m_scene->createRenderPass() };
        ramses::RenderGroup* m_renderGroup = { m_scene->createRenderGroup() };
        ramses::RenderGroup* m_nestedRenderGroup = { m_scene->createRenderGroup() };
        ramses::MeshNode* m_meshNode = { m_scene->createMeshNode() };
        ramses::LogicEngine* m_logic = { m_scene->createLogicEngine() };
        LogicViewer viewer{ *m_logic, doScreenshot };

    private:
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) must be static and non-const (see SetMockScreenshot)
        static MockScreenshot* m_mockScreenshot;
    };
}
