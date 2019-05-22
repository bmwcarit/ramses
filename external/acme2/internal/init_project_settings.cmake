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

# reset PROJECT_<value>
FOREACH(PROPERTY ${ACME2_API})
    SET(PROJECT_${PROPERTY} "")
ENDFOREACH()

# values provided by ACME_PROJECT call are in PROJECT_<value>
cmake_parse_arguments(PROJECT "" "" "${ACME2_API}" ${PROJECT_SETTINGS})

# default values are in DEFAULT_<value>
INCLUDE(${ACME2_BASE_DIR}/internal/init_default_settings.cmake)

# apply project settings or use externally provided values or use default values
FOREACH(PROPERTY ${ACME2_API})
    IF("${PROJECT_${PROPERTY}}" STREQUAL "")
        IF ("${${PROJECT_NAME}_${PROPERTY}}" STREQUAL "")
            SET(PROJECT_${PROPERTY} ${DEFAULT_${PROPERTY}})
        ELSE()
            SET(PROJECT_${PROPERTY} ${${PROJECT_NAME}_${PROPERTY}})
        ENDIF()
    ENDIF()
ENDFOREACH()
