//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerTestBase.h"
#include "ramses/client/logic/AnimationNodeConfig.h"
#include "ramses/client/logic/RenderGroupBindingElements.h"

namespace ramses::internal
{
    /**
     * Tests for the lua configuration file handling
     */
    class ALogicViewerLua2Engines : public ALogicViewerBase
    {
    public:
        ALogicViewerLua2Engines()
            : ALogicViewerBase(2)
        {
            setupLogic();
        }

        static auto* SetupInterface(LogicEngine& engine)
        {
            return engine.createLuaInterface(R"(
                function interface(IN,OUT)
                    IN.paramInt32 = Type:Int32()
                end
            )", "foo");
        }

        static auto* SetupScript(LogicEngine& engine, int32_t value)
        {
            return engine.createLuaScript(fmt::format(R"(
                function interface(IN,OUT)
                    IN.paramInt32 = Type:Int32()
                    IN.bar = Type:Int32()
                    OUT.paramInt32 = Type:Int32()
                end
                function run(IN,OUT)
                    OUT.paramInt32 = 2 * IN.paramInt32 + {}
                end
            )", value), {}, "foo");
        }

        void setupLogic()
        {
            auto* interface0 = SetupInterface(*m_logicEngines[0]);
            auto* interface1 = SetupInterface(*m_logicEngines[1]);
            auto* script0 = SetupScript(*m_logicEngines[0], 0);
            auto* script1 = SetupScript(*m_logicEngines[1], 1);

            ASSERT_TRUE(script0 != nullptr);
            ASSERT_TRUE(script1 != nullptr);

            m_logicEngines[0]->link(
                *interface0->getOutputs()->getChild("paramInt32"),
                *script0->getInputs()->getChild("paramInt32"));

            m_logicEngines[1]->link(
                *interface1->getOutputs()->getChild("paramInt32"),
                *script1->getInputs()->getChild("paramInt32"));

            m_logicEngines[0]->update();
            m_logicEngines[1]->update();
        }
    };

    TEST_F(ALogicViewerLua2Engines, loadLuaFileEmpty)
    {
        EXPECT_EQ(Result(), loadLua(""));
    }

    TEST_F(ALogicViewerLua2Engines, setInputByEngine0Name)
    {
        const Result ok;
        EXPECT_EQ(ok, loadLua(R"(R.logic["logic0"].interfaces.foo.IN.paramInt32.value = 42)"));
        auto* property = getInput<ramses::LuaScript>(*m_logicEngines[0], "foo", "paramInt32");
        EXPECT_EQ(42, property->get<int32_t>());
    }

    TEST_F(ALogicViewerLua2Engines, setInputByEngine1Name)
    {
        const Result ok;
        EXPECT_EQ(ok, loadLua(R"(R.logic["logic1"].interfaces.foo.IN.paramInt32.value = 43)"));
        auto* property = getInput<ramses::LuaScript>(*m_logicEngines[1], "foo", "paramInt32");
        EXPECT_EQ(43, property->get<int32_t>());
    }

    TEST_F(ALogicViewerLua2Engines, setInputByEngine0Id)
    {
        const Result ok;
        EXPECT_EQ(ok, loadLua(fmt::format(R"(R.logic[{}].interfaces.foo.IN.paramInt32.value = 42)", m_logicEngines[0]->getSceneObjectId().getValue())));
        auto* property = getInput<ramses::LuaScript>(*m_logicEngines[0], "foo", "paramInt32");
        EXPECT_EQ(42, property->get<int32_t>());
    }

    TEST_F(ALogicViewerLua2Engines, setInputByEngine1Id)
    {
        const Result ok;
        EXPECT_EQ(ok, loadLua(fmt::format(R"(R.logic[{}].interfaces.foo.IN.paramInt32.value = 43)", m_logicEngines[1]->getSceneObjectId().getValue())));
        auto* property = getInput<ramses::LuaScript>(*m_logicEngines[1], "foo", "paramInt32");
        EXPECT_EQ(43, property->get<int32_t>());
    }

    TEST_F(ALogicViewerLua2Engines, findAllLogicEngines)
    {
        const Result ok;
        EXPECT_EQ(ok, loadLua(R"(
            a,b = R.logic()
            a.interfaces.foo.IN.paramInt32.value = 42
            b.interfaces.foo.IN.paramInt32.value = 43
        )"));
        auto* property0 = getInput<ramses::LuaScript>(*m_logicEngines[0], "foo", "paramInt32");
        EXPECT_EQ(42, property0->get<int32_t>());
        auto* property1 = getInput<ramses::LuaScript>(*m_logicEngines[1], "foo", "paramInt32");
        EXPECT_EQ(43, property1->get<int32_t>());
    }

    TEST_F(ALogicViewerLua2Engines, iterateAllLogicEngines)
    {
        const Result ok;
        EXPECT_EQ(ok, loadLua(R"(
            for i, engine in ipairs({R.logic()}) do
                engine.interfaces.foo.IN.paramInt32.value = i
            end
        )"));
        auto* property0 = getInput<ramses::LuaScript>(*m_logicEngines[0], "foo", "paramInt32");
        EXPECT_EQ(1, *property0->get<int32_t>());
        auto* property1 = getInput<ramses::LuaScript>(*m_logicEngines[1], "foo", "paramInt32");
        EXPECT_EQ(2, *property1->get<int32_t>());
    }

    TEST_F(ALogicViewerLua2Engines, viewsAreGlobal)
    {
        const Result ok;
        EXPECT_NE(ok, loadLua(R"(
            defaultView = {
                name = "Default View",
                description = "Description for the default view",
                update = function(time_ms)
                    R.logic.logic0.interfaces.foo.IN.paramInt32.value = 42
                    R.logic.logic1.interfaces.foo.IN.paramInt32.value = 142
                end
            }

            R.logic.logic1.views = {defaultView}
        )"));

        // Views are global (not engine specific)
        EXPECT_EQ(0u, viewer->getViewCount());
    }

    TEST_F(ALogicViewerLua2Engines, simpleView)
    {
        const Result ok;
        auto* property0 = getInput<ramses::LuaScript>(*m_logicEngines[0], "foo", "paramInt32");
        auto* property1 = getInput<ramses::LuaScript>(*m_logicEngines[1], "foo", "paramInt32");
        EXPECT_EQ(0u, viewer->getViewCount());
        EXPECT_FALSE(viewer->getView(0u).isValid());
        EXPECT_FALSE(viewer->getView(1u).isValid());
        EXPECT_EQ(0, property0->get<int32_t>().value());
        EXPECT_EQ(0, property1->get<int32_t>().value());

        EXPECT_EQ(ok, loadLua(R"(
            defaultView = {
                name = "Default View",
                description = "Description for the default view",
                update = function(time_ms)
                    R.logic.logic0.interfaces.foo.IN.paramInt32.value = 42
                    R.logic.logic1.interfaces.foo.IN.paramInt32.value = 142
                end
            }

            R.views = {defaultView}
        )"));

        EXPECT_EQ(1u, viewer->getViewCount());
        EXPECT_EQ(1u, viewer->getCurrentView());
        auto view = viewer->getView(1u);
        EXPECT_TRUE(view.isValid());
        EXPECT_EQ("Default View", view.name());
        EXPECT_EQ("Description for the default view", view.description());
        EXPECT_EQ(0u, view.getInputCount());
        EXPECT_EQ(nullptr, view.getInput(0u));

        EXPECT_EQ(ok, viewer->update());
        EXPECT_EQ(42, property0->get<int32_t>().value());
        EXPECT_EQ(142, property1->get<int32_t>().value());
    }

    TEST_F(ALogicViewerLua2Engines, viewWithInputs)
    {
        const Result ok;
        auto* property0 = getInput<ramses::LuaInterface>(*m_logicEngines[0], "foo", "paramInt32");
        auto* property1 = getInput<ramses::LuaInterface>(*m_logicEngines[1], "foo", "paramInt32");

        EXPECT_EQ(ok, loadLua(R"(
            defaultView = {
                name = "View1",
                description = "View with 2 user inputs from different engines",
                update = function(time_ms)
                end,
                inputs = {R.logic.logic0.interfaces.foo.IN.paramInt32, R.logic.logic1.interfaces.foo.IN.paramInt32}
            }

            R.views = {defaultView}
        )"));

        EXPECT_EQ(1u, viewer->getViewCount());
        EXPECT_EQ(1u, viewer->getCurrentView());
        auto view = viewer->getView(1u);
        EXPECT_TRUE(view.isValid());
        EXPECT_EQ("View1", view.name());
        EXPECT_EQ("View with 2 user inputs from different engines", view.description());
        EXPECT_EQ(2u, view.getInputCount());
        EXPECT_EQ(property0, view.getInput(0u));
        EXPECT_EQ(property1, view.getInput(1u));
    }

    TEST_F(ALogicViewerLua2Engines, changeView)
    {
        const Result ok;
        auto* property0 = getInput<ramses::LuaInterface>(*m_logicEngines[0], "foo", "paramInt32");
        auto* property1 = getInput<ramses::LuaInterface>(*m_logicEngines[1], "foo", "paramInt32");

        EXPECT_EQ(ok, loadLua(R"(
            view1 = {
                name = "View1",
                description = "View1 with 2 user inputs",
                update = function(time_ms)
                end,
                inputs = {
                    R.logic["logic0"].interfaces.foo.IN.paramInt32,
                    R.logic["logic1"].interfaces.foo.IN.paramInt32,
                }
            }

            view2 = {
                name = "View2",
                description = "View2 with no inputs",
                update = function(time_ms)
                    R.logic["logic1"].interfaces.foo.IN.paramInt32.value = 1773
                end,
            }

            R.views = {view1, view2}
        )"));

        EXPECT_EQ(2u, viewer->getViewCount());
        EXPECT_EQ(1u, viewer->getCurrentView());
        auto view1 = viewer->getView(1u);
        EXPECT_TRUE(view1.isValid());
        EXPECT_EQ("View1", view1.name());
        EXPECT_EQ("View1 with 2 user inputs", view1.description());
        EXPECT_EQ(2u, view1.getInputCount());
        EXPECT_EQ(property0, view1.getInput(0u));
        EXPECT_EQ(property1, view1.getInput(1u));

        EXPECT_EQ(ok, viewer->update());
        EXPECT_EQ(0, property0->get<int32_t>().value());
        EXPECT_EQ(0, property1->get<int32_t>().value());

        viewer->setCurrentView(2u);

        EXPECT_EQ(2u, viewer->getCurrentView());
        auto view2 = viewer->getView(2u);
        EXPECT_EQ("View2", view2.name());
        EXPECT_EQ("View2 with no inputs", view2.description());
        EXPECT_EQ(0u, view2.getInputCount());

        EXPECT_EQ(ok, viewer->update());
        EXPECT_EQ(0, property0->get<int32_t>().value());
        EXPECT_EQ(1773, property1->get<int32_t>().value());
    }

    TEST_F(ALogicViewerLua2Engines, callByName)
    {
        const Result ok;
        auto* property0 = getInput<ramses::LuaInterface>(*m_logicEngines[0], "foo", "paramInt32");
        auto* property1 = getInput<ramses::LuaInterface>(*m_logicEngines[1], "foo", "paramInt32");

        EXPECT_EQ(ok, loadLua(R"(
            function f1()
                R.logic["logic0"].interfaces.foo.IN.paramInt32.value = -91
            end

            function f2()
                R.logic["logic1"].interfaces.foo.IN.paramInt32.value = 908
            end
        )"));

        EXPECT_EQ(0, property0->get<int32_t>().value());
        EXPECT_EQ(0, property1->get<int32_t>().value());
        EXPECT_EQ(ok, viewer->call("f1"));
        EXPECT_EQ(-91, property0->get<int32_t>().value());
        EXPECT_EQ(ok, viewer->call("f2"));
        EXPECT_EQ(908, property1->get<int32_t>().value());
    }

    TEST_F(ALogicViewerLua2Engines, update)
    {
        const Result ok;
        EXPECT_EQ(Result(), loadLua(R"(
            function update0()
                R.logic["logic0"].update()
            end
            function update1()
                R.logic["logic1"].update()
            end
        )"));

        EXPECT_EQ(0, getOutput<ramses::LuaScript>(*m_logicEngines[0], "foo", "paramInt32")->get<int32_t>());
        EXPECT_EQ(1, getOutput<ramses::LuaScript>(*m_logicEngines[1], "foo", "paramInt32")->get<int32_t>());

        EXPECT_TRUE(getInput<ramses::LuaInterface>(*m_logicEngines[0], "foo", "paramInt32")->set(static_cast<int32_t>(42)));
        EXPECT_TRUE(getInput<ramses::LuaInterface>(*m_logicEngines[1], "foo", "paramInt32")->set(static_cast<int32_t>(142)));

        EXPECT_EQ(ok, viewer->call("update1"));
        EXPECT_EQ(0, *getOutput<ramses::LuaScript>(*m_logicEngines[0], "foo", "paramInt32")->get<int32_t>());
        EXPECT_EQ(2 * 142 + 1, *getOutput<ramses::LuaScript>(*m_logicEngines[1], "foo", "paramInt32")->get<int32_t>());

        EXPECT_EQ(ok, viewer->call("update0"));
        EXPECT_EQ(2 * 42, *getOutput<ramses::LuaScript>(*m_logicEngines[0], "foo", "paramInt32")->get<int32_t>());
        EXPECT_EQ(2 * 142 + 1, *getOutput<ramses::LuaScript>(*m_logicEngines[1], "foo", "paramInt32")->get<int32_t>());
    }

    TEST_F(ALogicViewerLua2Engines, link_unlink)
    {
        auto* in = getInput<ramses::LuaScript>(*m_logicEngines[1], "foo", "bar");
        EXPECT_FALSE(in->isLinked());
        EXPECT_FALSE(in->hasIncomingLink());
        EXPECT_FALSE(in->hasOutgoingLink());

        EXPECT_EQ(Result(), loadLua(R"(
            R.logic.logic1.link(R.logic.logic1.interfaces.foo.OUT.paramInt32, R.logic.logic1.scripts.foo.IN.bar)
        )"));

        EXPECT_TRUE(in->isLinked());
        EXPECT_TRUE(in->hasIncomingLink());
        EXPECT_FALSE(in->hasOutgoingLink());

        EXPECT_EQ(Result(), loadLua(R"(
            R.logic.logic1.unlink(R.logic.logic1.interfaces.foo.OUT.paramInt32, R.logic.logic1.scripts.foo.IN.bar)
        )"));
        EXPECT_FALSE(in->isLinked());
        EXPECT_FALSE(in->hasIncomingLink());
        EXPECT_FALSE(in->hasOutgoingLink());
    }

    TEST_F(ALogicViewerLua2Engines, enableUpdateReport)
    {
        // set update interval to 1 to avoid random test failures
        // (only the longest update is reported for an interval)
        const size_t updateInterval = 1u; // in frames
        EXPECT_FALSE(viewer->isUpdateReportEnabled());
        auto& summary0 = viewer->getUpdateReport(*m_logicEngines[0]);
        auto& summary1 = viewer->getUpdateReport(*m_logicEngines[1]);

        viewer->enableUpdateReport(true, updateInterval);
        EXPECT_TRUE(viewer->isUpdateReportEnabled());
        EXPECT_TRUE(getInput<ramses::LuaInterface>(*m_logicEngines[1], "foo", "paramInt32")->set(static_cast<int32_t>(142)));
        EXPECT_EQ(Result(), viewer->update());

        EXPECT_EQ(0u, summary0.getLinkActivations().maxValue);
        EXPECT_EQ(0u, summary0.getNodesExecuted().size());
        EXPECT_EQ(2u, summary0.getNodesSkippedExecution().size());

        EXPECT_EQ(1u, summary1.getLinkActivations().maxValue);
        EXPECT_EQ(2u, summary1.getNodesExecuted().size());
        EXPECT_EQ(0u, summary1.getNodesSkippedExecution().size());
    }
}
