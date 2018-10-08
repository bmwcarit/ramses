############################################################################
#
# Copyright 2014 BMW Car IT GmbH
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

# make sure, every target includes directory of generated build configs.

GET_TARGET_PROPERTY(include_dir_list ${ACME_NAME} INCLUDE_DIRECTORIES)
LIST(APPEND include_dir_list ${CMAKE_BINARY_DIR}/BuildConfig)
LIST(REMOVE_DUPLICATES include_dir_list)
SET_TARGET_PROPERTIES(${ACME_NAME} PROPERTIES INCLUDE_DIRECTORIES "${include_dir_list}")
