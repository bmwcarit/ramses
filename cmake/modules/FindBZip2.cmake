#  -------------------------------------------------------------------------
#  Copyright (C) 2022 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF((CMAKE_SYSTEM_NAME MATCHES "iOS"))

    # If we only give the library name on iOS to the linker instead of a full path, 
    # Xcode can smartly switch between device and simulator version of the library.
    SET(BZip2_LIBRARIES "-lbz2")
    SET(BZip2_FOUND TRUE)

    MARK_AS_ADVANCED(
        BZip2_LIBRARIES
    )

ENDIF()
