#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set(CPACK_GENERATOR ${ramses-sdk_CPACK_GENERATOR})

set(CPACK_SOURCE_GENERATOR "TGZ")

if (NOT DEFINED GIT_COMMIT_COUNT OR NOT DEFINED GIT_COMMIT_HASH)
    message(FATAL_ERROR "GIT_COMMIT_COUNT and GIT_COMMIT_HASH must be set")
endif()
set(SCM_VERSION "${GIT_COMMIT_COUNT}-${GIT_COMMIT_HASH}")

if("${CPACK_PACKAGE_NAME}" STREQUAL "")
    set(CPACK_PACKAGE_NAME "ramses")
endif()

set(CPACK_PACKAGE_VERSION                "${RAMSES_VERSION}-${SCM_VERSION}")
set(CPACK_SOURCE_STRIP_FILES             TRUE)
set(CPACK_STRIP_FILES                    FALSE)
set(CPACK_PACKAGE_CONTACT               "ramses-oss@list.bmw.com")
set(CPACK_PACKAGE_VENDOR                "ramses")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY   "A distributed 3D rendering framework for embedded systems")
set(CPACK_PACKAGE_DESCRIPTION           "A packaged version of ramses. Generated using CPack.")

if(ramses-sdk_CPACK_GENERATOR STREQUAL "DEB")
    # Enables CPack to add proper dependency info to the package, see docs for more info
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()

# Allows providing custom package suffix, use "-<commitsha>" by default
if(RAMSES_CUSTOM_PACKAGE_SUFFIX)
    set(PACKAGE_SUFFIX ${RAMSES_CUSTOM_PACKAGE_SUFFIX})
else()
    set(PACKAGE_SUFFIX ${SCM_VERSION})
endif()

set(CPACK_PACKAGE_FILE_NAME  "${CPACK_PACKAGE_NAME}-${RAMSES_VERSION}-${PACKAGE_SUFFIX}")

include(CPack)
