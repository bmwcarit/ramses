#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

get_filename_component(GLM_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
get_filename_component(GLM_INSTALL_PREFIX "${GLM_INSTALL_PREFIX}" DIRECTORY)
get_filename_component(GLM_INSTALL_PREFIX "${GLM_INSTALL_PREFIX}" DIRECTORY)
get_filename_component(GLM_INSTALL_PREFIX "${GLM_INSTALL_PREFIX}" DIRECTORY)
get_filename_component(GLM_INSTALL_PREFIX "${GLM_INSTALL_PREFIX}" DIRECTORY)

# we always use the Khronos reference headers
SET(GLM_INCLUDE_DIRS
    ${GLM_INSTALL_PREFIX}/external/glm
)

MARK_AS_ADVANCED(
    GLM_INCLUDE_DIRS
)

add_library(glm::glm INTERFACE IMPORTED)
set_target_properties(glm::glm PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE_DIRS})


SET(glm_FOUND TRUE)

