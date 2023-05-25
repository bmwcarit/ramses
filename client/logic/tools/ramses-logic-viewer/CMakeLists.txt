#  -------------------------------------------------------------------------
#  Copyright (C) 2021 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set(logic_viewer_header
    LogicViewerHeadlessApp.h
    LogicViewerApp.h
    LogicViewer.h
    LogicViewerLog.h
    LogicViewerSettings.h
    LogicViewerLuaTypes.h
    UpdateReportSummary.h
    Arguments.h
    SceneSetup.h)

set(logic_viewer_src
    LogicViewerHeadlessApp.cpp
    LogicViewerApp.cpp
    LogicViewer.cpp
    LogicViewerLog.cpp
    LogicViewerSettings.cpp
    LogicViewerLuaTypes.cpp)

add_library(ramses-logic-viewer-lib INTERFACE)
target_sources(ramses-logic-viewer-lib INTERFACE ${logic_viewer_header})
target_sources(ramses-logic-viewer-lib INTERFACE ${logic_viewer_src})
target_include_directories(ramses-logic-viewer-lib INTERFACE ${PROJECT_SOURCE_DIR}/client/logic/lib)
target_link_libraries(ramses-logic-viewer-lib INTERFACE imgui fmt::fmt sol2::sol2 lua::lua ramses-cli)

add_library(ramses-logic-viewer-gui-lib INTERFACE)
target_sources(ramses-logic-viewer-gui-lib INTERFACE
    LogicViewerGuiApp.h
    LogicViewerGui.h
    LogicViewerGuiApp.cpp
    LogicViewerGui.cpp
    ImguiClientHelper.h
    ImguiClientHelper.cpp)
target_link_libraries(ramses-logic-viewer-gui-lib INTERFACE ramses-logic-viewer-lib)

expandModuleByPlatform(
    NAME            ramses-logic-viewer
    TYPE            BINARY
    LINKED_TYPE     DYNAMIC
    ANY_PLATFORM
    ENABLE_INSTALL  ${ramses-sdk_ENABLE_INSTALL}
    SRC_FILES       main.cpp
    DEPENDENCIES    ramses-logic-viewer-gui-lib
)

if(ramses-sdk_BUILD_HEADLESS_SHARED_LIB)
    createModule(
        NAME                    ramses-logic-viewer-headless
        TYPE                    BINARY
        ENABLE_INSTALL          ON
        SRC_FILES               main_headless.cpp
        DEPENDENCIES            ramses-logic-viewer-lib ramses-shared-lib-headless
    )

    if (ramses-sdk_ENABLE_INSTALL)
        install(
            TARGETS ramses-logic-viewer-headless
            DESTINATION ${RAMSES_INSTALL_RUNTIME_PATH})
    endif()
endif()
