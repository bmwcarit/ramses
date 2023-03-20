#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


function(setSharedTargetProperties)
    cmake_parse_arguments(
        TARGET          # Prefix of parsed args
        ""              # Options
        # Single-value args:
        "NAME;LINK_VISIBILITY"
        # Multi-value-args
        "INCLUDE_PATHS;DEPENDENCIES;PUBLIC_DEFINES;INTERFACE_DEFINES"
        ${ARGN}
    )

    if(TARGET_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed setSharedTargetProperties properties: '${TARGET_UNPARSED_ARGUMENTS}'")
    endif()

    if (TARGET_INCLUDE_PATHS)
        foreach(inc ${TARGET_INCLUDE_PATHS})
            get_filename_component(inc_resolved "${inc}" ABSOLUTE)
            target_include_directories(${TARGET_NAME} PUBLIC "${inc_resolved}")
        endforeach()
    endif()

    if (TARGET_PUBLIC_DEFINES)
        target_compile_definitions(${TARGET_NAME} PUBLIC ${TARGET_PUBLIC_DEFINES})
    endif()

    if (TARGET_INTERFACE_DEFINES)
        target_compile_definitions(${TARGET_NAME} INTERFACE ${TARGET_INTERFACE_DEFINES})
    endif()

    if(NOT TARGET_LINK_VISIBILITY)
        message(FATAL_ERROR "setSharedTargetProperties: must specify linker visibility!")
    endif()

    if(TARGET_DEPENDENCIES)
        foreach(DEPENDENCY ${TARGET_DEPENDENCIES})
            if(TARGET ${DEPENDENCY})
                if("${TARGET_LINK_VISIBILITY}" STREQUAL "PUBLIC")
                    target_link_libraries(${TARGET_NAME} PUBLIC ${DEPENDENCY})
                elseif("${TARGET_LINK_VISIBILITY}" STREQUAL "PRIVATE")
                    target_link_libraries(${TARGET_NAME} PRIVATE ${DEPENDENCY})
                else()
                    # TODO Also support interface as visibility specifier and use it for header-only dependencies
                    message(FATAL_ERROR "linkDependencies: unknown visibility specifier ${TARGET_LINK_VISIBILITY}!")
                endif()
            else()
                # TODO this entire else branch should not be needed with modern CMake
                # ensure it was already found by outside dependency checker
                if (NOT ${DEPENDENCY}_FOUND)
                    message(FATAL_ERROR "${TARGET_NAME}: Missing dependency ${DEPENDENCY}")
                endif()

                # link includes and libs from vars
                if (${DEPENDENCY}_INCLUDE_DIRS)
                    target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${${DEPENDENCY}_INCLUDE_DIRS})
                endif()

                if(${DEPENDENCY}_LIBRARIES)
                    target_link_libraries(${TARGET_NAME} PUBLIC ${${DEPENDENCY}_LIBRARIES})
                endif()
            endif()
        endforeach()
    endif()

    folderizeTarget(${TARGET_NAME})
endfunction()

function(createModule)
    cmake_parse_arguments(
        MODULE              # Prefix of parsed args
        ""  # Options
        # Single-value-args
        "NAME;TYPE;ENABLE_INSTALL"
        # Multi-value-args
        "SRC_FILES;INCLUDE_PATHS;DEPENDENCIES;RESOURCE_FOLDERS;PUBLIC_DEFINES;INTERFACE_DEFINES"
        ${ARGN}
    )

    # Check for configuration errors
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed createModule properties: '${MODULE_UNPARSED_ARGUMENTS}'")
    endif()

    # resolve file wildcards
    file(GLOB GLOBBED_MODULE_SRC_FILES LIST_DIRECTORIES false ${MODULE_SRC_FILES})

    # check, if all dependencies can be resolved
    # TODO This can be for sure modernized with newer CMake functionality for dependency resolving
    set(MSG "")
    set(MODULE_BUILD_ENABLED TRUE)
    foreach(DEPENDENCY ${MODULE_DEPENDENCIES})
        if(NOT TARGET ${DEPENDENCY})
            list(FIND GLOBAL_WILL_NOT_FIND_DEPENDENCY ${DEPENDENCY} SKIP_FIND)

            if(NOT ${DEPENDENCY}_FOUND AND SKIP_FIND EQUAL -1)
                find_package(${DEPENDENCY} QUIET)
            endif()
            if(NOT ${DEPENDENCY}_FOUND)
                set(GLOBAL_WILL_NOT_FIND_DEPENDENCY "${DEPENDENCY};${GLOBAL_WILL_NOT_FIND_DEPENDENCY}" CACHE INTERNAL "")

                list(APPEND MSG "missing ${DEPENDENCY}")
                if(GLOBAL_MODULE_DEPENDENCY_CHECK)
                    set(MODULE_BUILD_ENABLED FALSE)
                endif()
            endif()
        endif()
    endforeach()

    if(NOT MODULE_BUILD_ENABLED)
        message(STATUS "- ${MODULE_NAME} [${MSG}]")
        set(GLOBAL_WILL_NOT_FIND_DEPENDENCY "${MODULE_NAME};${GLOBAL_WILL_NOT_FIND_DEPENDENCY}" CACHE INTERNAL "")
        return()
    endif()

    if(NOT "${MSG}" STREQUAL "")
        message("        build enabled, but")
        foreach(M ${MSG})
            message("        - ${M}")
        endforeach()
        message(FATAL_ERROR "aborting configuration")
    endif()

    message(STATUS "+ ${MODULE_NAME} (${MODULE_TYPE})")

    # TODO create more than one components to make it possible to install libs, tools and test content separately
    set(INSTALL_COMPONENT "ramses-sdk-${PROJECT_VERSION}")

    set(LINK_VISIBILITY PRIVATE)
    if("${MODULE_TYPE}" STREQUAL "BINARY")
        add_executable(${MODULE_NAME} ${GLOBBED_MODULE_SRC_FILES})
        set_target_properties(${MODULE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

        # TODO Replace this by a simple script which only copies resources, never installs them
        if(MODULE_RESOURCE_FOLDERS)
            copyResourcesForTarget(
                TARGET ${MODULE_NAME}
                FOLDERS ${MODULE_RESOURCE_FOLDERS}
                INSTALL ${MODULE_ENABLE_INSTALL}
                INSTALL_COMPONENT ${MODULE_INSTALL_COMPONENT})
        endif()
    elseif("${MODULE_TYPE}" STREQUAL "STATIC_LIBRARY")
        add_library(${MODULE_NAME} STATIC ${GLOBBED_MODULE_SRC_FILES})
        # TODO this looks wrong, should not link all static libs dependencies public! Make settable and fix
        set(LINK_VISIBILITY PUBLIC)
    elseif("${MODULE_TYPE}" STREQUAL "SHARED_LIBRARY")
        add_library(${MODULE_NAME} SHARED ${GLOBBED_MODULE_SRC_FILES})
    else()
        message(FATAL_ERROR "Invalid module type '${MODULE_TYPE}'!")
    endif()

    setSharedTargetProperties(
        NAME ${MODULE_NAME}
        INCLUDE_PATHS ${MODULE_INCLUDE_PATHS}
        DEPENDENCIES ${MODULE_DEPENDENCIES}
        LINK_VISIBILITY ${LINK_VISIBILITY}
        PUBLIC_DEFINES ${MODULE_PUBLIC_DEFINES}
        INTERFACE_DEFINES ${MODULE_INTERFACE_DEFINES}
    )

    if(MODULE_ENABLE_INSTALL)
        # Special case for MSVC which expects DLLs in the executable path
        # TODO maybe there is a better solution with modern CMake?
        if("${MODULE_TYPE}" STREQUAL "BINARY" OR MSVC)
            set(INSTALL_DEST ${RAMSES_INSTALL_RUNTIME_PATH})
        else()
            set(INSTALL_DEST ${RAMSES_INSTALL_LIBRARY_PATH})
        endif()

        install(TARGETS ${MODULE_NAME} DESTINATION ${INSTALL_DEST} COMPONENT "${MODULE_INSTALL_COMPONENT}")
        if (MSVC)
            install(FILES $<TARGET_PDB_FILE:${MODULE_NAME}> DESTINATION ${RAMSES_INSTALL_RUNTIME_PATH} CONFIGURATIONS Debug RelWithDebInfo)
        endif()
    endif()
endfunction()

set(PLATFORM_LIST
    "x11-egl-es-3-0"

    "wayland-ivi-egl-es-3-0"
    "wayland-shell-egl-es-3-0"

    "windows-wgl-es-3-0"
    "windows-wgl-4-2-core"
    "windows-wgl-4-5"

    "android-egl-es-3-0"
)

# Creates N targets <name>-<platform> for each <platform> in PLATFORM_LIST
function(expandModuleByPlatform)
    cmake_parse_arguments(
        MODULE              # Prefix of parsed args
        ""  # Options
        # Single-value-args
        "NAME;TYPE;ENABLE_INSTALL;LINKED_TYPE"
        # Multi-value-args
        "SRC_FILES;INCLUDE_PATHS;DEPENDENCIES;RESOURCE_FOLDERS"
        ${ARGN}
    )
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed expandModuleByPlatform properties: '${MODULE_UNPARSED_ARGUMENTS}'")
    endif()

    # TODO remove this global setting altogether. For now, this keeps the behavior unchanged
    set(GLOBAL_MODULE_DEPENDENCY_CHECK_OLD ${GLOBAL_MODULE_DEPENDENCY_CHECK})
    set(GLOBAL_MODULE_DEPENDENCY_CHECK ON)

    foreach(PLATFORM_NAME ${PLATFORM_LIST})
        if (TARGET platform-${PLATFORM_NAME})
            set(MODULE_NAME_EXTENDED "${MODULE_NAME}-${PLATFORM_NAME}")

            # TODO Remove this dependency hack
            if(${MODULE_LINKED_TYPE} STREQUAL "STATIC")
                set(MODULE_DEPENDENCIES "ramses-renderer-lib;platform-${PLATFORM_NAME};ramses-build-options-base;${MODULE_DEPENDENCIES}")
            elseif(${MODULE_LINKED_TYPE} STREQUAL "DYNAMIC")
                set(MODULE_DEPENDENCIES "ramses-shared-lib-${PLATFORM_NAME};ramses-build-options-base;${MODULE_DEPENDENCIES}")
            else()
                message(FATAL_ERROR "Unrecognized value '${MODULE_LINKED_TYPE}' for parameter LINKED_TYPE")
            endif()

            createModule(NAME                   ${MODULE_NAME_EXTENDED}
                        TYPE                    ${MODULE_TYPE}
                        ENABLE_INSTALL          ${MODULE_ENABLE_INSTALL}
                        SRC_FILES               ${MODULE_SRC_FILES}
                        INCLUDE_PATHS           ${MODULE_INCLUDE_PATHS}
                        DEPENDENCIES            ${MODULE_DEPENDENCIES}
                        RESOURCE_FOLDERS        ${MODULE_RESOURCE_FOLDERS}
                        )

            # TODO: this can be removed - move to general shared lib handling
            if("${MODULE_TYPE}" STREQUAL "SHARED_LIBRARY" AND TARGET ${MODULE_NAME_EXTENDED})
                set_target_properties(${MODULE_NAME_EXTENDED} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
                get_target_property(MODULE_SOVERSION ${MODULE_NAME_EXTENDED} SOVERSION)
                message(STATUS "    setting shared library version to ${MODULE_SOVERSION}")
            endif()
        endif()

    endforeach()

    set(GLOBAL_MODULE_DEPENDENCY_CHECK ${GLOBAL_MODULE_DEPENDENCY_CHECK_OLD})

endfunction()

function(MODULE_WITH_SHARED_LIBRARY)
    foreach(PLATFORM_NAME ${PLATFORM_LIST})
        set(RAMSES_PLATFORM_SHLIB_NAME ramses-shared-lib-${PLATFORM_NAME})
        if (TARGET ${RAMSES_PLATFORM_SHLIB_NAME})
            createModule(
                DEPENDENCIES               ${RAMSES_PLATFORM_SHLIB_NAME}
                                           ramses-build-options-base
                ${ARGN})
            break()
        endif()
    endforeach()
endfunction()

function(RENDERER_TEST_PER_CONFIG MODULE_PREFIX_NAME TEST_SUFFIX)
    foreach(PLATFORM_NAME ${PLATFORM_LIST})
        if (TARGET platform-${PLATFORM_NAME})
            set(TEST_TARGET "${MODULE_PREFIX_NAME}-${PLATFORM_NAME}")

            makeTestFromTarget(
                TARGET ${TEST_TARGET}
                SUFFIX ${TEST_SUFFIX})
        endif()
    endforeach()
endfunction()
