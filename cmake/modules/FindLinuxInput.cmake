#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

SET( LinuxInput_FOUND FALSE)

IF (TARGET_OS MATCHES "Linux")

    FIND_PATH(LinuxInput_INCLUDE_DIRS linux/input.h
        /usr/include
    )

    IF(LinuxInput_INCLUDE_DIRS)
        SET(LinuxInput_FOUND TRUE)
    ENDIF()

    MARK_AS_ADVANCED(
        LinuxInput_INCLUDE_DIRS
        LinuxInput_FOUND
    )

ENDIF()
