//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ImguiClientHelper.h"
#include "LogicViewerLog.h"
#include "ramses/client/logic/Collection.h"
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
#include "ramses/client/logic/RenderBufferBinding.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/AnchorPoint.h"
#include "ramses/client/logic/SkinBinding.h"
#include "fmt/format.h"
#include <array>

namespace ramses::internal
{
    struct ViewerSettings;

    class LogicEngineGui
    {
    public:
        explicit LogicEngineGui(const ViewerSettings& settings);

        template <class T>
        void drawCollection(ramses::LogicEngine& logicEngine, std::string_view ns, const char* headline);

        template<class T, class C>
        void drawCollection(ramses::Collection<T> collection, std::string_view ns, const char* headline, const C& cb);

        void draw(ramses::LuaInterface* obj);
        void draw(ramses::LuaScript* obj);
        void draw(ramses::AnimationNode* obj);
        void draw(ramses::TimerNode* obj);
        void draw(ramses::NodeBinding* obj);
        void draw(ramses::CameraBinding* obj);
        void draw(ramses::RenderPassBinding* obj);
        void draw(ramses::RenderGroupBinding* obj);
        void draw(ramses::MeshNodeBinding* obj);
        void draw(ramses::AnchorPoint* obj);
        void draw(ramses::SkinBinding* obj);
        void draw(ramses::AppearanceBinding* obj);
        void draw(ramses::RenderBufferBinding* obj);

        void drawNode(ramses::LogicNode* obj);
        void drawProperty(ramses::Property* prop, size_t index);

        /**
         * copies all node inputs to clipboard (lua syntax)
         */
        template <class T>
        void copyInputs(const std::string_view& ns, Collection<T> collection);

    private:
        static bool DrawTreeNode(ramses::LogicObject* obj);

        void drawNodeContextMenu(ramses::LogicNode* obj, const std::string_view& ns);
        void drawOutProperty(const ramses::Property* prop, size_t index);
        static void DrawDataArray(const ramses::DataArray* obj, std::string_view context = std::string_view());

        const ViewerSettings& m_settings;
    };

    template <class T>
    inline void LogicEngineGui::drawCollection(ramses::LogicEngine& logicEngine, std::string_view ns, const char* headline)
    {
        const bool openCollection = ImGui::CollapsingHeader(headline);
        if (ImGui::BeginPopupContextItem(fmt::format("{}ContextMenu", ns).c_str()))
        {
            if (ImGui::MenuItem(fmt::format("{}: Copy all inputs", headline).c_str()))
            {
                copyInputs(ns, logicEngine.getCollection<T>());
            }
            ImGui::EndPopup();
        }
        if (openCollection)
        {
            for (auto* obj : logicEngine.getCollection<T>())
            {
                const bool open = DrawTreeNode(obj);
                drawNodeContextMenu(obj, ns);
                if (open)
                {
                    draw(obj);
                    ImGui::TreePop();
                }
            }
        }
    }

    template<class T, class C>
    void LogicEngineGui::drawCollection(ramses::Collection<T> collection, std::string_view ns, const char* headline, const C& cbDrawItem)
    {
        const bool openCollection = ImGui::TreeNode(headline);
        if (ImGui::BeginPopupContextItem(fmt::format("{}ContextMenu", ns).c_str()))
        {
            if (ImGui::MenuItem(fmt::format("{}: Copy all inputs", headline).c_str()))
            {
                copyInputs(ns, collection);
            }
            ImGui::EndPopup();
        }
        if (openCollection)
        {
            for (auto* obj : collection)
            {
                cbDrawItem(obj);
            }
            ImGui::TreePop();
        }
    }

    template <class T>
    inline void LogicEngineGui::copyInputs(const std::string_view& ns, Collection<T> collection)
    {
        LogicViewerLog::PathVector path;
        path.push_back(LogicViewer::ltnModule);
        path.push_back(ns);
        LogicViewerLog log(m_settings);
        for (auto* node : collection)
        {
            log.logInputs(node, path);
        }
        ImGui::SetClipboardText(log.getText().c_str());
    }
}

