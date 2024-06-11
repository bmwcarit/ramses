#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

###################################
######## API HEADERS LIB ##########
###################################

add_library(ramses-api INTERFACE)
target_include_directories(ramses-api INTERFACE .)
target_link_libraries(ramses-api INTERFACE glm::glm)

# framework
file(GLOB
    RAMSES_FRAMEWORK_API_FILES_HEADER
    ramses/framework/*.h)

# client
file(GLOB
    RAMSES_CLIENT_API_FILES_HEADER
    ramses/client/*.h)

if(ramses-sdk_TEXT_SUPPORT)
    file(GLOB
        RAMSES_CLIENT_TEXT_API_FILES_HEADER
        ramses/client/text/*.h)

    target_compile_definitions(ramses-api INTERFACE RAMSES_TEXT_ENABLED)
endif()

file(GLOB
    RAMSES_CLIENT_LOGIC_API_FILES_HEADER
    ramses/client/logic/*.h)

# renderer
if(ANY_WINDOW_TYPE_ENABLED)
    file(GLOB
        RAMSES_RENDERER_API_FILES_HEADER
        ramses/renderer/*.h)
endif()

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19.0")
    target_sources(ramses-api PRIVATE   ${RAMSES_FRAMEWORK_API_FILES_HEADER}
                                        ${RAMSES_CLIENT_API_FILES_HEADER}
                                        ${RAMSES_CLIENT_TEXT_API_FILES_HEADER}
                                        ${RAMSES_CLIENT_LOGIC_API_FILES_HEADER}
                                        ${RAMSES_RENDERER_API_FILES_HEADER})

    folderizeTarget(ramses-api)
endif()

###################################
############ INSTALL ##############
###################################

if(ramses-sdk_ENABLE_INSTALL)
    install(FILES ${RAMSES_FRAMEWORK_API_FILES_HEADER} DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}/ramses/framework" COMPONENT ramses-sdk-devel)

    install(FILES ${RAMSES_CLIENT_API_FILES_HEADER} DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}/ramses/client" COMPONENT ramses-sdk-devel)
    if(ramses-sdk_TEXT_SUPPORT)
        install(FILES ${RAMSES_CLIENT_TEXT_API_FILES_HEADER} DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}/ramses/client/text" COMPONENT ramses-sdk-devel)
    endif()
    install(FILES ${RAMSES_CLIENT_LOGIC_API_FILES_HEADER} DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}/ramses/client/logic" COMPONENT ramses-sdk-devel)

    if(ANY_WINDOW_TYPE_ENABLED)
        install(FILES ${RAMSES_RENDERER_API_FILES_HEADER} DESTINATION "${RAMSES_INSTALL_HEADERS_PATH}/ramses/renderer" COMPONENT ramses-sdk-devel)
    endif()
endif()
