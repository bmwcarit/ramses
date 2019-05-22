#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

INCLUDE(FindPackageHandleStandardArgs)

# try to find system boost (forward all arguments, skip this file)
FIND_PACKAGE(Boost ${Boost_FIND_VERSION} COMPONENTS system log QUIET NO_CMAKE_PATH)

IF (Boost_FOUND)
    SET(Boost_SOURCE_TYPE "system")

ELSE()
    # handle linux and windows separately, not much in common
    IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        # boost path in container
        SET(Boost_BASE_PATH /opt/boost/${TARGET_ARCH}/)

        # try to find libs and includes
        FIND_LIBRARY(boost_system_LIBRARY
            boost_system
            PATHS ${Boost_BASE_PATH}/lib/)
        FIND_LIBRARY(boost_thread_LIBRARY
            boost_thread
            PATHS ${Boost_BASE_PATH}/lib/)
        FIND_LIBRARY(boost_filesystem_LIBRARY
            boost_filesystem
            PATHS ${Boost_BASE_PATH}/lib/)
        FIND_LIBRARY(boost_log_LIBRARY
            boost_log
            PATHS ${Boost_BASE_PATH}/lib/)
        FIND_PATH(Boost_INCLUDE_DIR
            boost/asio/steady_timer.hpp
            PATHS ${Boost_BASE_PATH}/include)

        # do the checking
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost
            FOUND_VAR Boost_FOUND
            REQUIRED_VARS boost_system_LIBRARY boost_thread_LIBRARY boost_filesystem_LIBRARY boost_log_LIBRARY Boost_INCLUDE_DIR)

        # set output vars
        SET(Boost_LIBRARIES ${boost_system_LIBRARY} ${boost_thread_LIBRARY} ${boost_filesystem_LIBRARY} ${boost_log_LIBRARY})
        SET(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIR})

    ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        # do some magic path guessing
        SET(Boost_BASE_PATH "$ENV{Boost_BASE_PATH}" CACHE STRING "Path to boost prebuilt package")

        # guess lib and include paths
        IF (Boost_BASE_PATH)
            MESSAGE(STATUS "Use Boost_BASE_PATH: ${Boost_BASE_PATH}")

            # get bitness
            IF ("${TARGET_ARCH}" STREQUAL "X86_64")
                SET(Boost_LIB_BITNESS "64")
            ELSE()
                SET(Boost_LIB_BITNESS "32")
            ENDIF()

            # get msvc version
            IF ("${MSVC_VERSION}" EQUAL 1900)  # VS2015
                SET(Boost_MSVC_LIB_PATH "14.0")
            ELSEIF ("${MSVC_VERSION}" EQUAL 1910 OR "${MSVC_VERSION}" GREATER 1910)  # VS2017 is 1910 - 1919
                SET(Boost_MSVC_LIB_PATH "14.1")
            ENDIF()

            IF (DEFINED Boost_MSVC_LIB_PATH)
                SET(Boost_LIBRARY_PATH_GUESSED "${Boost_BASE_PATH}/lib${Boost_LIB_BITNESS}-msvc-${Boost_MSVC_LIB_PATH}")
                SET(Boost_INCLUDE_PATH_GUESSED "${Boost_BASE_PATH}")
            ENDIF()
        ENDIF()

        # these are the final search paths
        SET(Boost_LIBRARY_SEARCH_PATH "" CACHE STRING "Boost library path")
        SET(Boost_INCLUDE_SEARCH_PATH "" CACHE STRING "Boost include path")

        IF (NOT Boost_LIBRARY_SEARCH_PATH)
            SET(Boost_LIBRARY_SEARCH_PATH "${Boost_LIBRARY_PATH_GUESSED}")
        ENDIF()

        IF (NOT Boost_INCLUDE_SEARCH_PATH)
            SET(Boost_INCLUDE_SEARCH_PATH "${Boost_INCLUDE_PATH_GUESSED}")
        ENDIF()

        # check include path
        FIND_PATH(Boost_INCLUDE_DIR
            boost/asio/steady_timer.hpp
            PATHS ${Boost_INCLUDE_SEARCH_PATH})

        IF (Boost_LIBRARY_SEARCH_PATH AND EXISTS "${Boost_LIBRARY_SEARCH_PATH}/")
            FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost
                FOUND_VAR Boost_FOUND
                REQUIRED_VARS Boost_LIBRARY_SEARCH_PATH Boost_INCLUDE_DIR)

            # set output vars
            SET(Boost_LIBRARIES ) # uses auto-linking
            SET(Boost_LINK_DIRECTORIES ${Boost_LIBRARY_SEARCH_PATH})
            SET(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIR})

            # add library search directory to global search paths
            LINK_DIRECTORIES(${Boost_LINK_DIRECTORIES})
        ENDIF()
    ENDIF()

    SET(Boost_SOURCE_TYPE "direct")
ENDIF()

IF (Boost_FOUND)
    # default defines
    SET(Boost_DEFINES BOOST_ASIO_HAS_STD_CHRONO BOOST_ASIO_HAS_STD_ARRAY)
    IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        LIST(APPEND Boost_DEFINES BOOST_ASIO_DISABLE_IOCP)
    ENDIF()

    IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        # add defines/libs depending on shared or static library found
        LIST(GET Boost_LIBRARIES 0 Boost_SOME_LIBRARY)
        GET_FILENAME_COMPONENT(Boost_SOME_LIBRARY_SUFFIX ${Boost_SOME_LIBRARY} EXT)

        IF (${Boost_SOME_LIBRARY_SUFFIX} STREQUAL ${CMAKE_SHARED_LIBRARY_SUFFIX})
            SET(Boost_HAS_SHARED_LIBS TRUE)
        ELSE()
            SET(Boost_HAS_SHARED_LIBS FALSE)
        ENDIF()
    ELSE()
        SET(Boost_HAS_SHARED_LIBS FALSE)
    ENDIF()

    IF (Boost_HAS_SHARED_LIBS)
        SET(Boost_LIBRARY_TYPE "shared")
        # dynamic linking defines
        LIST(APPEND Boost_DEFINES BOOST_ALL_DYN_LINK)
    ELSE()
        SET(Boost_LIBRARY_TYPE "static")
        # extra libs required for static linking on linux
        IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            LIST(APPEND Boost_LIBRARIES rt)
        ENDIF()
    ENDIF()

    ACME_INFO("+ Boost (${Boost_SOURCE_TYPE}, ${Boost_LIBRARY_TYPE})")
ELSE()
    ACME_INFO("- Boost")
ENDIF()

