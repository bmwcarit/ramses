#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

cmake_policy(SET CMP0048 NEW)

# Only interpret if() arguments as variables or keywords when unquoted.
if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

# Support new TEST if() operator
if (POLICY CMP0064)
    cmake_policy(SET CMP0064 NEW)
endif()

# Convert relative paths to absolute paths in target_sources
if (POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
endif()

if (POLICY CMP0022)
    cmake_policy(SET CMP0022 NEW)
endif()
