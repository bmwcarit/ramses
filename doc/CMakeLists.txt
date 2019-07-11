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
    SET(DOXYGEN_FOUND ${DOXYGEN_FOUND} CACHE INTERNAL "")
    SET(DOXYGEN_DOT_FOUND ${DOXYGEN_DOT_FOUND} CACHE INTERNAL "")
    IF(NOT DOXYGEN_FOUND)
        ACME_INFO("Doxygen was not found on this system. Documentation generation was disabled.")
        RETURN()
    ENDIF()
ENDIF()


FUNCTION(GENERATE_DOCUMENT documentname documentversion description inputfilelist imagepathlist)

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

    # run doxygen to generate latex output
    ADD_CUSTOM_COMMAND(OUTPUT ${documentname-no-spaces}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_FILE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating documentation in: ${DOXYGEN_TARGET_DIR}" VERBATIM
        DEPENDS ${DOC_FILE_LIST}
    )

    # add target to create document
    ADD_CUSTOM_TARGET(doc-${documentname-no-spaces}
        COMMENT "Generated documentation in: ${DOXYGEN_TARGET_DIR}" VERBATIM
        DEPENDS ${documentname-no-spaces}
        SOURCES ${DOC_FILE_LIST}
    )

    ACME_INFO("+ doc-${documentname-no-spaces}")

    SET_PROPERTY(TARGET doc-${documentname-no-spaces} PROPERTY FOLDER "doc")

ENDFUNCTION()


# define the generated documents

SET(DEVELOPER_DOCUMENTS
    ${CMAKE_CURRENT_SOURCE_DIR}/developer/*.dox
)

GENERATE_DOCUMENT(
    "RAMSES Developer Information"
    "${RAMSES_VERSION}"
    "Developer information for people working with the RAMSES code base"
    "${DEVELOPER_DOCUMENTS}"
    "${CMAKE_CURRENT_SOURCE_DIR}/developer/images;"
)

# get all header from client, framework & renderer api
file(GLOB_RECURSE
    RAMSES_API_ALL_HEADERS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ../framework/ramses-framework-api/include/ramses-framework-api/*.h
    ../client/ramses-client/ramses-client-api/include/ramses-client-api/*.h
    ../client/ramses-text/ramses-text-api/include/ramses-text-api/*.h
    ../client/ramses-client/ramses-client-api/include/*.h
    ../renderer/RendererLib/ramses-renderer-api/include/ramses-renderer-api/*.h)

FILE(GLOB RAMSES_API_DOCUMENTS_ALL
    general/*.dox
    ${RAMSES_API_ALL_HEADERS}
    ../examples/ramses-example-*/*/*.h
    ../examples/ramses-example-*/*/*.cpp
    )

GENERATE_DOCUMENT(
    "RAMSES General"
    "${RAMSES_VERSION}"
    "RAMSES General Overview"
    "${RAMSES_API_DOCUMENTS_ALL}"
    "${CMAKE_CURRENT_SOURCE_DIR}/general/images"
)

SET(DLT_DOCUMENTS
    ${CMAKE_CURRENT_SOURCE_DIR}/dlt-logging/*.dox
)

GENERATE_DOCUMENT(
    "RAMSES DLT Logging"
    "${RAMSES_VERSION}"
    "Architecture of RAMSES Framework"
    "${DLT_DOCUMENTS}"
    "${CMAKE_CURRENT_SOURCE_DIR}/dlt-logging/images"
)

SET(EMBEDDED_COMPOSITING_DOCUMENTS
    ${CMAKE_CURRENT_SOURCE_DIR}/embedded-compositing/*.dox
    ${RAMSES_API_ALL_HEADERS}
    ../examples/ramses-example-basic-compositing/*/*.cpp
)

GENERATE_DOCUMENT(
    "RAMSES Embedded Compositing Support"
    "${RAMSES_VERSION}"
    "Embedded Compositing Support"
    "${EMBEDDED_COMPOSITING_DOCUMENTS}"
    ""
)

SET(RAMSES_TOOLS_DOCUMENTS
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/*.dox
    ${RAMSES_API_ALL_HEADERS}
)

GENERATE_DOCUMENT(
    "RAMSES Tools"
    "${RAMSES_VERSION}"
    "Ramses Tools Support"
    "${RAMSES_TOOLS_DOCUMENTS}"
    ""
)
