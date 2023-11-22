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
                elseif("${TARGET_LINK_VISIBILITY}" STREQUAL "INTERFACE")
                    target_link_libraries(${TARGET_NAME} INTERFACE ${DEPENDENCY})
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

# macro to import dependencies and check if any are none available. If a missing dependency is found
# the passed variable as 'MISSING_DEPENDENCY_OUT' is overritten with the missing
# dependency name, otherwise it gets unset
macro(importDependenciesAndCheckMissing MISSING_DEPENDENCY_OUT)
    # discard any values in MISSING_DEPENDENCY_OUT if any.
    # If all dependencies are found it becomes undefined
    unset(${MISSING_DEPENDENCY_OUT})

    foreach(DEPENDENCY ${ARGN})
        if((NOT TARGET ${DEPENDENCY}) AND (NOT ${DEPENDENCY}_FOUND))
            # since macro's dont create a new scope, the variables declared
            # in the find scripts will be visible in the calling scope
            find_package(${DEPENDENCY})
            if(NOT ${DEPENDENCY}_FOUND)
                set(${MISSING_DEPENDENCY_OUT} ${DEPENDENCY})
                break()
            endif()
        endif()
    endforeach()
endmacro()

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

    importDependenciesAndCheckMissing(MISSING_DEPENDENCY ${MODULE_DEPENDENCIES})
    if(MISSING_DEPENDENCY)
        message(FATAL_ERROR "Aborting configuration! Missing dependency: ${MISSING_DEPENDENCY} for module: ${MODULE_NAME}!")
    endif()

    message(STATUS "+ ${MODULE_NAME} (${MODULE_TYPE})")

    # TODO create more than one components to make it possible to install libs, tools and test content separately
    set(INSTALL_COMPONENT "ramses-sdk-${PROJECT_VERSION}")

    set(LINK_VISIBILITY PRIVATE)
    if("${MODULE_TYPE}" STREQUAL "INTERFACE_LIB")
        set(LINK_VISIBILITY INTERFACE)
    elseif("${MODULE_TYPE}" STREQUAL "BINARY")
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
    elseif("${MODULE_TYPE}" STREQUAL "OBJECT")
        add_library(${MODULE_NAME} OBJECT ${GLOBBED_MODULE_SRC_FILES})
        set(LINK_VISIBILITY PUBLIC)
    elseif("${MODULE_TYPE}" STREQUAL "SHARED_LIBRARY")
        add_library(${MODULE_NAME} SHARED ${GLOBBED_MODULE_SRC_FILES})

        set_target_properties(${MODULE_NAME} PROPERTIES RAMSES_VERSION "${ramses-sdk_VERSION}")
        set_target_properties(${MODULE_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
        get_target_property(MYMODULE_SOVERSION ${MODULE_NAME} SOVERSION)
        message(STATUS "    setting shared library '${MODULE_NAME}' property RAMSES_VERSION to '${ramses-sdk_VERSION}'")
        message(STATUS "    setting shared library '${MODULE_NAME}' property SOVERSION to '${MYMODULE_SOVERSION}'")

    elseif("${MODULE_TYPE}" STREQUAL "INTERFACE_LIB")
        if(GLOBBED_MODULE_SRC_FILES)
            message(FATAL_ERROR "Can't have interface library with source files attached!")
        endif()
        add_library(${MODULE_NAME} INTERFACE)
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

    if(MODULE_ENABLE_INSTALL AND ramses-sdk_ENABLE_INSTALL)
        if("${MODULE_TYPE}" STREQUAL "INTERFACE_LIB" OR "${MODULE_TYPE}" STREQUAL "OBJECT")
            message(FATAL_ERROR "Can't install interface libraries or objects!")
        endif()

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


function(createModuleWithRenderer)
    cmake_parse_arguments(
        MODULE              # Prefix of parsed args
        ""  # Options
        # Single-value-args
        "NAME;TYPE;ENABLE_INSTALL"
        # Multi-value-args
        "SRC_FILES;INCLUDE_PATHS;DEPENDENCIES;RESOURCE_FOLDERS"
        ${ARGN}
    )
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed createModuleWithRenderer properties: '${MODULE_UNPARSED_ARGUMENTS}'")
    endif()

    set(MODULE_DEPENDENCIES "ramses-renderer;ramses-client;ramses-build-options-base;${MODULE_DEPENDENCIES}")

    createModule(NAME                   ${MODULE_NAME}
                TYPE                    ${MODULE_TYPE}
                ENABLE_INSTALL          ${MODULE_ENABLE_INSTALL}
                SRC_FILES               ${MODULE_SRC_FILES}
                INCLUDE_PATHS           ${MODULE_INCLUDE_PATHS}
                DEPENDENCIES            ${MODULE_DEPENDENCIES}
                RESOURCE_FOLDERS        ${MODULE_RESOURCE_FOLDERS}
                )

    target_link_libraries(${MODULE_NAME} INTERFACE ramses-api)
endfunction()
