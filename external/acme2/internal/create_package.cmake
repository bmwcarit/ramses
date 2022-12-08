############################################################################
#
# Copyright (C) 2014 BMW Car IT GmbH
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
############################################################################

set(CPACK_GENERATOR ${ramses_CPACK_GENERATOR})

SET(CPACK_SOURCE_GENERATOR "TGZ")

IF (NOT DEFINED GIT_COMMIT_COUNT OR NOT DEFINED GIT_COMMIT_HASH)
    message(FATAL_ERROR "GIT_COMMIT_COUNT and GIT_COMMIT_HASH must be set")
ENDIF()
SET(SCM_VERSION "${GIT_COMMIT_COUNT}-${GIT_COMMIT_HASH}")

IF("${CPACK_PACKAGE_NAME}" STREQUAL "")
    SET(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
ENDIF()

SET(CPACK_PACKAGE_VERSION               "${PROJECT_VERSION_STRING}-${SCM_VERSION}")
SET(CPACK_PACKAGE_CONTACT               "ramses-oss@list.bmw.com")
SET(CPACK_SOURCE_STRIP_FILES            TRUE)
set(CPACK_STRIP_FILES                   FALSE)
set(CPACK_PACKAGE_VENDOR                "ramses")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY   "A distributed 3D rendering framework for embedded systems")
set(CPACK_PACKAGE_DESCRIPTION           "A packaged version of ramses. Generated using CPack.")
set(CPACK_PACKAGE_VERSION_MAJOR         ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR         ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH         ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE         "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README          "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_ICON                  "${CMAKE_CURRENT_SOURCE_DIR}/doc/general/images/ramses_logo_with_alpha2.png")

if(ramses_CPACK_GENERATOR STREQUAL "DEB")
    # Enables CPack to add proper dependency info to the package, see docs for more info
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()

# Allows providing custom package suffix, use "-<commitsha>" by default
if(RAMSES_CUSTOM_PACKAGE_SUFFIX)
    set(PACKAGE_SUFFIX ${RAMSES_CUSTOM_PACKAGE_SUFFIX})
else()
    set(PACKAGE_SUFFIX ${SCM_VERSION})
endif()

SET(CPACK_PACKAGE_FILE_NAME  "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION_STRING}-${PACKAGE_SUFFIX}")

INCLUDE(CPack)

