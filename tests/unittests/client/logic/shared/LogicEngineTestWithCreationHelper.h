//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <gtest/gtest.h>

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LuaModule.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/AnimationNodeConfig.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AnchorPoint.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"

#include "LogicEngineTest_Base.h"

namespace ramses::internal
{
    using namespace testing;

    using LogicObjectTypes = ::testing::Types <
        LuaModule,
        LuaScript,
        LuaInterface,
        NodeBinding,
        AppearanceBinding,
        CameraBinding,
        RenderPassBinding,
        RenderGroupBinding,
        MeshNodeBinding,
        SkinBinding,
        DataArray,
        AnimationNode,
        TimerNode,
        AnchorPoint
    >;

    class ALogicEngineBaseWithCreationHelper : public ALogicEngineBase
    {
    public:
        template <typename ObjectType>
        ObjectType* createObjectOfType([[maybe_unused]] std::string_view name)
        {
        }
    };

    template <> inline LuaModule* ALogicEngineBaseWithCreationHelper::createObjectOfType<LuaModule>(std::string_view name)
    {
        return this->m_logicEngine->createLuaModule(this->m_moduleSourceCode, {}, name);
    }
    template <> inline LuaScript* ALogicEngineBaseWithCreationHelper::createObjectOfType<LuaScript>(std::string_view name)
    {
        return this->m_logicEngine->createLuaScript(this->m_valid_empty_script, {}, name);
    }
    template <> inline LuaInterface* ALogicEngineBaseWithCreationHelper::createObjectOfType<LuaInterface>(std::string_view name)
    {
        return this->m_logicEngine->createLuaInterface(this->m_interfaceSourceCode, name);
    }
    template <> inline NodeBinding* ALogicEngineBaseWithCreationHelper::createObjectOfType<NodeBinding>(std::string_view name)
    {
        return this->m_logicEngine->createNodeBinding(*this->m_node, ERotationType::Euler_XYZ, name);
    }
    template <> inline AppearanceBinding* ALogicEngineBaseWithCreationHelper::createObjectOfType<AppearanceBinding>(std::string_view name)
    {
        return this->m_logicEngine->createAppearanceBinding(*this->m_appearance, name);
    }
    template <> inline CameraBinding* ALogicEngineBaseWithCreationHelper::createObjectOfType<CameraBinding>(std::string_view name)
    {
        return this->m_logicEngine->createCameraBinding(*this->m_camera, name);
    }
    template <> inline RenderPassBinding* ALogicEngineBaseWithCreationHelper::createObjectOfType<RenderPassBinding>(std::string_view name)
    {
        return this->m_logicEngine->createRenderPassBinding(*this->m_renderPass, name);
    }
    template <> inline RenderGroupBinding* ALogicEngineBaseWithCreationHelper::createObjectOfType<RenderGroupBinding>(std::string_view name)
    {
        RenderGroupBindingElements elements;
        EXPECT_TRUE(elements.addElement(*this->m_meshNode, "mesh"));
        return this->m_logicEngine->createRenderGroupBinding(*this->m_renderGroup, elements, name);
    }
    template <> inline MeshNodeBinding* ALogicEngineBaseWithCreationHelper::createObjectOfType<MeshNodeBinding>(std::string_view name)
    {
        return this->m_logicEngine->createMeshNodeBinding(*this->m_meshNode, name);
    }
    template <> inline SkinBinding* ALogicEngineBaseWithCreationHelper::createObjectOfType<SkinBinding>(std::string_view name)
    {
        const auto nodeBinding = this->m_logicEngine->createNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodeForSkin");
        auto appearanceBinding = this->m_logicEngine->createAppearanceBinding(*m_appearance, "appearanceForSkin");
        const auto optUniform = appearanceBinding->getRamsesAppearance().getEffect().findUniformInput("jointMat");
        assert(optUniform != std::nullopt);
        return this->m_logicEngine->createSkinBinding({ nodeBinding }, { matrix44f{ 0.f } }, *appearanceBinding, *optUniform, name);
    }
    template <> inline DataArray* ALogicEngineBaseWithCreationHelper::createObjectOfType<DataArray>(std::string_view name)
    {
        return this->m_logicEngine->createDataArray<int32_t>({1}, name);
    }
    template <> inline AnimationNode* ALogicEngineBaseWithCreationHelper::createObjectOfType<AnimationNode>(std::string_view name)
    {
        auto* data = this->m_logicEngine->createDataArray(std::vector<float>{ 0.f, 1.f });
        const AnimationChannel channel{ "channel", data, data, EInterpolationType::Step };
        AnimationNodeConfig config;
        config.addChannel(channel);
        return this->m_logicEngine->createAnimationNode(config, name);
    }
    template <> inline TimerNode* ALogicEngineBaseWithCreationHelper::createObjectOfType<TimerNode>(std::string_view name)
    {
        return this->m_logicEngine->createTimerNode(name);
    }
    template <> inline AnchorPoint* ALogicEngineBaseWithCreationHelper::createObjectOfType<AnchorPoint>(std::string_view name)
    {
        const auto nodeBinding = this->m_logicEngine->createNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodeForAnchor");
        const auto camBinding = this->m_logicEngine->createCameraBinding(*this->m_camera, "camForAnchor");
        return this->m_logicEngine->createAnchorPoint(*nodeBinding, *camBinding, name);
    }
}
