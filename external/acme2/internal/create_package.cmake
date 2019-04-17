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

SET(CPACK_GENERATOR "TGZ")
SET(CPACK_SOURCE_GENERATOR "TGZ")

IF (NOT DEFINED GIT_COMMIT_COUNT OR NOT DEFINED GIT_COMMIT_HASH)
    message(FATAL_ERROR "GIT_COMMIT_COUNT and GIT_COMMIT_HASH must be set")
ENDIF()
SET(SCM_VERSION "${GIT_COMMIT_COUNT}-${GIT_COMMIT_HASH}")

IF("${CPACK_PACKAGE_NAME}" STREQUAL "")
	SET(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
ENDIF()

#SET(CPACK_RPM_COMPONENT_INSTALL         1)
#SET(CPACK_ARCHIVE_COMPONENT_INSTALL     1)
SET(CPACK_PACKAGE_VERSION                "${PROJECT_VERSION_STRING}-${SCM_VERSION}")
SET(CPACK_PACKAGE_CONTACT                "${PROJECT_CONTACT}")
SET(CPACK_SOURCE_STRIP_FILES             TRUE)
SET(CPACK_STRIP_FILES                    FALSE)

SET(CPACK_PACKAGE_FILE_NAME  "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION_STRING}-${SCM_VERSION}")

INCLUDE(CPack)
