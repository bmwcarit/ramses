#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET(WindowsSDK_FOUND TRUE)
ELSE()
    SET(WindowsSDK_FOUND FALSE)
ENDIF()
