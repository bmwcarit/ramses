#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF((CMAKE_SYSTEM_NAME MATCHES "Darwin") OR (CMAKE_SYSTEM_NAME MATCHES "iOS"))

    FIND_LIBRARY(QUARTZCORE_LIBRARY QuartzCore)
    FIND_PATH(QuartzCore_INCLUDE_DIRS CAMetalLayer.h)

    SET(QuartzCore_LIBRARIES "${QUARTZCORE_LIBRARY}" )

    IF(QuartzCore_LIBRARIES AND QuartzCore_INCLUDE_DIRS)
        SET(QuartzCore_FOUND TRUE)
    ENDIF()

    MARK_AS_ADVANCED(
        QuartzCore_FOUND
        QuartzCore_LIBRARIES
        QuartzCore_INCLUDE_DIRS
    )

ENDIF()
