//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"
#include "ramses-logic/AnimationNodeConfig.h"

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
#include "impl/AnimationNodeImpl.h"
#include "impl/TimerNodeImpl.h"
#include "impl/AnchorPointImpl.h"
#include "impl/SkinBindingImpl.h"

namespace rlogic
{
    class ALogicEngine_Lookup : public ALogicEngine
    {
    public:
        ALogicEngine_Lookup() : ALogicEngine{ EFeatureLevel_Latest } // test with latest feature level so all possible API objects are available
        {
        }

    protected:
        AnimationNode* createAnimationNode(const DataArray* dataArray)
        {
            AnimationNodeConfig config;
            config.addChannel({ "channel", dataArray, dataArray });
            return m_logicEngine.createAnimationNode(config, "animNode");
        }
    };

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirName)
    {
        const auto luaModule = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        const auto script = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        const auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        const auto cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto renderPassBinding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rpbinding");
        const auto renderGroupBinding = createRenderGroupBinding();
        const auto meshNodeBinding = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "mb");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "dataarray");
        const auto animNode = createAnimationNode(dataArray);
        const auto timerNode = m_logicEngine.createTimerNode("timerNode");
        const auto intf = m_logicEngine.createLuaInterface(m_interfaceSourceCode, "intf");
        const auto anchor = m_logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
        const auto skin = createSkinBinding(*nodeBinding, *appearanceBinding, m_logicEngine);

        EXPECT_EQ(luaModule, m_logicEngine.findByName<LuaModule>("luaModule"));
        EXPECT_EQ(script, m_logicEngine.findByName<LuaScript>("script"));
        EXPECT_EQ(nodeBinding, m_logicEngine.findByName<RamsesNodeBinding>("nodebinding"));
        EXPECT_EQ(appearanceBinding, m_logicEngine.findByName<RamsesAppearanceBinding>("appbinding"));
        EXPECT_EQ(cameraBinding, m_logicEngine.findByName<RamsesCameraBinding>("camerabinding"));
        EXPECT_EQ(renderPassBinding, m_logicEngine.findByName<RamsesRenderPassBinding>("rpbinding"));
        EXPECT_EQ(renderGroupBinding, m_logicEngine.findByName<RamsesRenderGroupBinding>("renderGroupBinding"));
        EXPECT_EQ(meshNodeBinding, m_logicEngine.findByName<RamsesMeshNodeBinding>("mb"));
        EXPECT_EQ(dataArray, m_logicEngine.findByName<DataArray>("dataarray"));
        EXPECT_EQ(animNode, m_logicEngine.findByName<AnimationNode>("animNode"));
        EXPECT_EQ(timerNode, m_logicEngine.findByName<TimerNode>("timerNode"));
        EXPECT_EQ(intf, m_logicEngine.findByName<LuaInterface>("intf"));
        EXPECT_EQ(anchor, m_logicEngine.findByName<AnchorPoint>("anchor"));
        EXPECT_EQ(skin, m_logicEngine.findByName<SkinBinding>("skin"));

        EXPECT_EQ(luaModule, m_logicEngine.findByName<LogicObject>("luaModule"));
        EXPECT_EQ(script, m_logicEngine.findByName<LogicObject>("script"));
        EXPECT_EQ(nodeBinding, m_logicEngine.findByName<LogicObject>("nodebinding"));
        EXPECT_EQ(appearanceBinding, m_logicEngine.findByName<LogicObject>("appbinding"));
        EXPECT_EQ(cameraBinding, m_logicEngine.findByName<LogicObject>("camerabinding"));
        EXPECT_EQ(renderPassBinding, m_logicEngine.findByName<LogicObject>("rpbinding"));
        EXPECT_EQ(renderGroupBinding, m_logicEngine.findByName<LogicObject>("renderGroupBinding"));
        EXPECT_EQ(meshNodeBinding, m_logicEngine.findByName<LogicObject>("mb"));
        EXPECT_EQ(dataArray, m_logicEngine.findByName<LogicObject>("dataarray"));
        EXPECT_EQ(animNode, m_logicEngine.findByName<LogicObject>("animNode"));
        EXPECT_EQ(timerNode, m_logicEngine.findByName<LogicObject>("timerNode"));
        EXPECT_EQ(intf, m_logicEngine.findByName<LogicObject>("intf"));
        EXPECT_EQ(anchor, m_logicEngine.findByName<LogicObject>("anchor"));
        EXPECT_EQ(skin, m_logicEngine.findByName<LogicObject>("skin"));

        auto it = m_logicEngine.getCollection<LogicObject>().cbegin();
        EXPECT_EQ(*it++, luaModule);
        EXPECT_EQ(*it++, script);
        EXPECT_EQ(*it++, nodeBinding);
        EXPECT_EQ(*it++, appearanceBinding);
        EXPECT_EQ(*it++, cameraBinding);
        EXPECT_EQ(*it++, renderPassBinding);
        EXPECT_EQ(*it++, renderGroupBinding);
        EXPECT_EQ(*it++, meshNodeBinding);
        EXPECT_EQ(*it++, dataArray);
        EXPECT_EQ(*it++, animNode);
        EXPECT_EQ(*it++, timerNode);
        EXPECT_EQ(*it++, intf);
        EXPECT_EQ(*it++, anchor);
        EXPECT_EQ(*it++, skin);
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirName_Const)
    {
        const auto luaModule = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        const auto script = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        const auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        const auto cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto renderPassBinding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rpbinding");
        const auto renderGroupBinding = createRenderGroupBinding();
        const auto meshNodeBinding = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "mb");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "dataarray");
        const auto animNode = createAnimationNode(dataArray);
        const auto timerNode = m_logicEngine.createTimerNode("timerNode");
        const auto intf = m_logicEngine.createLuaInterface(m_interfaceSourceCode, "intf");
        const auto anchor = m_logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
        const auto skin = createSkinBinding(*nodeBinding, *appearanceBinding, m_logicEngine);

        const LogicEngine& immutableLogicEngine = m_logicEngine;
        EXPECT_EQ(luaModule, immutableLogicEngine.findByName<LuaModule>("luaModule"));
        EXPECT_EQ(script, immutableLogicEngine.findByName<LuaScript>("script"));
        EXPECT_EQ(nodeBinding, immutableLogicEngine.findByName<RamsesNodeBinding>("nodebinding"));
        EXPECT_EQ(appearanceBinding, immutableLogicEngine.findByName<RamsesAppearanceBinding>("appbinding"));
        EXPECT_EQ(cameraBinding, immutableLogicEngine.findByName<RamsesCameraBinding>("camerabinding"));
        EXPECT_EQ(renderPassBinding, immutableLogicEngine.findByName<RamsesRenderPassBinding>("rpbinding"));
        EXPECT_EQ(renderGroupBinding, immutableLogicEngine.findByName<RamsesRenderGroupBinding>("renderGroupBinding"));
        EXPECT_EQ(meshNodeBinding, immutableLogicEngine.findByName<RamsesMeshNodeBinding>("mb"));
        EXPECT_EQ(dataArray, immutableLogicEngine.findByName<DataArray>("dataarray"));
        EXPECT_EQ(animNode, immutableLogicEngine.findByName<AnimationNode>("animNode"));
        EXPECT_EQ(timerNode, immutableLogicEngine.findByName<TimerNode>("timerNode"));
        EXPECT_EQ(intf, immutableLogicEngine.findByName<LuaInterface>("intf"));
        EXPECT_EQ(anchor, immutableLogicEngine.findByName<AnchorPoint>("anchor"));
        EXPECT_EQ(skin, immutableLogicEngine.findByName<SkinBinding>("skin"));

        EXPECT_EQ(luaModule, immutableLogicEngine.findByName<LogicObject>("luaModule"));
        EXPECT_EQ(script, immutableLogicEngine.findByName<LogicObject>("script"));
        EXPECT_EQ(nodeBinding, immutableLogicEngine.findByName<LogicObject>("nodebinding"));
        EXPECT_EQ(appearanceBinding, immutableLogicEngine.findByName<LogicObject>("appbinding"));
        EXPECT_EQ(cameraBinding, immutableLogicEngine.findByName<LogicObject>("camerabinding"));
        EXPECT_EQ(renderPassBinding, immutableLogicEngine.findByName<LogicObject>("rpbinding"));
        EXPECT_EQ(renderGroupBinding, immutableLogicEngine.findByName<LogicObject>("renderGroupBinding"));
        EXPECT_EQ(meshNodeBinding, immutableLogicEngine.findByName<LogicObject>("mb"));
        EXPECT_EQ(dataArray, immutableLogicEngine.findByName<LogicObject>("dataarray"));
        EXPECT_EQ(animNode, immutableLogicEngine.findByName<LogicObject>("animNode"));
        EXPECT_EQ(timerNode, immutableLogicEngine.findByName<LogicObject>("timerNode"));
        EXPECT_EQ(intf, immutableLogicEngine.findByName<LogicObject>("intf"));
        EXPECT_EQ(anchor, immutableLogicEngine.findByName<LogicObject>("anchor"));
        EXPECT_EQ(skin, immutableLogicEngine.findByName<LogicObject>("skin"));
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirName_CanBeUsedWithRealType)
    {
        m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        const auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        const auto cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rpbinding");
        createRenderGroupBinding();
        m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "mb");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        createAnimationNode(dataArray);
        m_logicEngine.createTimerNode("timerNode");
        m_logicEngine.createLuaInterface(m_interfaceSourceCode, "intf");
        m_logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
        createSkinBinding(*nodeBinding, *appearanceBinding, m_logicEngine);

        const auto* luaModuleFound         = m_logicEngine.findByName<LogicObject>("luaModule")->as<LuaModule>();
        const auto* luaScriptFound         = m_logicEngine.findByName<LogicObject>("script")->as<LuaScript>();
        const auto* nodeBindingFound       = m_logicEngine.findByName<LogicObject>("nodebinding")->as<RamsesNodeBinding>();
        const auto* appearanceBindingFound = m_logicEngine.findByName<LogicObject>("appbinding")->as<RamsesAppearanceBinding>();
        const auto* cameraBindingFound     = m_logicEngine.findByName<LogicObject>("camerabinding")->as<RamsesCameraBinding>();
        const auto* renderPassBindingFound = m_logicEngine.findByName<LogicObject>("rpbinding")->as<RamsesRenderPassBinding>();
        const auto* renderGroupBindingFound = m_logicEngine.findByName<LogicObject>("renderGroupBinding")->as<RamsesRenderGroupBinding>();
        const auto* meshNodeBindingFound   = m_logicEngine.findByName<LogicObject>("mb")->as<RamsesMeshNodeBinding>();
        const auto* dataArrayFound         = m_logicEngine.findByName<LogicObject>("dataarray")->as<DataArray>();
        const auto* animNodeFound          = m_logicEngine.findByName<LogicObject>("animNode")->as<AnimationNode>();
        const auto* timerNodeFound         = m_logicEngine.findByName<LogicObject>("timerNode")->as<TimerNode>();
        const auto* intfFound              = m_logicEngine.findByName<LogicObject>("intf")->as<LuaInterface>();
        const auto* anchorFound            = m_logicEngine.findByName<LogicObject>("anchor")->as<AnchorPoint>();
        const auto* skinFound              = m_logicEngine.findByName<LogicObject>("skin")->as<SkinBinding>();

        ASSERT_NE(nullptr, luaModuleFound);
        ASSERT_NE(nullptr, luaScriptFound);
        ASSERT_NE(nullptr, nodeBindingFound);
        ASSERT_NE(nullptr, appearanceBindingFound);
        ASSERT_NE(nullptr, cameraBindingFound);
        ASSERT_NE(nullptr, renderPassBindingFound);
        ASSERT_NE(nullptr, renderGroupBindingFound);
        ASSERT_NE(nullptr, meshNodeBindingFound);
        ASSERT_NE(nullptr, dataArrayFound);
        ASSERT_NE(nullptr, animNodeFound);
        ASSERT_NE(nullptr, timerNodeFound);
        ASSERT_NE(nullptr, intfFound);
        ASSERT_NE(nullptr, anchorFound);
        ASSERT_NE(nullptr, skinFound);

        EXPECT_EQ(luaModuleFound->getName(), "luaModule");
        EXPECT_EQ(luaScriptFound->getName(), "script");
        EXPECT_EQ(nodeBindingFound->getName(), "nodebinding");
        EXPECT_EQ(appearanceBindingFound->getName(), "appbinding");
        EXPECT_EQ(cameraBindingFound->getName(), "camerabinding");
        EXPECT_EQ(renderPassBindingFound->getName(), "rpbinding");
        EXPECT_EQ(renderGroupBindingFound->getName(), "renderGroupBinding");
        EXPECT_EQ(meshNodeBindingFound->getName(), "mb");
        EXPECT_EQ(dataArrayFound->getName(), "dataarray");
        EXPECT_EQ(animNodeFound->getName(), "animNode");
        EXPECT_EQ(timerNodeFound->getName(), "timerNode");
        EXPECT_EQ(intfFound->getName(), "intf");
        EXPECT_EQ(anchorFound->getName(), "anchor");
        EXPECT_EQ(skinFound->getName(), "skin");
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirName_CanBeUsedAsRealType_Const)
    {
        m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        const auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        const auto cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rpbinding");
        createRenderGroupBinding();
        m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "mb");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        createAnimationNode(dataArray);
        m_logicEngine.createTimerNode("timerNode");
        m_logicEngine.createLuaInterface(m_interfaceSourceCode, "intf");
        m_logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
        createSkinBinding(*nodeBinding, *appearanceBinding, m_logicEngine);

        const LogicEngine& immutableLogicEngine   = m_logicEngine;
        const auto*        luaModuleFound         = immutableLogicEngine.findByName<LogicObject>("luaModule")->as<LuaModule>();
        const auto*        luaScriptFound         = immutableLogicEngine.findByName<LogicObject>("script")->as<LuaScript>();
        const auto*        nodeBindingFound       = immutableLogicEngine.findByName<LogicObject>("nodebinding")->as<RamsesNodeBinding>();
        const auto*        appearanceBindingFound = immutableLogicEngine.findByName<LogicObject>("appbinding")->as<RamsesAppearanceBinding>();
        const auto*        cameraBindingFound     = immutableLogicEngine.findByName<LogicObject>("camerabinding")->as<RamsesCameraBinding>();
        const auto*        renderPassBindingFound = immutableLogicEngine.findByName<LogicObject>("rpbinding")->as<RamsesRenderPassBinding>();
        const auto*        renderGroupBindingFound = immutableLogicEngine.findByName<LogicObject>("renderGroupBinding")->as<RamsesRenderGroupBinding>();
        const auto*        meshNodeBindingFound   = immutableLogicEngine.findByName<LogicObject>("mb")->as<RamsesMeshNodeBinding>();
        const auto*        dataArrayFound         = immutableLogicEngine.findByName<LogicObject>("dataarray")->as<DataArray>();
        const auto*        animNodeFound          = immutableLogicEngine.findByName<LogicObject>("animNode")->as<AnimationNode>();
        const auto*        timerNodeFound         = immutableLogicEngine.findByName<LogicObject>("timerNode")->as<TimerNode>();
        const auto*        intfFound              = immutableLogicEngine.findByName<LogicObject>("intf")->as<LuaInterface>();
        const auto*        anchorFound            = immutableLogicEngine.findByName<LogicObject>("anchor")->as<AnchorPoint>();
        const auto*        skinFound              = immutableLogicEngine.findByName<LogicObject>("skin")->as<SkinBinding>();

        ASSERT_NE(nullptr, luaModuleFound);
        ASSERT_NE(nullptr, luaScriptFound);
        ASSERT_NE(nullptr, nodeBindingFound);
        ASSERT_NE(nullptr, appearanceBindingFound);
        ASSERT_NE(nullptr, cameraBindingFound);
        ASSERT_NE(nullptr, renderPassBindingFound);
        ASSERT_NE(nullptr, renderGroupBindingFound);
        ASSERT_NE(nullptr, meshNodeBindingFound);
        ASSERT_NE(nullptr, dataArrayFound);
        ASSERT_NE(nullptr, animNodeFound);
        ASSERT_NE(nullptr, timerNodeFound);
        ASSERT_NE(nullptr, intfFound);
        ASSERT_NE(nullptr, anchorFound);
        ASSERT_NE(nullptr, skinFound);

        EXPECT_EQ(luaModuleFound->getName(), "luaModule");
        EXPECT_EQ(luaScriptFound->getName(), "script");
        EXPECT_EQ(nodeBindingFound->getName(), "nodebinding");
        EXPECT_EQ(appearanceBindingFound->getName(), "appbinding");
        EXPECT_EQ(cameraBindingFound->getName(), "camerabinding");
        EXPECT_EQ(renderPassBindingFound->getName(), "rpbinding");
        EXPECT_EQ(renderGroupBindingFound->getName(), "renderGroupBinding");
        EXPECT_EQ(meshNodeBindingFound->getName(), "mb");
        EXPECT_EQ(dataArrayFound->getName(), "dataarray");
        EXPECT_EQ(animNodeFound->getName(), "animNode");
        EXPECT_EQ(timerNodeFound->getName(), "timerNode");
        EXPECT_EQ(intfFound->getName(), "intf");
        EXPECT_EQ(anchorFound->getName(), "anchor");
        EXPECT_EQ(skinFound->getName(), "skin");
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirId)
    {
        const auto luaModule         = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        const auto script            = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        const auto nodeBinding       = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        const auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        const auto cameraBinding     = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto renderPassBinding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rpbinding");
        const auto renderGroupBinding = createRenderGroupBinding();
        const auto meshBinding       = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "mb");
        const auto dataArray         = m_logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        const auto animNode          = createAnimationNode(dataArray);
        const auto timerNode         = m_logicEngine.createTimerNode("timerNode");
        const auto intf              = m_logicEngine.createLuaInterface(m_interfaceSourceCode, "intf");
        const auto anchor            = m_logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
        const auto skin              = createSkinBinding(*nodeBinding, *appearanceBinding, m_logicEngine);

        EXPECT_EQ(luaModule, m_logicEngine.findLogicObjectById(1u));
        EXPECT_EQ(script, m_logicEngine.findLogicObjectById(2u));
        EXPECT_EQ(nodeBinding, m_logicEngine.findLogicObjectById(3u));
        EXPECT_EQ(appearanceBinding, m_logicEngine.findLogicObjectById(4u));
        EXPECT_EQ(cameraBinding, m_logicEngine.findLogicObjectById(5u));
        EXPECT_EQ(renderPassBinding, m_logicEngine.findLogicObjectById(6u));
        EXPECT_EQ(renderGroupBinding, m_logicEngine.findLogicObjectById(7u));
        EXPECT_EQ(meshBinding, m_logicEngine.findLogicObjectById(8u));
        EXPECT_EQ(dataArray, m_logicEngine.findLogicObjectById(9u));
        EXPECT_EQ(animNode, m_logicEngine.findLogicObjectById(10u));
        EXPECT_EQ(timerNode, m_logicEngine.findLogicObjectById(11u));
        EXPECT_EQ(intf, m_logicEngine.findLogicObjectById(12u));
        EXPECT_EQ(anchor, m_logicEngine.findLogicObjectById(13u));
        EXPECT_EQ(skin, m_logicEngine.findLogicObjectById(14u));
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirId_Const)
    {
        const auto luaModule         = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        const auto script            = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        const auto nodeBinding       = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        const auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        const auto cameraBinding     = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto renderPassBinding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rpbinding");
        const auto renderGroupBinding = createRenderGroupBinding();
        const auto meshBinding       = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "mb");
        const auto dataArray         = m_logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        const auto animNode          = createAnimationNode(dataArray);
        const auto timerNode         = m_logicEngine.createTimerNode("timerNode");
        const auto intf              = m_logicEngine.createLuaInterface(m_interfaceSourceCode, "intf");
        const auto anchor            = m_logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
        const auto skin              = createSkinBinding(*nodeBinding, *appearanceBinding, m_logicEngine);

        const LogicEngine& immutableLogicEngine = m_logicEngine;
        EXPECT_EQ(luaModule, immutableLogicEngine.findLogicObjectById(1u));
        EXPECT_EQ(script, immutableLogicEngine.findLogicObjectById(2u));
        EXPECT_EQ(nodeBinding, immutableLogicEngine.findLogicObjectById(3u));
        EXPECT_EQ(appearanceBinding, immutableLogicEngine.findLogicObjectById(4u));
        EXPECT_EQ(cameraBinding, immutableLogicEngine.findLogicObjectById(5u));
        EXPECT_EQ(renderPassBinding, immutableLogicEngine.findLogicObjectById(6u));
        EXPECT_EQ(renderGroupBinding, immutableLogicEngine.findLogicObjectById(7u));
        EXPECT_EQ(meshBinding, immutableLogicEngine.findLogicObjectById(8u));
        EXPECT_EQ(dataArray, immutableLogicEngine.findLogicObjectById(9u));
        EXPECT_EQ(animNode, immutableLogicEngine.findLogicObjectById(10u));
        EXPECT_EQ(timerNode, immutableLogicEngine.findLogicObjectById(11u));
        EXPECT_EQ(intf, immutableLogicEngine.findLogicObjectById(12u));
        EXPECT_EQ(anchor, immutableLogicEngine.findLogicObjectById(13u));
        EXPECT_EQ(skin, immutableLogicEngine.findLogicObjectById(14u));
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirName_CutsNameAtNullTermination)
    {
        RamsesAppearanceBinding* appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        EXPECT_EQ(appearanceBinding, m_logicEngine.findByName<RamsesAppearanceBinding>("appbinding\0withsurprise"));
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsAfterRenaming_ByNewNameOnly)
    {
        LuaModule* luaModule = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        LuaScript* script = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        RamsesNodeBinding* nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        RamsesAppearanceBinding* appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        RamsesCameraBinding* cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        RamsesRenderPassBinding* renderPassBinding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rpbinding");
        auto renderGroupBinding = createRenderGroupBinding();
        auto meshBinding = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "meshbinding");
        auto dataArray = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "dataarray");
        auto animNode = createAnimationNode(dataArray);
        auto timerNode = m_logicEngine.createTimerNode("timerNode");
        auto intf = m_logicEngine.createLuaInterface(m_interfaceSourceCode, "intf");
        auto anchor = m_logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
        auto skin = createSkinBinding(*nodeBinding, *appearanceBinding, m_logicEngine);

        // Rename
        luaModule->setName("L");
        script->setName("S");
        nodeBinding->setName("NB");
        appearanceBinding->setName("AB");
        cameraBinding->setName("CB");
        renderPassBinding->setName("RPB");
        renderGroupBinding->setName("RGB");
        meshBinding->setName("MB");
        dataArray->setName("DA");
        animNode->setName("AN");
        timerNode->setName("TN");
        intf->setName("I");
        anchor->setName("A");
        skin->setName("SB");

        // Can't find by old name
        EXPECT_EQ(nullptr, m_logicEngine.findByName<LuaModule>("luaModule"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<LuaScript>("script"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesNodeBinding>("nodebinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesAppearanceBinding>("appbinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesCameraBinding>("camerabinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesRenderPassBinding>("rpbinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesRenderGroupBinding>("renderGroupBinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesMeshNodeBinding>("meshbinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<DataArray>("dataarray"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<AnimationNode>("animNode"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<TimerNode>("timerNode"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<LuaInterface>("intf"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<AnchorPoint>("anchor"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<SkinBinding>("skin"));

        // Found by new name
        EXPECT_EQ(luaModule, m_logicEngine.findByName<LuaModule>("L"));
        EXPECT_EQ(script, m_logicEngine.findByName<LuaScript>("S"));
        EXPECT_EQ(nodeBinding, m_logicEngine.findByName<RamsesNodeBinding>("NB"));
        EXPECT_EQ(appearanceBinding, m_logicEngine.findByName<RamsesAppearanceBinding>("AB"));
        EXPECT_EQ(cameraBinding, m_logicEngine.findByName<RamsesCameraBinding>("CB"));
        EXPECT_EQ(renderPassBinding, m_logicEngine.findByName<RamsesRenderPassBinding>("RPB"));
        EXPECT_EQ(renderGroupBinding, m_logicEngine.findByName<RamsesRenderGroupBinding>("RGB"));
        EXPECT_EQ(meshBinding, m_logicEngine.findByName<RamsesMeshNodeBinding>("MB"));
        EXPECT_EQ(dataArray, m_logicEngine.findByName<DataArray>("DA"));
        EXPECT_EQ(animNode, m_logicEngine.findByName<AnimationNode>("AN"));
        EXPECT_EQ(timerNode, m_logicEngine.findByName<TimerNode>("TN"));
        EXPECT_EQ(intf, m_logicEngine.findByName<LuaInterface>("I"));
        EXPECT_EQ(anchor, m_logicEngine.findByName<AnchorPoint>("A"));
        EXPECT_EQ(skin, m_logicEngine.findByName<SkinBinding>("SB"));
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectByNameOnlyIfTypeMatches)
    {
        m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        const auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        const auto cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rpbinding");
        createRenderGroupBinding();
        m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "meshbinding");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "dataarray");
        createAnimationNode(dataArray);
        m_logicEngine.createTimerNode("timerNode");
        m_logicEngine.createLuaInterface(m_interfaceSourceCode, "intf");
        m_logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
        createSkinBinding(*nodeBinding, *appearanceBinding, m_logicEngine);

        EXPECT_EQ(nullptr, m_logicEngine.findByName<LuaModule>("dataarray"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<LuaScript>("nodebinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesNodeBinding>("appbinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<LuaScript>("camerabinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesAppearanceBinding>("animNode"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesCameraBinding>("script"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesRenderPassBinding>("luaModule"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesRenderGroupBinding>("appbinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<LuaModule>("meshbinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<DataArray>("renderGroupBinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<AnimationNode>("anchor"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<SkinBinding>("timerNode"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<TimerNode>("intf"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<AnchorPoint>("rpbinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesMeshNodeBinding>("skin"));
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectByNameOnlyStringMatchesExactly)
    {
        m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");

        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesNodeBinding>("Nodebinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesNodeBinding>("node"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesNodeBinding>("binding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesNodeBinding>("Xnodebinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findByName<RamsesNodeBinding>("nodebindinY"));
    }

    TEST_F(ALogicEngine_Lookup, GetHLObjectFromImpl)
    {
        const auto module = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        const auto script = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        const auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        const auto cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto rpBinding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rpbinding");
        const auto rgBinding = createRenderGroupBinding();
        const auto meshBinding = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "meshbinding");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "dataarray");
        const auto animNode = createAnimationNode(dataArray);
        const auto timer = m_logicEngine.createTimerNode("timerNode");
        const auto intf = m_logicEngine.createLuaInterface(m_interfaceSourceCode, "intf");
        const auto anchor = m_logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
        const auto skin = createSkinBinding(*nodeBinding, *appearanceBinding, m_logicEngine);

        EXPECT_EQ(module, &module->m_impl.getLogicObject());
        EXPECT_EQ(script, &script->m_script.getLogicObject());
        EXPECT_EQ(nodeBinding, &nodeBinding->m_nodeBinding.getLogicObject());
        EXPECT_EQ(appearanceBinding, &appearanceBinding->m_appearanceBinding.getLogicObject());
        EXPECT_EQ(cameraBinding, &cameraBinding->m_cameraBinding.getLogicObject());
        EXPECT_EQ(rpBinding, &rpBinding->m_renderPassBinding.getLogicObject());
        EXPECT_EQ(rgBinding, &rgBinding->m_renderGroupBinding.getLogicObject());
        EXPECT_EQ(meshBinding, &meshBinding->m_meshNodeBinding.getLogicObject());
        EXPECT_EQ(dataArray, &dataArray->m_impl.getLogicObject());
        EXPECT_EQ(animNode, &animNode->m_animationNodeImpl.getLogicObject());
        EXPECT_EQ(timer, &timer->m_timerNodeImpl.getLogicObject());
        EXPECT_EQ(intf, &intf->m_interface.getLogicObject());
        EXPECT_EQ(anchor, &anchor->m_anchorPointImpl.getLogicObject());
        EXPECT_EQ(skin, &skin->m_skinBinding.getLogicObject());
    }

    TEST_F(ALogicEngine_Lookup, GetHLObjectFromImpl_const)
    {
        const auto module = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        const auto script = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        const auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        const auto cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto rpBinding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "rpbinding");
        const auto rgBinding = createRenderGroupBinding();
        const auto meshBinding = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "meshbinding");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "dataarray");
        const auto animNode = createAnimationNode(dataArray);
        const auto timer = m_logicEngine.createTimerNode("timerNode");
        const auto intf = m_logicEngine.createLuaInterface(m_interfaceSourceCode, "intf");
        const auto anchor = m_logicEngine.createAnchorPoint(*nodeBinding, *cameraBinding, "anchor");
        const auto skin = createSkinBinding(*nodeBinding, *appearanceBinding, m_logicEngine);

        const auto& moduleImpl = module->m_impl;
        const auto& scriptImpl = script->m_script;
        const auto& nodeBindingImpl = nodeBinding->m_nodeBinding;
        const auto& appearanceBindingImpl = appearanceBinding->m_appearanceBinding;
        const auto& cameraBindingImpl = cameraBinding->m_cameraBinding;
        const auto& rpBindingImpl = rpBinding->m_renderPassBinding;
        const auto& rgBindingImpl = rgBinding->m_renderGroupBinding;
        const auto& meshBindingImpl = meshBinding->m_meshNodeBinding;
        const auto& dataArrayImpl = dataArray->m_impl;
        const auto& animNodeImpl = animNode->m_animationNodeImpl;
        const auto& timerImpl = timer->m_timerNodeImpl;
        const auto& intfImpl = intf->m_interface;
        const auto& anchorImpl = anchor->m_anchorPointImpl;
        const auto& skinImpl = skin->m_skinBinding;

        EXPECT_EQ(module, &moduleImpl.getLogicObject());
        EXPECT_EQ(script, &scriptImpl.getLogicObject());
        EXPECT_EQ(nodeBinding, &nodeBindingImpl.getLogicObject());
        EXPECT_EQ(appearanceBinding, &appearanceBindingImpl.getLogicObject());
        EXPECT_EQ(cameraBinding, &cameraBindingImpl.getLogicObject());
        EXPECT_EQ(rpBinding, &rpBindingImpl.getLogicObject());
        EXPECT_EQ(rgBinding, &rgBindingImpl.getLogicObject());
        EXPECT_EQ(meshBinding, &meshBindingImpl.getLogicObject());
        EXPECT_EQ(dataArray, &dataArrayImpl.getLogicObject());
        EXPECT_EQ(animNode, &animNodeImpl.getLogicObject());
        EXPECT_EQ(timer, &timerImpl.getLogicObject());
        EXPECT_EQ(intf, &intfImpl.getLogicObject());
        EXPECT_EQ(anchor, &anchorImpl.getLogicObject());
        EXPECT_EQ(skin, &skinImpl.getLogicObject());
    }
}

