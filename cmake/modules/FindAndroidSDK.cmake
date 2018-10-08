#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

SET(AndroidSDK_FOUND FALSE)

FIND_LIBRARY(ANDROID_LIB android)

IF(ANDROID_LIB)
    SET(AndroidSDK_LIBRARIES ${ANDROID_LIB} log)

    MARK_AS_ADVANCED(
        AndroidSDK_LIBRARIES
    )

    SET(AndroidSDK_FOUND TRUE)
ENDIF()
