#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_library(ramses-viewer-lib INTERFACE)
target_sources(ramses-viewer-lib INTERFACE
    LogicViewer.h
    LogicViewer.cpp
    LogicViewerLog.h
    LogicViewerLog.cpp
    LogicViewerLuaTypes.h
    LogicViewerLuaTypes.cpp
    ProgressMonitor.h
    ResourceList.h
    Result.h
    SceneSetup.h
    UpdateReportSummary.h
    ValidationCache.h
    ViewerApp.h
    ViewerApp.cpp
    ViewerHeadlessApp.h
    ViewerHeadlessApp.cpp
    ViewerSettings.h
    ViewerSettings.cpp
)
target_link_libraries(ramses-viewer-lib INTERFACE imgui fmt::fmt ramses-framework-cli ramses-client)

add_library(ramses-viewer-gui-lib INTERFACE)
target_sources(ramses-viewer-gui-lib INTERFACE
    LogicEngineGui.h
    LogicEngineGui.cpp
    LogicViewerGui.h
    LogicViewerGui.cpp
    SceneViewerGui.h
    SceneViewerGui.cpp
    ViewerGui.h
    ViewerGui.cpp
    ViewerGuiApp.h
    ViewerGuiApp.cpp
)
target_link_libraries(ramses-viewer-gui-lib INTERFACE ramses-viewer-lib ramses-imgui)

if(TARGET ramses-renderer)
    createModule(
        NAME            ramses-viewer
        TYPE            BINARY
        ENABLE_INSTALL  ON
        SRC_FILES       main.cpp
        DEPENDENCIES    ramses-viewer-gui-lib
                        ramses-renderer
    )
endif()

createModule(
    NAME            ramses-viewer-headless
    TYPE            BINARY
    ENABLE_INSTALL  ON
    SRC_FILES       main_headless.cpp
    DEPENDENCIES    ramses-viewer-lib
)
