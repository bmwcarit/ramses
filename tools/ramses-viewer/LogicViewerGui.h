//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ImguiClientHelper.h"
#include "ramses/client/logic/Collection.h"
#include <array>

namespace ramses
{
    class Scene;
    class RamsesRenderer;
    class LogicEngine;
    class Property;
    class LogicObject;
    class LogicNode;
    class DataArray;
}

namespace ramses::internal
{
    class LogicViewer;
    struct ViewerSettings;

    class LogicViewerGui
    {
    public:
        /**
        * @param luafile the luafile argument provided by command line
        *        If it does not exist the LogicViewerGui will provide a UI to save the default settings
        */
        explicit LogicViewerGui(LogicViewer& viewer, ViewerSettings& settings, std::string luafile);
        void draw();
        void openErrorPopup(const std::string& message);
        void setSceneTexture(ramses::TextureSampler* sampler, uint32_t width, uint32_t height);
        void setRendererInfo(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId, ramses::displayBufferId_t displayBufferId, const glm::vec4& initialClearColor);

    private:
        void drawMenuItemShowWindow();
        void drawMenuItemReload();
        void drawMenuItemCopy();

        void drawGlobalContextMenu();
        void drawSceneTexture();
        void drawErrorPopup();
        void drawWindow();
        void drawMenuBar();
        void drawCurrentView();
        void drawInterfaces();
        void drawScripts();
        void drawAnimationNodes();
        void drawTimerNodes();
        void drawNodeBindings();
        void drawCameraBindings();
        void drawRenderPassBindings();
        void drawRenderGroupBindings();
        void drawMeshNodeBindings();
        void drawAnchorPoints();
        void drawSkinBindings();
        void drawUpdateReport();
        void drawAppearanceBindings();
        void drawDisplaySettings();
        void drawSaveDefaultLuaFile();

        static bool DrawTreeNode(ramses::LogicObject* obj);

        void drawNodeContextMenu(ramses::LogicNode* obj, const std::string_view& ns);
        void drawNode(ramses::LogicNode* obj);
        void drawProperty(ramses::Property* prop, size_t index);
        void drawOutProperty(const ramses::Property* prop, size_t index);
        static void DrawDataArray(const ramses::DataArray* obj, std::string_view context = std::string_view());

        void reloadConfiguration();
        void loadLuaFile(const std::string& filename);
        void saveDefaultLuaFile();

        /**
         * copies all node inputs to clipboard (lua syntax)
         */
        template <class T>
        void copyInputs(const std::string_view& ns, Collection<T> collection);
        void copyScriptInputs();

        ViewerSettings& m_settings;
        LogicViewer& m_viewer;
        ramses::LogicEngine& m_logicEngine;

        std::string m_lastErrorMessage;
        std::string m_filename;
        size_t m_updateReportInterval = 60u;

        ramses::TextureSampler* m_sampler = nullptr;
        ImVec2 m_samplerSize;

        ramses::RamsesRenderer* m_renderer = nullptr;
        ramses::displayId_t m_displayId;
        ramses::displayBufferId_t m_displayBufferId;
        glm::vec4 m_clearColor{ 0, 0, 0, 1 };
        bool m_skipUnmodifiedBuffers = true;
    };
}

