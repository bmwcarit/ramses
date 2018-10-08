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

GET_TARGET_PROPERTY(current_debug_flags ${ACME_NAME} COMPILE_DEFINITIONS)
IF(current_debug_flags STREQUAL "current_debug_flags-NOTFOUND")
    SET(current_debug_flags "")
ELSE()
    SET(current_debug_flags "${current_debug_flags} ")
ENDIF()

SET(current_debug_flags "${current_debug_flags} -fprofile-arcs -ftest-coverage" )
SET_TARGET_PROPERTIES(${ACME_NAME} PROPERTIES COMPILE_FLAGS "${current_debug_flags}")

TARGET_LINK_LIBRARIES(${ACME_NAME} gcov)
