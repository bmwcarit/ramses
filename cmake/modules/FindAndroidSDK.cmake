#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if (AndroidSDK_FOUND)
    return()
endif()

set(AndroidSDK_FOUND FALSE)

find_library(ANDROID_LIB android)
if(ANDROID_LIB)
    set(AndroidSDK_LIBRARIES ${ANDROID_LIB} log)

    mark_as_advanced(
        AndroidSDK_LIBRARIES
    )

    set(AndroidSDK_FOUND TRUE)
endif()
