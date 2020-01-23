#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# test system for doxygen
IF(NOT DOXYGEN_FOUND)
    INCLUDE(FindDoxygen)
    IF(NOT DOXYGEN_FOUND)
        ACME_INFO("Doxygen was not found on this system. Documentation generation was disabled.")
        RETURN()
    ENDIF()
ENDIF()


FUNCTION(GENERATE_DOCUMENT documentname documentversion description inputfilelist imagepathlist targetname)

    # prepare internal data
    SET(INPUT_STRING_FOR_DOXYGEN "")
    STRING(REPLACE " " "-" documentname-no-spaces ${documentname})
    FILE(GLOB DOC_FILE_LIST ${inputfilelist})
    FOREACH(DOC_FILE ${DOC_FILE_LIST})
        SET(INPUT_STRING_FOR_DOXYGEN "${INPUT_STRING_FOR_DOXYGEN}\nINPUT += ${DOC_FILE}")
    ENDFOREACH()
    FOREACH(IMAGE_PATH ${imagepathlist})
        SET(INPUT_STRING_FOR_DOXYGEN "${INPUT_STRING_FOR_DOXYGEN}\nIMAGE_PATH += ${IMAGE_PATH}")
    ENDFOREACH()

    # doxygen config
    SET(DOXYGEN_TEMPLATE    "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in")
    SET(DOXYGEN_CONFIG_FILE "${CMAKE_BINARY_DIR}/Doxyfile.${documentname-no-spaces}")
    SET(DOXYGEN_TARGET_DIR  "${CMAKE_BINARY_DIR}/doc/${documentname-no-spaces}")

    # variables for doxygen template
    SET(RAMSES_DOCUMENT_NAME        "${documentname}")
    SET(RAMSES_DOCUMENT_DESCRIPTION "${description}")
    SET(RAMSES_DOXYGEN_TARGET_DIR   "${DOXYGEN_TARGET_DIR}")
    SET(RAMSES_DOXYGEN_INPUT_FILES  "${INPUT_STRING_FOR_DOXYGEN}")
    SET(RAMSES_VERSION              "${documentversion}")
    SET(RAMSES_EXAMPLE_PATH         "${ramses-sdk_SOURCE_DIR}/examples")
    SET(RAMSES_LOGO_SMALL           "${ramses-sdk_SOURCE_DIR}/doc/doxygen/ramses_small.png")

    # generate doxygen config file
    CONFIGURE_FILE("${DOXYGEN_TEMPLATE}" ${DOXYGEN_CONFIG_FILE} @ONLY)

    file(WRITE "${CMAKE_BINARY_DIR}/check-doxygen-errors.cmake"
        "file(READ \"DoxygenWarningLog.txt\" INP)\n"
        "if (NOT \"\${INP}\" STREQUAL \"\")\n"
        "  message(FATAL_ERROR \"Encountered doxygen warnings:\\n\${INP}\")\n"
        "endif()\n")

    # run doxygen to generate latex output
    ADD_CUSTOM_COMMAND(OUTPUT ${documentname-no-spaces}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_FILE}
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/check-doxygen-errors.cmake"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating documentation in: ${DOXYGEN_TARGET_DIR}" VERBATIM
        DEPENDS ${DOC_FILE_LIST}
    )

    # add target to create document
    ADD_CUSTOM_TARGET(${targetname}
        COMMENT "Generated documentation in: ${DOXYGEN_TARGET_DIR}" VERBATIM
        DEPENDS ${documentname-no-spaces}
        SOURCES ${DOC_FILE_LIST}
    )

    ACME_INFO("+ ${targetname}")

    SET_PROPERTY(TARGET ${targetname} PROPERTY FOLDER "doc")

ENDFUNCTION()


# all in one docs

file(GLOB_RECURSE
    RAMSES_DOX_INPUT
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ../framework/ramses-framework-api/include/*.h
    ../client/ramses-client/ramses-client-api/include/*.h
    ../client/ramses-client/ramses-text-api/include/*.h
    ../renderer/RendererLib/ramses-renderer-api/include/*.h
    ../examples/ramses-example-*/*.h
    ../examples/ramses-example-*/*.cpp
    *.dox)

set(RAMSES_DOX_IMAGE_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/developer/images
    ${CMAKE_CURRENT_SOURCE_DIR}/dlt-logging/images
    ${CMAKE_CURRENT_SOURCE_DIR}/general/images
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/images)

 GENERATE_DOCUMENT(
     "RAMSES Documentation"
     "${RAMSES_VERSION}"
     "Information for RAMSES users and developers"
     "${RAMSES_DOX_INPUT}"
     "${RAMSES_DOX_IMAGE_PATHS}"
     "doc")
