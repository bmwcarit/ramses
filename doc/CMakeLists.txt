#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# skip if doxygen or dot not found
find_package(Doxygen QUIET)
if (NOT TARGET Doxygen::doxygen OR NOT TARGET Doxygen::dot)
    message(STATUS "Doxygen and/or dot were not found on this system. Documentation generation was disabled.")
    return()
endif()

# input files
file(GLOB_RECURSE
    DOXYGEN_INPUT_LIST
    ${PROJECT_SOURCE_DIR}/*/ramses-*-api/include/*.h
    ${PROJECT_SOURCE_DIR}/*/*/ramses-*-api/include/*.h
    ${PROJECT_SOURCE_DIR}/*/*/*/ramses-*-api/include/*.h
    ../examples/ramses-example-*/*.h
    ../examples/ramses-example-*/*.cpp
    *.dox)

# image and dot dirs
file(GLOB_RECURSE
    DOXYGEN_IMAGE_PATH_LIST
    LIST_DIRECTORIES TRUE
    ${CMAKE_CURRENT_SOURCE_DIR}/*/images)

# variable setup
set(DOXYGEN_LOGO           "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/ramses_small.png")
set(DOXYGEN_TARGET_DIR     "${CMAKE_BINARY_DIR}/doc")
set(DOXYGEN_EXAMPLE_PATH   "${PROJECT_SOURCE_DIR}/examples")
set(DOXYGEN_LAYOUT         "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/DoxygenLayout.xml")

string(REPLACE ";" " " DOXYGEN_INPUT  "${DOXYGEN_INPUT_LIST}")
string(REPLACE ";" " " DOXYGEN_IMAGE_PATH  "${DOXYGEN_IMAGE_PATH_LIST}")

# generate doxygen config file
set(DOXYGEN_CONFIG_FILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in")
set(DOXYGEN_CONFIG_FILE "${CMAKE_BINARY_DIR}/Doxyfile")
configure_file(${DOXYGEN_CONFIG_FILE_IN} ${DOXYGEN_CONFIG_FILE} @ONLY)

# create error checker script
file(WRITE "${CMAKE_BINARY_DIR}/check-doxygen-errors.cmake"
    "file(READ \"DoxygenWarningLog.txt\" INP)\n"
    "if (NOT \"\${INP}\" STREQUAL \"\")\n"
    "  message(FATAL_ERROR \"Encountered doxygen warnings:\\n\${INP}\")\n"
    "endif()\n")

# run doxygen
add_custom_command(OUTPUT ${DOXYGEN_TARGET_DIR}/doxygen.stamp
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E touch ${DOXYGEN_TARGET_DIR}/doxygen.stamp
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_FILE}
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/check-doxygen-errors.cmake"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating documentation in: ${DOXYGEN_TARGET_DIR}" VERBATIM
    DEPENDS ${DOXYGEN_INPUT_LIST} ${DOXYGEN_CONFIG_FILE_IN})

# add target to create document
add_custom_target(doc
    COMMENT "Generated documentation in: ${DOXYGEN_TARGET_DIR}" VERBATIM
    DEPENDS ${DOXYGEN_TARGET_DIR}/doxygen.stamp)
ACME_FOLDERIZE_TARGET(doc)

# attach sources for VS
file(GLOB_RECURSE
    DOC_SOURCES
    LIST_DIRECTORIES FALSE
    *.*)
set_property(TARGET doc APPEND PROPERTY SOURCES ${DOC_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${DOC_SOURCES})

message(STATUS "+ doc")
