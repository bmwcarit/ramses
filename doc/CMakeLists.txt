#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if(ramses-sdk_FORCE_BUILD_DOCS)
    set(PACKAGE_FIND_MODE REQUIRED)
    set(DOCS_ERROR_TYPE FATAL_ERROR)
else()
    set(PACKAGE_FIND_MODE QUIET)
    set(DOCS_ERROR_TYPE STATUS)
endif()

# error if doxygen not found
find_package(Doxygen ${PACKAGE_FIND_MODE})

if (NOT DOXYGEN_FOUND)
    message(${DOCS_ERROR_TYPE} "Doxygen was not found on this system. Documentation generation was disabled. \
        Please make the doxygen and the dot executables available on your system!")
    return()
endif()

find_package(Sphinx ${PACKAGE_FIND_MODE})
if (NOT SPHINX_FOUND)
    message(${DOCS_ERROR_TYPE} "Sphinx was not found on this system. Documentation generation was disabled. \
        Please install python3 and do a pip3 install -r doc/sphinx/requirements.txt!")
    return()
endif()

# only used to set as dependency for docs build (so that docs are rebuilt in developer mode
# when an API file is changed)
file(GLOB_RECURSE
    API_FILES
    ${PROJECT_SOURCE_DIR}/include/ramses/*.h
)

string(REPLACE ";" " " DOXYGEN_INPUT  "${API_FILES}")

# TODO check if the INPUT value is the right one
# Specify source content (what to build docs for?) and target folder (where to put the result)
set(DOXYGEN_INPUT       "${DOXYGEN_INPUT}")
set(DOXYGEN_TARGET_DIR  "${CMAKE_BINARY_DIR}/doxygen")

# configure doxygen config file
set(DOXYGEN_CONFIG_FILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in")
set(DOXYGEN_CONFIG_FILE "${CMAKE_BINARY_DIR}/Doxyfile")
configure_file(${DOXYGEN_CONFIG_FILE_IN} ${DOXYGEN_CONFIG_FILE} @ONLY)

# create error checker script
file(WRITE "${CMAKE_BINARY_DIR}/check-doxygen-errors.cmake"
    "file(READ \"DoxygenWarningLog.txt\" INP)\n"
    "if (NOT \"\${INP}\" STREQUAL \"\")\n"
    "  message(FATAL_ERROR \"Encountered doxygen warnings:\\n\${INP}\")\n"
    "endif()\n")

set(DOXYGEN_STAMP_FILE ${DOXYGEN_TARGET_DIR}/doxygen.stamp)

# run doxygen
add_custom_command(OUTPUT ${DOXYGEN_STAMP_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E touch ${DOXYGEN_STAMP_FILE}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_FILE}
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/check-doxygen-errors.cmake"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating doxygen files in: ${DOXYGEN_TARGET_DIR}" VERBATIM
    DEPENDS ${API_FILES} ${DOXYGEN_CONFIG_FILE_IN})

# add target to create doxygen xml
add_custom_target(ramses-doxygen
    COMMENT "Generated doxygen files in: ${DOXYGEN_TARGET_DIR}" VERBATIM
    DEPENDS ${DOXYGEN_STAMP_FILE}
    SOURCES ${API_FILES} ${DOXYGEN_CONFIG_FILE_IN}
)

folderizeTarget(ramses-doxygen)

set(SPHINX_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/sphinx)
set(SPHINX_TARGET_DIR ${CMAKE_BINARY_DIR}/sphinx)
set(SPHINX_IDX_FILE ${SPHINX_TARGET_DIR}/index.html)

file(GLOB
    SPHINX_INPUT_LIST_PAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/sphinx/*.rst
    ${CMAKE_CURRENT_SOURCE_DIR}/sphinx/*.md
    )

file(GLOB
    SPHINX_INPUT_LIST_CLASSES
    ${CMAKE_CURRENT_SOURCE_DIR}/sphinx/classes/*)

file(GLOB
    SPHINX_INPUT_LIST_EXAMPLES
    ${CMAKE_CURRENT_SOURCE_DIR}/sphinx/examples/*)

set(SPHINX_BUILDER "html") # Use 'linkcheck' to check for validity of links

# Only regenerate Sphinx files when needed (either doxygen target or sphinx input files changed)
add_custom_command(OUTPUT ${SPHINX_IDX_FILE}
                   COMMAND
                    ${SPHINX_EXECUTABLE}
                        -b ${SPHINX_BUILDER}
                        -Dbreathe_projects.ramses=${DOXYGEN_TARGET_DIR}/xml
                        -Drelease=${RAMSES_VERSION}
                        -W --keep-going
                        ${SPHINX_SRC_FILES} ${SPHINX_TARGET_DIR}
                   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                   DEPENDS
                        ${SPHINX_INPUT_LIST_PAGES}
                        ${SPHINX_INPUT_LIST_CLASSES}
                        ${SPHINX_INPUT_LIST_EXAMPLES}
                        ${DOXYGEN_STAMP_FILE}
                        ${PROJECT_SOURCE_DIR}/README.md
                        ${PROJECT_SOURCE_DIR}/CONTRIBUTING.rst
                        ${PROJECT_SOURCE_DIR}/CHANGELOG.md
                   MAIN_DEPENDENCY ${SPHINX_SRC_FILES}/conf.py
                   COMMENT "Generating Sphinx docs")

# add target to create sphinx html
add_custom_target(ramses-sphinx ALL
    DEPENDS ${SPHINX_IDX_FILE}
    SOURCES
        ${SPHINX_INPUT_LIST_PAGES}
        ${SPHINX_INPUT_LIST_CLASSES}
        ${SPHINX_INPUT_LIST_EXAMPLES}
        ${SPHINX_SRC_FILES}/conf.py
)

source_group("pages" FILES ${SPHINX_INPUT_LIST_PAGES})
source_group("classes" FILES ${SPHINX_INPUT_LIST_CLASSES})
source_group("examples" FILES ${SPHINX_INPUT_LIST_EXAMPLES})

folderizeTarget(ramses-sphinx)
