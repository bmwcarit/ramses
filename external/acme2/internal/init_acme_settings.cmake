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

# translate all MODULE_* provided by ACME2 module
# to checked and preprocessed ACME_* variables

# init acme settings with module settings
FOREACH(PROPERTY ${ACME2_API})
    SET(ACME_${PROPERTY} ${MODULE_${PROPERTY}})
ENDFOREACH()

# resolve file wildcards
GET_ALL_FILES(ACME_FILES_PRIVATE_HEADER    "${MODULE_FILES_PRIVATE_HEADER}")
GET_ALL_FILES(ACME_FILES_SOURCE            "${MODULE_FILES_SOURCE}")
GET_ALL_FILES(ACME_FILES_RESOURCE          "${MODULE_FILES_RESOURCE}")

FILE(GLOB ACME_INCLUDE_BASE ${MODULE_INCLUDE_BASE})

SET(ACME_PACKAGE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
