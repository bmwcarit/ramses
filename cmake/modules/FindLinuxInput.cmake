#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set( LinuxInput_FOUND FALSE)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_path(LinuxInput_INCLUDE_DIRS linux/input.h
        /usr/include
    )

    if(LinuxInput_INCLUDE_DIRS)
        SET(LinuxInput_FOUND TRUE)
    endif()

    mark_as_advanced(
        LinuxInput_INCLUDE_DIRS
        LinuxInput_FOUND
    )
endif()
