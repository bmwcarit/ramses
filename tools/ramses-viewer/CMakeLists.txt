#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/roboto.h
    COMMAND $<TARGET_FILE:imgui_binary_to_compressed> ${PROJECT_SOURCE_DIR}/examples/ramses-example-text-languages/res/ramses-example-text-languages-Roboto-Regular.ttf roboto_ttf > ${CMAKE_CURRENT_BINARY_DIR}/roboto.h
    COMMENT "Generate roboto.h"
    )

add_custom_target(roboto_gen DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/roboto.h)
folderizeTarget(roboto_gen)

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
    RemoteScenesGui.h
    RemoteScenesGui.cpp
    RendererControl.h
    RendererControl.cpp
    SceneViewerGui.h
    SceneViewerGui.cpp
    StreamViewer.h
    StreamViewer.cpp
    ViewerGui.h
    ViewerGui.cpp
    ViewerGuiApp.h
    ViewerGuiApp.cpp
)
target_link_libraries(ramses-viewer-gui-lib INTERFACE ramses-viewer-lib ramses-imgui)
target_include_directories(ramses-viewer-gui-lib INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(ramses-viewer-gui-lib roboto_gen)

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
