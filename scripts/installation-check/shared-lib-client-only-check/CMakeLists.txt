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

FIND_PACKAGE(ramses-shared-lib-client-only ${RAMSES_VERSION} REQUIRED NO_CMAKE_BUILDS_PATH)

# make sure the libraries are found without the need to set LD_LIBRARY_PATH
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${CMAKE_PREFIX_PATH}/lib")

ADD_EXECUTABLE(ramses-shared-lib-check "ramses-shared-lib-check.cpp")
TARGET_LINK_LIBRARIES(ramses-shared-lib-check ${ramses-shared-lib-client-only_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(ramses-shared-lib-check PRIVATE ${ramses-shared-lib-client-only_INCLUDE_DIRS})
