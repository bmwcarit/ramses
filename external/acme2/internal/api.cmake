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

SET(ACME2_API

    # Project metadata
    NAME
    TYPE
    ALLOWED_TEST_SUFFIXES
    TEST_SUFFIX
    DESCRIPTION
    URL

    # version
    VERSION_MAJOR
    VERSION_MINOR
    VERSION_PATCH
    VERSION_STRING

    # source paths
    INCLUDE_BASE
    FILES_PRIVATE_HEADER
    FILES_SOURCE
    FILES_RESOURCE
    RESOURCE_FOLDER

    # dependencies
    DEPENDENCIES

    # install module: boolean
    ENABLE_INSTALL

    # content
    CONTENT
)

SET(ACME2_DEFAULT_VALUES

    # Project metadata
    NAME                    "default_name"
    TYPE                    STATIC_LIBRARY
    ALLOWED_TEST_SUFFIXES   "UNITTEST"
    TEST_SUFFIX             "UNITTEST"
    DESCRIPTION             "default_description"
    URL                     "default_url"

    # version
    VERSION_MAJOR           0
    VERSION_MINOR           0
    VERSION_PATCH           0
    VERSION_STRING          "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"

    # source paths
    INCLUDE_BASE            include
    FILES_PRIVATE_HEADER    #src/*.h
    FILES_SOURCE            #src/*.h
    FILES_RESOURCE          #res/*
    RESOURCE_FOLDER

    # dependencies
    DEPENDENCIES

    # install module: boolean
    ENABLE_INSTALL          ON

    # content
    CONTENT
)

#provide variables DEFAULT_<ACME2_API>
cmake_parse_arguments(DEFAULT "" "" "${ACME2_API}" ${ACME2_DEFAULT_VALUES})
