#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# #### these compiler flags are disabled for external projects (= Blacklist) ####
SET(DISABLED_COMPILER_FLAGS
    --coverage   # coverage broken for some externals
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
