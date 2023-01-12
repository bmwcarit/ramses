#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# provide base targets for all ramses libraries

# base interface for platform dependencies
add_library(ramses-common-base INTERFACE)

# add platform specific libraries


IF (("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS") OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin"))
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
ELSEIF (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Integrity")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
ELSEIF (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Integrity")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)        
    target_link_libraries(ramses-common-base INTERFACE Threads::Threads)
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(ramses-common-base INTERFACE psapi)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    TARGET_LINK_LIBRARIES(ramses-common-base INTERFACE rt)
elseif (("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS") OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin"))
    FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    FIND_LIBRARY(COREGRAPHICS_LIBRARY CoreGraphics REQUIRED)
    FIND_LIBRARY(CORETEXT_LIBRARY CoreText REQUIRED)
    TARGET_LINK_LIBRARIES(ramses-common-base INTERFACE ${COREFOUNDATION_LIBRARY})
    TARGET_LINK_LIBRARIES(ramses-common-base INTERFACE ${COREGRAPHICS_LIBRARY})
    TARGET_LINK_LIBRARIES(ramses-common-base INTERFACE ${CORETEXT_LIBRARY})
endif()

# add warning flags
target_link_libraries(ramses-common-base INTERFACE ramses-build-options-base)
