//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerGui.h"
#include "LogicEngineGui.h"
#include "LogicViewer.h"
#include "LogicViewerLog.h"
#include "ViewerGuiApp.h"
#include "ViewerSettings.h"
#include "ramses/client/Scene.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/AnchorPoint.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/RenderBufferBinding.h"
#include "internal/PlatformAbstraction/FmtBase.h"
#include "glm/gtc/type_ptr.hpp"

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
#include "imgui_internal.h" // for ImGuiSettingsHandler
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

#include <filesystem>

namespace fs = std::filesystem;

template <> struct fmt::formatter<std::chrono::microseconds>
{
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext> constexpr auto format(const std::chrono::microseconds value, FormatContext& ctx)
    {
        const auto c = value.count();
        if (c == 0)
            return fmt::format_to(ctx.out(), "0");
        return fmt::format_to(ctx.out(), "{}.{:03}", c / 1000, c % 1000);
    }
};


namespace ramses::internal
{
    namespace
    {
        bool TreeNode(const void* ptr_id, const std::string& text)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
            return ImGui::TreeNode(ptr_id, "%s", text.c_str());
        }

        const char* NodeTypeName(const LogicNode* node)
        {
            const char* name = "Unknown";
            if (node->as<LuaInterface>() != nullptr)
            {
                name = "LuaInterface";
            }
            else if (node->as<LuaScript>() != nullptr)
            {
                name = "LuaScript";
            }
            else if (node->as<AnimationNode>() != nullptr)
            {
                name = "Animation";
            }
            else if (node->as<NodeBinding>() != nullptr)
            {
                name = "NodeBinding";
            }
            else if (node->as<AppearanceBinding>() != nullptr)
            {
                name = "AppearanceBinding";
            }
            else if (node->as<CameraBinding>() != nullptr)
            {
                name = "CameraBinding";
            }
            else if (node->as<RenderPassBinding>() != nullptr)
            {
                name = "RenderPassBinding";
            }
            else if (node->as<RenderGroupBinding>() != nullptr)
            {
                name = "RenderGroupBinding";
            }
            else if (node->as<MeshNodeBinding>() != nullptr)
            {
                name = "MeshNodeBinding";
            }
            else if (node->as<TimerNode>() != nullptr)
            {
                name = "Timer";
            }
            else if (node->as<AnchorPoint>() != nullptr)
            {
                name = "AnchorPoint";
            }
            else if (node->as<SkinBinding>() != nullptr)
            {
                name = "SkinBinding";
            }
            else if (node->as<RenderBufferBinding>() != nullptr)
            {
                name = "RenderBufferBinding";
            }
            return name;
        }
    }

    LogicViewerGui::LogicViewerGui(ViewerGuiApp& app, std::string& errorMessage)
        : m_settings(*app.getSettings())
        , m_viewer(*app.getLogicViewer())
        , m_lastErrorMessage(errorMessage)
        , m_filename(app.getLuaFile())
    {
        m_viewer.enableUpdateReport(m_settings.showUpdateReport, m_settings.updateReportInterval);
    }

    void LogicViewerGui::handleShortcuts()
    {
        if (ImGui::IsKeyPressed(ramses::EKeyCode_Left))
        {
            m_viewer.setCurrentView(m_viewer.getCurrentView() - 1);
        }
        else if (ImGui::IsKeyPressed(ramses::EKeyCode_Right))
        {
            m_viewer.setCurrentView(m_viewer.getCurrentView() + 1);
        }
        else if (ImGui::IsKeyPressed(ramses::EKeyCode_F5))
        {
            reloadConfiguration();
        }
        else
        {
        }
    }

    void LogicViewerGui::drawMenuItemReload()
    {
        if (ImGui::MenuItem("Reload lua configuration", "F5"))
        {
            reloadConfiguration();
        }
    }

    void LogicViewerGui::drawMenuItemSaveDefaultLua()
    {
        if (ImGui::MenuItem("Save default lua configuration", nullptr))
        {
            m_saveDefaultLuaFile = true;
        }
    }

    void LogicViewerGui::reloadConfiguration()
    {
        if (fs::exists(m_filename))
        {
            loadLuaFile(m_filename);
        }
    }

    void LogicViewerGui::loadLuaFile(const std::string& filename)
    {
        auto result = m_viewer.loadLuaFile(filename);
        if (!result.ok())
        {
            m_lastErrorMessage = result.getMessage();
        }
    }

    void LogicViewerGui::drawGlobalContextMenuItems()
    {
        drawMenuItemReload();
        if (ImGui::MenuItem("Next view", "Arrow Right", nullptr, (m_viewer.getCurrentView() < m_viewer.getViewCount())))
            m_viewer.setCurrentView(m_viewer.getCurrentView() + 1);
        if (ImGui::MenuItem("Previous view", "Arrow Left", nullptr, m_viewer.getCurrentView() > 1))
            m_viewer.setCurrentView(m_viewer.getCurrentView() - 1);
    }

    void LogicViewerGui::drawWindow()
    {
        if (!ImGui::Begin("Logic", &m_settings.showLogicWindow))
        {
            ImGui::End();
            return;
        }
        drawContents();
        ImGui::End();
    }

    void LogicViewerGui::drawContents()
    {
        drawCurrentView();

        if (m_settings.showUpdateReport)
        {
            for (auto* engine : m_viewer.getLogicEngines())
            {
                ImGui::Separator();
                drawUpdateReport(*engine);
            }
        }

        drawSaveDefaultLuaFile();
    }

    void LogicViewerGui::drawCurrentView()
    {
        const auto  viewCount = static_cast<int>(m_viewer.getViewCount());
        const auto& status    = m_viewer.getLastResult();
        if (!status.ok() || m_viewer.getLuaFilename().empty())
        {
            if (!status.ok())
            {
                ImGui::TextUnformatted(fmt::format("Error occurred in {}", m_viewer.getLuaFilename()).c_str());
                ImGui::TextUnformatted(status.getMessage().c_str());
            }
            else
            {
                ImGui::TextUnformatted("No lua configuration file found.");
            }
            ImGui::Separator();
            ImGui::InputText("##filename", &m_filename);
            ImGui::SameLine();
            if (ImGui::Button("Save default"))
                m_saveDefaultLuaFile = true;
            ImGui::SameLine();
            if (ImGui::Button("Open"))
            {
                fs::path luafile(m_filename);
                if (fs::exists(luafile))
                {
                    loadLuaFile(m_filename);
                }
                else if (!luafile.empty())
                {
                    m_lastErrorMessage = "File does not exist: " + m_filename;
                }
            }
        }
        else if (viewCount > 0)
        {
            auto       viewId = static_cast<int>(m_viewer.getCurrentView());
            const auto view   = m_viewer.getView(viewId);
            ImGui::TextUnformatted(!view.name().empty() ? view.name().c_str() : "<no name>");
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputInt("##View", &viewId))
                m_viewer.setCurrentView(viewId);
            ImGui::SameLine();
            ImGui::TextUnformatted(fmt::format("of {}", viewCount).c_str());

            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
            ImGui::TextWrapped("%s", view.description().c_str());

            LogicEngineGui engineGui(m_settings);
            for (size_t i = 0U; i < view.getInputCount(); ++i)
            {
                auto* prop = view.getInput(i);
                if (prop)
                    engineGui.drawProperty(view.getInput(i), i);
            }
        }
        else
        {
            ImGui::TextUnformatted("no views defined in configuration file");
        }
    }

    void LogicViewerGui::drawUpdateReport(LogicEngine& engine)
    {
        ImGui::TextUnformatted(fmt::format("Average Update Time: {} ms", m_viewer.getUpdateReport(engine).getTotalTime().average).c_str());
        ImGui::SameLine();
        imgui::HelpMarker("Time it took to update the whole logic nodes network (LogicEngine::update()).");
        ImGui::SameLine();
        std::string detailsText = "Show Details";
        if (m_viewer.getLogicEngines().size() > 1)
            detailsText = fmt::format("Show Details ([{}]: {})", engine.getSceneObjectId(), engine.getName());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
        if (ImGui::TreeNode(&engine, "%s", detailsText.c_str()))
        {
            auto interval = static_cast<int>(m_settings.updateReportInterval);
            bool refresh = m_viewer.isUpdateReportEnabled();
            if (ImGui::Checkbox("Auto Refresh", &refresh))
            {
                m_viewer.enableUpdateReport(refresh, m_settings.updateReportInterval);
            }
            ImGui::SetNextItemWidth(100);
            if (ImGui::DragInt("Refresh Interval", &interval, 0.5f, 1, 1000, "%d Frames"))
            {
                m_settings.updateReportInterval = static_cast<size_t>(interval);
                m_viewer.enableUpdateReport(refresh, m_settings.updateReportInterval);
            }
            const auto& report = m_viewer.getUpdateReport(engine);
            const auto& executed = report.getNodesExecuted();
            const auto& skipped  = report.getNodesSkippedExecution();
            const auto longest = report.getTotalTime().maxValue;

            ImGui::Separator();
            ImGui::TextUnformatted("Summary:");
            ImGui::SameLine();
            imgui::HelpMarker("Timing data is collected and summarized for {} frames.\n'min', 'max', 'avg' show the minimum, maximum, and average value for the measured interval.",
                m_settings.updateReportInterval);
            ImGui::Indent();

            const auto& updateTime = report.getTotalTime();
            ImGui::TextUnformatted(fmt::format("Total Update Time  (ms): max:{} min:{} avg:{}", updateTime.maxValue, updateTime.minValue, updateTime.average).c_str());
            ImGui::SameLine();
            imgui::HelpMarker("Time it took to update the whole logic nodes network (LogicEngine::update()).");

            const auto& sortTime = report.getSortTime();
            ImGui::TextUnformatted(fmt::format("Topology Sort Time (ms): max:{} min:{} avg:{}", sortTime.maxValue, sortTime.minValue, sortTime.average).c_str());
            ImGui::SameLine();
            imgui::HelpMarker("Time it took to sort logic nodes by their topology during update (see ramses::LogicEngineReport::getTopologySortExecutionTime()");

            const auto& links = report.getLinkActivations();
            ImGui::TextUnformatted(fmt::format("Activated Links: max:{} min:{} avg:{}", links.maxValue, links.minValue, links.average).c_str());
            ImGui::SameLine();
            imgui::HelpMarker("Number of input properties that had been updated by an output property (see ramses::LogicEngineReport::getTotalLinkActivations()).");
            ImGui::Unindent();

            ImGui::TextUnformatted(fmt::format("Details for the longest update ({} ms):", longest).c_str());
            LogicEngineGui engineGui(m_settings);
            if (TreeNode("Executed", fmt::format("Executed Nodes ({}):", executed.size())))
            {
                for (auto& timedNode : executed)
                {
                    auto* node = timedNode.first;
                    const auto percentage = (longest.count() > 0u) ? (100u * timedNode.second / longest) : 0u;
                    if (TreeNode(node, fmt::format("{}[{}]: {} [time:{} ms, {}%]", NodeTypeName(node), node->getSceneObjectId().getValue(), node->getName(), timedNode.second, percentage)))
                    {
                        engineGui.drawNode(node);
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }

            if (TreeNode("Skipped", fmt::format("Skipped Nodes ({}):", skipped.size())))
            {
                for (auto& node : skipped)
                {
                    if (TreeNode(node, fmt::format("{}[{}]: {}", NodeTypeName(node), node->getSceneObjectId().getValue(), node->getName())))
                    {
                        engineGui.drawNode(node);
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }

    void LogicViewerGui::drawSaveDefaultLuaFile()
    {
        if (m_saveDefaultLuaFile)
        {
            m_saveDefaultLuaFile = false;
            fs::path luafile(m_filename);
            if (fs::exists(luafile))
            {
                ImGui::OpenPopup("Overwrite?");
            }
            else if (!luafile.empty())
            {
                saveDefaultLuaFile();
            }
        }

        if (ImGui::BeginPopupModal("Overwrite?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextUnformatted(fmt::format("File exists:\n{}\nOverwrite default lua configuration?", m_filename).c_str());
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                saveDefaultLuaFile();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void LogicViewerGui::saveDefaultLuaFile()
    {
        const auto result = m_viewer.saveDefaultLuaFile(m_filename, m_settings);
        if (result.ok())
        {
            loadLuaFile(m_filename);
        }
        else
        {
            m_lastErrorMessage = result.getMessage();
        }
    }
}

