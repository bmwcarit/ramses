#  -------------------------------------------------------------------------
#  Copyright (C) 2022 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF((CMAKE_SYSTEM_NAME MATCHES "Darwin") OR (CMAKE_SYSTEM_NAME MATCHES "iOS"))

    FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
    FIND_PATH(CoreFoundation_INCLUDE_DIRS CFRunLoop.h)

    SET(CoreFoundation_LIBRARIES "${COREFOUNDATION_LIBRARY}" )

    IF(CoreFoundation_LIBRARIES AND CoreFoundation_INCLUDE_DIRS)
        SET(CoreFoundation_FOUND TRUE)
    ENDIF()

    MARK_AS_ADVANCED(
        CoreFoundation_LIBRARIES
        CoreFoundation_INCLUDE_DIRS
    )

ENDIF()
