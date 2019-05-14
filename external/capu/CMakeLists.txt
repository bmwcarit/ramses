#
# Copyright (C) 2012 BMW Car IT GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# check for required cmake vars
IF (NOT DEFINED TARGET_OS OR NOT DEFINED TARGET_ARCH OR "${TARGET_OS}" MATCHES "unknown" OR "${TARGET_ARCH}" MATCHES "unknown" OR NOT TARGET_BITNESS)
    ACME_ERROR("TARGET_OS, TARGET_ARCH and TARGET_BITNESS must be set")
ENDIF()

# set vars for defines and Config.h.in
# infer from cmake vars (vars also used in Config.h.in)
STRING(TOUPPER "OS_${TARGET_OS}" CAPU_OS_COMPILE_DEFINITION)
SET(CAPU_ARCH_BITNESS_COMPILE_DEFINITION "CAPU_ARCH_${TARGET_BITNESS}")

# all POSIX system must include this source file,
# which currently refers to all supported platforms
# except windows
IF (NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET(PLATFORM_SPECIFIC_SOURCE_FILES src/os/Posix/*.cpp)
    SET(PLATFORM_SPECIFIC_PUBLIC_HEADER_FILES include/ramses-capu/os/Posix/*.h)
ENDIF()

CONFIGURE_FILE(include/ramses-capu/Config.h.in ${PROJECT_BINARY_DIR}/ramses-capu/Config.h)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-capu
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/ramses-capu/*.h
                            include/ramses-capu/container/*.h
                            include/ramses-capu/os/*.h
                            include/ramses-capu/os/Generic/*.h
                            include/ramses-capu/os/${TARGET_OS}/*.h
                            include/ramses-capu/util/*.h
                            ${PROJECT_BINARY_DIR}/ramses-capu/*.h
                            ${PLATFORM_SPECIFIC_PUBLIC_HEADER_FILES}
                            include/ramses-capu/Config.h.in

    FILES_SOURCE            src/os/${TARGET_OS}/*.cpp
                            src/util/*.cpp
                            ${PLATFORM_SPECIFIC_SOURCE_FILES}


    INCLUDE_BASE            include
                            ${PROJECT_BINARY_DIR}
)

if (NOT "${TARGET_OS}" STREQUAL "Integrity")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    TARGET_LINK_LIBRARIES(ramses-capu PUBLIC Threads::Threads)
endif()

# conditionally link libs
if ("${TARGET_OS}" STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(ramses-capu PUBLIC psapi)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        # add windows sockets in mingw case
        TARGET_LINK_LIBRARIES(ramses-capu PUBLIC ws2_32 wsock32)
    endif()
elseif ("${TARGET_OS}" STREQUAL "Linux")
    TARGET_LINK_LIBRARIES(ramses-capu PUBLIC rt)
endif()

# set defines public to target
TARGET_COMPILE_DEFINITIONS(ramses-capu PUBLIC
    TARGET_OS=${TARGET_OS}
    ${CAPU_OS_COMPILE_DEFINITION}
    ${CAPU_ARCH_BITNESS_COMPILE_DEFINITION})


ACME_MODULE(
   #==========================================================================
   # general module information
   #==========================================================================
   NAME                    ramses-capu-test
   TYPE                    TEST
   ENABLE_INSTALL          ON
   #==========================================================================
   # files of this module
   #==========================================================================
   INCLUDE_BASE            test/src
   FILES_SOURCE            test/src/*.h
                           test/src/*.cpp
                           test/src/container/*.h
                           test/src/container/*.cpp
                           test/src/os/*.h
                           test/src/os/*.cpp
                           test/src/util/*.h
                           test/src/util/*.cpp
   #==========================================================================
   # dependencies
   #==========================================================================
   DEPENDENCIES            ramses-capu
                           ramses-gmock-main
   )

IF(TARGET ramses-capu-test AND "${TARGET_OS}" STREQUAL "Integrity")
    TARGET_COMPILE_OPTIONS(ramses-capu-test PRIVATE "-no_misalign_pack")
ENDIF()
