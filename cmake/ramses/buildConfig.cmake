#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


# allow limiting link parallelism
set(ramses-sdk_PARALLEL_LINK_JOBS "" CACHE STRING "Maximum number of parallel link jobs")

if(ramses-sdk_PARALLEL_LINK_JOBS)
  set_property(GLOBAL APPEND PROPERTY JOB_POOLS link_job_pool=${ramses-sdk_PARALLEL_LINK_JOBS})
  set(CMAKE_JOB_POOL_LINK link_job_pool)
endif()

function(createBuildConfig)

  set(TARGET_DIRECTORY ${CMAKE_BINARY_DIR}/BuildConfig)

  set(EXPORTED_VARIABLES
      PROJECT_NAME
      PROJECT_DESCRIPTION
      PROJECT_URL
      PROJECT_VERSION_STRING
      PROJECT_VERSION_MAJOR
      PROJECT_VERSION_MINOR
      PROJECT_VERSION_PATCH
      CMAKE_BUILD_TYPE
      CMAKE_CXX_COMPILER
      CMAKE_CXX_COMPILER_ID
      CMAKE_CXX_FLAGS
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_SYSTEM_NAME
      CMAKE_SYSTEM_VERSION
      CMAKE_TOOLCHAIN_FILE
      CMAKE_VERSION
      GIT_COMMIT_COUNT
      GIT_COMMIT_HASH
      TARGET_OS
      BUILD_ENV_VERSION_INFO_FULL
  )

  set(EXPORTED_INT_VARIABLES
      PROJECT_VERSION_MAJOR
      PROJECT_VERSION_MINOR
      PROJECT_VERSION_PATCH)

  if (NOT DEFINED GIT_COMMIT_COUNT OR NOT DEFINED GIT_COMMIT_HASH)
      message(FATAL_ERROR "GIT_COMMIT_COUNT and GIT_COMMIT_HASH must be set")
  endif()

  string(REPLACE "-" "_" PROJECT_NAME_NO_DASH ${PROJECT_NAME})
  string(TOUPPER ${PROJECT_NAME_NO_DASH} PROJECT_NAME_CAPITAL)
  string(TOLOWER ${PROJECT_NAME_NO_DASH} PROJECT_NAME_LOWERCASE)

  set(EXPORTED_RAMSES_CONFIG_SYMBOLS "")

  foreach(VAR ${EXPORTED_VARIABLES})
      if(NOT DEFINED ${VAR})
          set(${VAR} "(unknown)")
      endif()

      # escape '\'
      string(REPLACE "\\" "\\\\" TMP "${${VAR}}")
      string(REPLACE "\"" "\\\"" TMP "${TMP}")
      set(ESCAPED_${VAR} "${TMP}")

      set(EXPORTED_RAMSES_CONFIG_SYMBOLS "${EXPORTED_RAMSES_CONFIG_SYMBOLS}\nconst char* const ${PROJECT_NAME_CAPITAL}_${VAR} = \"@ESCAPED_${VAR}@\";")
  endforeach()

  foreach(VAR ${EXPORTED_INT_VARIABLES})
      if(DEFINED ${VAR})
          set(EXPORTED_RAMSES_CONFIG_SYMBOLS "${EXPORTED_RAMSES_CONFIG_SYMBOLS}\nconst int ${PROJECT_NAME_CAPITAL}_${VAR}_INT = @${VAR}@;")
      endif()
      endforeach()

  configure_file(
      ${PROJECT_SOURCE_DIR}/cmake/templates/build-config.h.in
      ${TARGET_DIRECTORY}/${PROJECT_NAME}-build-config.h.in
  )

  configure_file(
      ${TARGET_DIRECTORY}/${PROJECT_NAME}-build-config.h.in
      ${TARGET_DIRECTORY}/${PROJECT_NAME}-build-config.h
  )

  message(STATUS "G ${PROJECT_NAME}BuildConfig.h")

  # TODO Violin replace this with a target, using global includes like this is dangerous
  include_directories(${TARGET_DIRECTORY})
endfunction()
