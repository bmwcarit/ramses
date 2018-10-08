#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
set(CMAKE_CXX_STANDARD 11)
PROJECT(examples)

IF (NOT DEFINED RAMSES_RENDERER_PLATFORM)
    SET(RAMSES_RENDERER_PLATFORM "LINUX")
ENDIF()

FIND_PACKAGE(ramses-shared-lib ${RAMSES_VERSION} REQUIRED COMPONENTS ${RAMSES_RENDERER_PLATFORM} ${GL_VERSION} NO_CMAKE_BUILDS_PATH)

ADD_EXECUTABLE(ramses-shared-lib-check "ramses-shared-lib-check.cpp")
TARGET_LINK_LIBRARIES(ramses-shared-lib-check ${ramses-shared-lib_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(ramses-shared-lib-check PRIVATE ${ramses-shared-lib_INCLUDE_DIRS})
