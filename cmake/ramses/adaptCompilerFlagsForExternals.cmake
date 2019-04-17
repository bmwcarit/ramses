#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# #### these compiler flags are disabled for external projects (= Blacklist) ####
SET(DISABLED_COMPILER_FLAGS
    -Wall
    -Wextra
    -Wold-style-cast
    -Werror
    -Woverloaded-virtual
    -Wcast-align
    -Wshadow
    -Winconsistent-missing-override
    -Wformat-signedness
    -Wmissing-include-dirs
    /W1
    /W2
    /W3
    /W4
    /WX
    /w34100
    /w34388
    /w34242
    /w34265
    /w34355
    /w34062
    --quit_after_warnings
)

# disabled blacklisted compiler flags for this directory
# and subdirectories

REMOVE_FROM_FLAGS("${CMAKE_C_FLAGS}"                "${DISABLED_COMPILER_FLAGS}" CMAKE_C_FLAGS)
REMOVE_FROM_FLAGS("${CMAKE_C_FLAGS_DEBUG}"          "${DISABLED_COMPILER_FLAGS}" CMAKE_C_FLAGS_DEBUG)
REMOVE_FROM_FLAGS("${CMAKE_C_FLAGS_RELEASE}"        "${DISABLED_COMPILER_FLAGS}" CMAKE_C_FLAGS_RELEASE)
REMOVE_FROM_FLAGS("${CMAKE_C_FLAGS_RELWITHDEBINFO}" "${DISABLED_COMPILER_FLAGS}" CMAKE_C_FLAGS_RELWITHDEBINFO)

REMOVE_FROM_FLAGS("${CMAKE_CXX_FLAGS}"                "${DISABLED_COMPILER_FLAGS}" CMAKE_CXX_FLAGS)
REMOVE_FROM_FLAGS("${CMAKE_CXX_FLAGS_DEBUG}"          "${DISABLED_COMPILER_FLAGS}" CMAKE_CXX_FLAGS_DEBUG)
REMOVE_FROM_FLAGS("${CMAKE_CXX_FLAGS_RELEASE}"        "${DISABLED_COMPILER_FLAGS}" CMAKE_CXX_FLAGS_RELEASE)
REMOVE_FROM_FLAGS("${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" "${DISABLED_COMPILER_FLAGS}" CMAKE_CXX_FLAGS_RELWITHDEBINFO)

# define additional compiler flags for external components
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

    SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -Wno-switch-bool")
    SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -Wno-deprecated-declarations")
    SET(ADDITIONAL_CXX_COMPILER_FLAGS "${ADDITIONAL_CXX_COMPILER_FLAGS} -Wno-reorder")

ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

    SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -Wno-unused-parameter")
    SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -Wno-unused-value")
    SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -Wno-logical-op-parentheses")
    SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -Wno-switch")
    SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -Wno-logical-not-parentheses")
    SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -Wno-enum-conversion")
    SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -Wno-ignored-attributes")

    IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.4)
        SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -Wno-switch-bool")
    ENDIF()

    IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.5)
        SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -Wno-inconsistent-missing-override")
    ENDIF()

ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

    SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} /wd4312")

ELSE() # Integrity is last since CMAKE_CXX_COMPILER_ID is not set

    SET(ADDITIONAL_COMPILER_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -w --diag_suppress=1974")

ENDIF()

# append compiler specific flags for external components
IF(ADDITIONAL_COMPILER_FLAGS)
    SET(CMAKE_C_FLAGS_DEBUG             "${CMAKE_C_FLAGS_DEBUG}             ${ADDITIONAL_COMPILER_FLAGS}")
    SET(CMAKE_C_FLAGS_RELEASE           "${CMAKE_C_FLAGS_RELEASE}           ${ADDITIONAL_COMPILER_FLAGS}")
    SET(CMAKE_C_FLAGS_RELWITHDEBINFO    "${CMAKE_C_FLAGS_RELWITHDEBINFO}    ${ADDITIONAL_COMPILER_FLAGS}")
    SET(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG}           ${ADDITIONAL_COMPILER_FLAGS} ${ADDITIONAL_CXX_COMPILER_FLAGS}")
    SET(CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE}         ${ADDITIONAL_COMPILER_FLAGS} ${ADDITIONAL_CXX_COMPILER_FLAGS}")
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}  ${ADDITIONAL_COMPILER_FLAGS} ${ADDITIONAL_CXX_COMPILER_FLAGS}")
ENDIF()
