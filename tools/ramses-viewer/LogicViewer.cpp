//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewer.h"
#include "LogicViewerLuaTypes.h"
#include "LogicViewerLog.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/AnchorPoint.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/RenderBufferBinding.h"
#include "ramses/client/SceneObjectIterator.h"
#include "fmt/format.h"
#include "internal/logic/SolHelper.h"
#include <iostream>
#include <fstream>

namespace ramses::internal
{
    namespace
    {
        // NOLINTNEXTLINE(performance-unnecessary-value-param) The signature is forced by SOL. Therefore we have to disable this warning.
        int solExceptionHandler(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description)
        {
            if (maybe_exception)
            {
                return sol::stack::push(L, description);
            }
            return sol::stack::top(L);
        }

        template<class T>
        void registerNodeListType(sol::state& sol, const char* name) {
            sol.new_usertype<NodeListWrapper<T>>(name,
                                                 sol::no_constructor,
                                                 sol::meta_function::index,
                                                 &NodeListWrapper<T>::get,
                                                 sol::meta_function::call,
                                                 &NodeListWrapper<T>::iterator,
                                                 sol::meta_function::to_string,
                                                 [=]() { return name; });
            std::string iteratorName = name;
            iteratorName += "Iterator";
            sol.new_usertype<NodeListIterator<T>>(iteratorName, sol::no_constructor, sol::meta_function::call, &NodeListIterator<T>::call);
        }
    } // namespace

    struct LogicWrapper
    {
        explicit LogicWrapper(LogicViewer& viewer, LogicEngine& logicEngine)
            : m_logicEngine(logicEngine)
            , update ([&]() { viewer.updateEngine(logicEngine); })
            , interfaces(logicEngine)
            , scripts(logicEngine)
            , animations(logicEngine)
            , timers(logicEngine)
            , nodeBindings(logicEngine)
            , appearanceBindings(logicEngine)
            , cameraBindings(logicEngine)
            , renderPassBindings(logicEngine)
            , renderGroupBindings(logicEngine)
            , meshNodeBindings(logicEngine)
            , anchorPoints(logicEngine)
            , skinBindings(logicEngine)
            , renderBufferBindings(logicEngine)
        {
            link = [&](PropertyWrapper& src, PropertyWrapper& target) { return logicEngine.link(src.m_property, target.m_property); };
            unlink = [&](PropertyWrapper& src, PropertyWrapper& target) { return logicEngine.unlink(src.m_property, target.m_property); };
        }

        explicit LogicWrapper(LogicViewer& viewer, LogicEngine& logicEngine, sol::state& sol)
            : LogicWrapper(viewer, logicEngine)
        {
            views  = sol.create_table();
        }

        ramses::LogicEngine& m_logicEngine;

        // function objects allow lua syntax `rlogic.update()` - instead of `rlogic:update()`
        std::function<void()> update;
        std::function<bool(PropertyWrapper&, PropertyWrapper&)> link;
        std::function<bool(PropertyWrapper&, PropertyWrapper&)> unlink;

        sol::table views;

        NodeListWrapper<LuaInterface> interfaces;
        NodeListWrapper<LuaScript> scripts;
        NodeListWrapper<AnimationNode> animations;
        NodeListWrapper<TimerNode> timers;
        NodeListWrapper<NodeBinding> nodeBindings;
        NodeListWrapper<AppearanceBinding> appearanceBindings;
        NodeListWrapper<CameraBinding> cameraBindings;
        NodeListWrapper<RenderPassBinding> renderPassBindings;
        NodeListWrapper<RenderGroupBinding> renderGroupBindings;
        NodeListWrapper<MeshNodeBinding> meshNodeBindings;
        NodeListWrapper<AnchorPoint> anchorPoints;
        NodeListWrapper<SkinBinding> skinBindings;
        NodeListWrapper<RenderBufferBinding> renderBufferBindings;
    };

    struct LogicCollectionWrapper
    {
        explicit LogicCollectionWrapper(LogicViewer& viewer)
        {
            for (auto* logic : viewer.m_logicEngines)
            {
                m_logicWrappers.emplace_back(viewer, *logic, viewer.m_sol);
            }
        }

        sol::object get(sol::stack_object key, sol::this_state L)
        {
            auto strKey = key.as<sol::optional<std::string>>();
            auto it = m_logicWrappers.end();
            if (strKey)
            {
                it = std::find_if(m_logicWrappers.begin(), m_logicWrappers.end(),
                    [&](const LogicWrapper& wrapper) { return wrapper.m_logicEngine.getName() == *strKey; });
            }
            else
            {
                auto intKey = key.as<sol::optional<int>>();
                if (intKey)
                {
                    const sceneObjectId_t objId(*intKey);
                    it = std::find_if(m_logicWrappers.begin(), m_logicWrappers.end(),
                        [&](const LogicWrapper& wrapper) { return wrapper.m_logicEngine.getSceneObjectId() == objId; });
                }
            }

            if (it != m_logicWrappers.end())
            {
                return sol::object(L, sol::in_place, *it);
            }
            return sol::object(L, sol::in_place, sol::lua_nil);
        }

        sol::variadic_results call(sol::this_state L)
        {
            sol::variadic_results results;
            results.reserve(m_logicWrappers.size());
            for (auto& logicWrapper : m_logicWrappers)
                results.push_back(sol::object(L, sol::in_place, logicWrapper));
            return results;
        }

        std::vector<LogicWrapper> m_logicWrappers;
    };

    struct ViewerWrapper
    {
        ViewerWrapper(LogicViewer& viewer, sol::state& sol)
            : views(sol.create_table())
            , logic(viewer)
        {
        }
        sol::table views;
        LogicCollectionWrapper logic;
    };


    const char* const LogicViewer::ltnRoot         = "R";
    const char* const LogicViewer::ltnLogic        = "logic";
    const char* const LogicViewer::ltnScript      = "scripts";
    const char* const LogicViewer::ltnInterface   = "interfaces";
    const char* const LogicViewer::ltnAnimation   = "animationNodes";
    const char* const LogicViewer::ltnTimer       = "timerNodes";
    const char* const LogicViewer::ltnNode        = "nodeBindings";
    const char* const LogicViewer::ltnAppearance  = "appearanceBindings";
    const char* const LogicViewer::ltnCamera      = "cameraBindings";
    const char* const LogicViewer::ltnRenderPass  = "renderPassBindings";
    const char* const LogicViewer::ltnRenderGroup = "renderGroupBindings";
    const char* const LogicViewer::ltnMeshNode    = "meshNodeBindings";
    const char* const LogicViewer::ltnAnchorPoint = "anchorPoints";
    const char* const LogicViewer::ltnSkinBinding = "skinBindings";
    const char* const LogicViewer::ltnRenderBuffer = "renderBufferBindings";
    const char* const LogicViewer::ltnScreenshot  = "screenshot";
    const char* const LogicViewer::ltnViews       = "views";
    const char* const LogicViewer::ltnLink        = "link";
    const char* const LogicViewer::ltnUnlink      = "unlink";
    const char* const LogicViewer::ltnUpdate      = "update";
    const char* const LogicViewer::ltnIN          = "IN";
    const char* const LogicViewer::ltnOUT         = "OUT";

    const char* const LogicViewer::ltnPropertyValue   = "value";
    const char* const LogicViewer::ltnViewUpdate      = "update";
    const char* const LogicViewer::ltnViewInputs      = "inputs";
    const char* const LogicViewer::ltnViewName        = "name";
    const char* const LogicViewer::ltnViewDescription = "description";

    LogicViewer::LogicViewer(ramses::Scene& scene, ScreenshotFunc screenshotFunc)
        : m_screenshotFunc(std::move(screenshotFunc))
    {
        SceneObjectIterator iter{scene, ERamsesObjectType::LogicEngine};
        while (auto* logic = object_cast<LogicEngine*>(iter.getNext()))
            m_logicEngines.push_back(logic);
        m_startTime = std::chrono::steady_clock::now();
    }

    Result LogicViewer::loadLuaFile(const std::string& filename)
    {
        m_result = Result();
        m_sol = sol::state();
        m_sol.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug);
        m_sol.set_exception_handler(&solExceptionHandler);
        registerNodeListType<LuaInterface>(m_sol, "Interfaces");
        registerNodeListType<LuaScript>(m_sol, "LuaScripts");
        registerNodeListType<AnimationNode>(m_sol, "AnimationNodes");
        registerNodeListType<TimerNode>(m_sol, "TimerNodes");
        registerNodeListType<NodeBinding>(m_sol, "NodeBindings");
        registerNodeListType<AppearanceBinding>(m_sol, "AppearanceBindings");
        registerNodeListType<CameraBinding>(m_sol, "CameraBindings");
        registerNodeListType<RenderPassBinding>(m_sol, "RenderPassBindings");
        registerNodeListType<RenderGroupBinding>(m_sol, "RenderGroupBindings");
        registerNodeListType<MeshNodeBinding>(m_sol, "MeshNodeBindings");
        registerNodeListType<AnchorPoint>(m_sol, "AnchorPoints");
        registerNodeListType<SkinBinding>(m_sol, "SkinBindings");
        registerNodeListType<RenderBufferBinding>(m_sol, "RenderBufferBindings");
        m_sol.new_usertype<LogicNodeWrapper>("LogicNode", sol::meta_function::index, &LogicNodeWrapper::get, sol::meta_function::to_string, &LogicNodeWrapper::toString);
        m_sol.new_usertype<PropertyWrapper>("LogicProperty",
                                            ltnPropertyValue,
                                            sol::property(&PropertyWrapper::getValue, &PropertyWrapper::setValue),
                                            sol::meta_function::index,
                                            &PropertyWrapper::get,
                                            sol::meta_function::to_string,
                                            &PropertyWrapper::toString);
        m_sol.new_usertype<LogicWrapper>(
            "RamsesLogic",
            sol::no_constructor,
            ltnInterface,
            sol::readonly(&LogicWrapper::interfaces),
            ltnScript,
            sol::readonly(&LogicWrapper::scripts),
            ltnAnimation,
            sol::readonly(&LogicWrapper::animations),
            ltnTimer,
            sol::readonly(&LogicWrapper::timers),
            ltnNode,
            sol::readonly(&LogicWrapper::nodeBindings),
            ltnAppearance,
            sol::readonly(&LogicWrapper::appearanceBindings),
            ltnCamera,
            sol::readonly(&LogicWrapper::cameraBindings),
            ltnRenderPass,
            sol::readonly(&LogicWrapper::renderPassBindings),
            ltnRenderGroup,
            sol::readonly(&LogicWrapper::renderGroupBindings),
            ltnMeshNode,
            sol::readonly(&LogicWrapper::meshNodeBindings),
            ltnAnchorPoint,
            sol::readonly(&LogicWrapper::anchorPoints),
            ltnSkinBinding,
            sol::readonly(&LogicWrapper::skinBindings),
            ltnRenderBuffer,
            sol::readonly(&LogicWrapper::renderBufferBindings),
            ltnUpdate,
            &LogicWrapper::update,
            ltnLink,
            &LogicWrapper::link,
            ltnUnlink,
            &LogicWrapper::unlink
            );

        m_sol.new_usertype<LogicCollectionWrapper>(
            "RamsesLogicEngines",
            sol::no_constructor,
            sol::meta_function::index,
            &LogicCollectionWrapper::get,
            sol::meta_function::call,
            &LogicCollectionWrapper::call,
            sol::meta_function::to_string,
            []() { return "LogicCollectionWrapper"; }
        );

        m_sol.new_usertype<ViewerWrapper>(
            "RamsesLogicViewer",
            sol::no_constructor,
            ltnLogic,
            sol::readonly(&ViewerWrapper::logic),
            ltnViews,
            &ViewerWrapper::views,
            ltnScreenshot,
            [&](const std::string& screenshotFile) {
                updateAllEngines(); // ensure that all state changes are applied
                if (!m_screenshotFunc)
                {
                    throw sol::error("No screenshots available in current configuration");
                }
                return m_screenshotFunc(screenshotFile);
            },
            ltnUpdate,
            [&]() { updateAllEngines(); }
        );

        m_sol[ltnRoot] = ViewerWrapper(*this, m_sol);

        m_luaFilename = filename;
        if (!filename.empty())
        {
            load(m_sol.load_file(filename));
        }
        return m_result;
    }

    void LogicViewer::load(sol::load_result&& loadResult)
    {
        if (!loadResult.valid())
        {
            sol::error err = loadResult;
            m_result = Result(err.what());
        }
        else
        {
            sol::protected_function mainFunction = loadResult;
            auto mainResult = mainFunction();
            if (!mainResult.valid())
            {
                sol::error err = mainResult;
                m_result = Result(err.what());
            }
        }
    }

    Result LogicViewer::exec(const std::string& lua)
    {
        load(m_sol.load_buffer(lua.c_str(), lua.size()));
        return m_result;
    }

    Result LogicViewer::call(const std::string& functionName)
    {
        sol::protected_function protectedFunction = m_sol[functionName];
        auto result = protectedFunction();
        if (!result.valid())
        {
            sol::error err = result;
            m_result = Result(err.what());
        }
        return m_result;
    }

    Result LogicViewer::update()
    {
        updateAllEngines();
        // don't update if there's already an error
        if (m_result.ok())
        {
            sol::optional<sol::table> view = m_sol[ltnRoot][ltnViews][m_view];
            if (view)
            {
                const auto elapsed   = std::chrono::steady_clock::now() - m_startTime;
                const auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                sol::optional<sol::protected_function> func = (*view)[ltnViewUpdate];
                if (func)
                {
                    auto result = (*func)(millisecs);
                    if (!result.valid())
                    {
                        sol::error err = result;
                        std::cerr << err.what() << std::endl;
                        m_result = Result(err.what());
                        return m_result;
                    }
                }
                else
                {
                    m_result = Result("update() function is missing for current view");
                    return m_result;
                }
            }
        }
        return Result();
    }

    size_t LogicViewer::getViewCount() const
    {
        sol::optional<sol::table> tbl = m_sol[ltnRoot][ltnViews];
        return tbl ? tbl->size() : 0U;
    }

    void LogicViewer::setCurrentView(size_t viewId)
    {
        if (viewId >= 1U && viewId <= getViewCount())
            m_view = viewId;
    }

    LogicViewer::View LogicViewer::getView(size_t viewId) const
    {
        sol::optional<sol::table> tbl = m_sol[ltnRoot][ltnViews][viewId];
        return View(std::move(tbl));
    }

    void LogicViewer::updateAllEngines()
    {
        for (auto* logic : m_logicEngines)
            updateEngine(*logic);
    }

    void LogicViewer::updateEngine(ramses::LogicEngine& engine)
    {
        engine.update();
        if (m_updateReportEnabled)
        {
            m_updateReports[engine.getSceneObjectId()].add(engine.getLastUpdateReport());
        }
    }

    Result LogicViewer::saveDefaultLuaFile(const std::string& filename, const ViewerSettings& settings)
    {
        std::ofstream outfile;
        outfile.open(filename, std::ios::out | std::ios::binary);
        if (!outfile.is_open())
        {
            return Result(fmt::format("Could not open file: {}", filename));
        }
        LogicViewerLog log(settings);
        log.logText("function default()\n");

        log.setIndent("    ");
        for (auto* logic : m_logicEngines)
        {
            log.logAllInputs<LuaInterface>(*logic, "--Interfaces\n", LogicViewer::ltnInterface);
            log.logAllInputs<LuaScript>(*logic, "--Scripts\n", LogicViewer::ltnScript);
            log.logAllInputs<NodeBinding>(*logic, "--Node bindings\n", LogicViewer::ltnNode);
            log.logAllInputs<AppearanceBinding>(*logic, "--Appearance bindings\n", LogicViewer::ltnAppearance);
            log.logAllInputs<CameraBinding>(*logic, "--Camera bindings\n", LogicViewer::ltnCamera);
            log.logAllInputs<RenderPassBinding>(*logic, "--RenderPass bindings\n", LogicViewer::ltnRenderPass);
            log.logAllInputs<RenderGroupBinding>(*logic, "--RenderGroup bindings\n", LogicViewer::ltnRenderGroup);
            log.logAllInputs<MeshNodeBinding>(*logic, "--MeshNode bindings\n", LogicViewer::ltnMeshNode);
            log.logAllInputs<AnchorPoint>(*logic, "--Anchor points\n", LogicViewer::ltnAnchorPoint);
            log.logAllInputs<SkinBinding>(*logic, "--Skin bindings\n", LogicViewer::ltnSkinBinding);
            log.logAllInputs<RenderBufferBinding>(*logic, "--RenderBuffer bindings\n", LogicViewer::ltnRenderBuffer);
        }
        log.setIndent({});

        log.logText("end\n\n");
        const std::string code = fmt::format(R"(
defaultView = {{
    name = "Default",
    description = "",
    update = function(time_ms)
        default()
    end,
    inputs = {{}}
}}

{0}.views = {{defaultView}}

-- sample test function for automated image base tests
-- can be executed by command line parameter --exec=test_default
function test_default()
    -- modify properties
    default()
    -- stores a screenshot (relative to the working directory)
    {0}.screenshot("test_default.png")
end
)", LogicViewer::ltnRoot);
        log.logText(code);
        outfile << log.getText();
        if (outfile.bad())
        {
            return Result(fmt::format("Could not write to file: {}", filename));
        }
        outfile.close();
        return Result();
    }
}

