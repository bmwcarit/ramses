############################################################################
#
# Copyright (C) 2014 BMW Car IT GmbH
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
############################################################################

FUNCTION(ADD_TESTCOVERAGE_TARGETS name dependency)
    SET(WORKDIR "${CMAKE_BINARY_DIR}/CodeCoverage")

    ADD_CUSTOM_COMMAND(OUTPUT CreateInitialCoverage_${name}
        COMMAND lcov --zerocounters --directory ${CMAKE_BINARY_DIR} --output-file ${WORKDIR}/everything_initial_${name}.info --quiet
        COMMAND lcov --capture --initial --directory ${CMAKE_BINARY_DIR} --output-file ${WORKDIR}/everything_initial_${name}.info --quiet
        DEPENDS ${dependency}
        WORKING_DIRECTORY "${WORKDIR}"
        COMMENT "Codecoverage (${name}): Initialize everything to 0"
    )

    ACME_INFO(ACMEPLUGIN_LINUX_TESTCOVERAGE_TESTCOMMAND_${name}="${ACMEPLUGIN_LINUX_TESTCOVERAGE_TESTCOMMAND_${name}}")
    ADD_CUSTOM_COMMAND(OUTPUT RUNTESTS_${name}
        COMMAND ${ACMEPLUGIN_LINUX_TESTCOVERAGE_TESTCOMMAND_${name}}
        DEPENDS CreateInitialCoverage_${name}
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Codecoverage (${name}): Run tests to gather coverage information"
    )

    ACME_INFO(ACMEPLUGIN_LINUX_TESTCOVERAGE_EXCLUDE_${name}="${ACMEPLUGIN_LINUX_TESTCOVERAGE_EXCLUDE_${name}}")
    ADD_CUSTOM_COMMAND(OUTPUT CreateTestCoverage_Data_${name}
        COMMAND lcov --capture --directory ${CMAKE_BINARY_DIR} --kernel-directory ${CMAKE_BINARY_DIR} --output-file ${WORKDIR}/everything_afterRun_${name}.info
                 --base-directory ${CMAKE_SOURCE_DIR} --no-external --quiet
        COMMAND lcov --add-tracefile ${WORKDIR}/everything_initial_${name}.info --add-tracefile ${WORKDIR}/everything_afterRun_${name}.info --output-file ${WORKDIR}/combined_${name}.info  --quiet
        COMMAND lcov
         --remove ${WORKDIR}/combined_${name}.info /usr/*.* ${ACMEPLUGIN_LINUX_TESTCOVERAGE_EXCLUDE_${name}}
         --output-file ${WORKDIR}/combinedFiltered_${name}.info --quiet --base-directory ${CMAKE_SOURCE_DIR} --no-external
        DEPENDS RUNTESTS_${name}
        COMMENT "Codecoverage (${name}): Crunch coverage data"
        WORKING_DIRECTORY "${WORKDIR}"
    )

    ADD_CUSTOM_COMMAND(OUTPUT CreateTestCoverage_${name}
        COMMAND genhtml -s -l combinedFiltered_${name}.info --quiet --output-directory ${WORKDIR}/html_${name}
        DEPENDS CreateTestCoverage_Data_${name}
        COMMENT "Codecoverage (${name}): Generate html"
        WORKING_DIRECTORY "${WORKDIR}"
    )

    IF (NOT TARGET CreateTestCoverage${name})
        ADD_CUSTOM_TARGET(CreateTestCoverage${name}
            COMMENT "Creating TestCoverage"
            DEPENDS CreateTestCoverage_${name}
        )
    ENDIF()
ENDFUNCTION()

IF("${TARGET_OS}" STREQUAL "Linux")
    IF(NOT LCOV_FOUND)
        FIND_PROGRAM(LCOV_PATH lcov)
    ENDIF()
ENDIF()

ADD_CUSTOM_COMMAND(OUTPUT CreateCoverageDirectory
    COMMAND rm
        ARGS -rf CodeCoverage
    COMMAND mkdir
        ARGS "CodeCoverage"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Creating CodeCoverage directory"
)

SET(DEPENDENCY CreateCoverageDirectory)
FOREACH(CURRENT_NAME ${ACMEPLUGIN_LINUX_TESTCOVERAGE_NAMES})
   IF(ACMEPLUGIN_LINUX_TESTCOVERAGE_TESTCOMMAND_${CURRENT_NAME})
      IF(ACMEPLUGIN_LINUX_TESTCOVERAGE_EXCLUDE_${CURRENT_NAME})
        ACME_INFO("Adding test coverage run for ${CURRENT_NAME}")
        ADD_TESTCOVERAGE_TARGETS(${CURRENT_NAME} ${DEPENDENCY})
        SET(DEPENDENCY CreateTestCoverage${CURRENT_NAME})
      ELSE()
        ACME_ERROR("Must provide ACMEPLUGIN_LINUX_TESTCOVERAGE_EXCLUDE_${CURRENT_NAME}")
      ENDIF()
   ELSE()
       ACME_ERROR("Must provide ACMEPLUGIN_LINUX_TESTCOVERAGE_TESTCOMMAND_${CURRENT_NAME}")
  ENDIF()
ENDFOREACH()

IF (NOT TARGET CreateTestCoverage)
    ADD_CUSTOM_TARGET(CreateTestCoverage
        COMMENT "Creating TestCoverage"
        DEPENDS ${DEPENDENCY}
    )
ENDIF()
