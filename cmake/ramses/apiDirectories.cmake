#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# This CMake file contains CMake variables containing directory paths which contain
# a) RAMSES API header files (RAMSES_XXX_API_HEADERS)
# b) RAMSES source files (RAMSES_XXX_FILES_SOURCE)
# for RAMSES framework, RAMSES client and RAMSES renderer

########################## RAMSES FRAMEWORK API DIRECTORY VARIABLES #######################

SET(RAMSES_FRAMEWORK_API_INCLUDE_BASE
                            framework/ramses-framework-api/include/)
SET(RAMSES_FRAMEWORK_API_HEADERS
                            framework/ramses-framework-api/include/*.h
                            framework/ramses-framework-api/include/ramses-framework-api/*.h)

########################## RAMSES CLIENT API DIRECTORY VARIABLES ###########################

SET(RAMSES_CLIENT_API_INCLUDE_BASE
                            client/ramses-client-api/include/)
SET(RAMSES_CLIENT_API_HEADERS
                            client/ramses-client-api/include/*.h
                            client/ramses-client-api/include/ramses-client-api/*.h
                            client/ramses-client-api/include/ramses-client-api/text/*.h)


######################## RAMSES RENDERER API DIRECTORY VARIABLES ###########################

SET(RAMSES_RENDERER_API_INCLUDE_BASE
                            renderer/ramses-renderer-api/include/)
SET(RAMSES_RENDERER_API_HEADERS
                            renderer/ramses-renderer-api/include/*.h
                            renderer/ramses-renderer-api/include/ramses-renderer-api/*.h)


######################## Helper function to get required relative paths of the above  ###########################

FUNCTION(RAMSES_GET_PATHS_RELATIVE_TO_DIRECTORY REL_TO_SDKROOT_PATHS RELATIVE_TO_DIR OUT_PATHS)
  SET(OUT_PATHS_TMP)
  FOREACH(PATH ${REL_TO_SDKROOT_PATHS})
    FILE(RELATIVE_PATH REL_PATH "${RELATIVE_TO_DIR}" "${ramses-sdk_ROOT_CMAKE_PATH}/${PATH}")
    LIST(APPEND OUT_PATHS_TMP "${REL_PATH}")
  ENDFOREACH()
  SET(${OUT_PATHS} ${OUT_PATHS_TMP} PARENT_SCOPE)
ENDFUNCTION()
