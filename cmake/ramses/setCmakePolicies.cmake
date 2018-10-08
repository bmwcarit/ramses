#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# Only interpret if() arguments as variables or keywords when unquoted.
IF (POLICY CMP0054)
    CMAKE_POLICY(SET CMP0054 NEW)
ENDIF()

# Support new TEST if() operator
IF (POLICY CMP0064)
    CMAKE_POLICY(SET CMP0064 NEW)
ENDIF()
