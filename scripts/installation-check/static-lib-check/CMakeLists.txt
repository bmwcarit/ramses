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

FIND_PACKAGE(ramses-static-lib ${RAMSES_VERSION} REQUIRED COMPONENTS ${RAMSES_RENDERER_PLATFORM} ${GL_VERSION} NO_CMAKE_BUILDS_PATH)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(PLATFORM_DEPENDANT_DEPENDENCIES rt pthread)
ELSE()
    SET(PLATFORM_DEPENDANT_DEPENDENCIES "")
ENDIF()

ADD_EXECUTABLE(ramses-static-lib-check "ramses-static-lib-check.cpp")
TARGET_LINK_LIBRARIES(ramses-static-lib-check ${ramses-static-lib_LIBRARIES} ${PLATFORM_DEPENDANT_DEPENDENCIES})
TARGET_INCLUDE_DIRECTORIES(ramses-static-lib-check PRIVATE ${ramses-static-lib_INCLUDE_DIRS})

