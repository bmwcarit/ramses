#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF(CMAKE_SYSTEM_NAME MATCHES "Darwin")

    FIND_PATH(Cocoa_INCLUDE_DIRS Cocoa.h)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)

    SET(Cocoa_LIBRARIES "${COCOA_LIBRARY}" )

    IF(Cocoa_LIBRARIES AND Cocoa_INCLUDE_DIRS)
        SET(Cocoa_FOUND TRUE)
    ENDIF()

    MARK_AS_ADVANCED(
        Cocoa_INCLUDE_DIRS
        Cocoa_LIBRARIES
    )

ENDIF()
