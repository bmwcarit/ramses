#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF (ramses-sdk_ENABLE_DLT)
    find_package(PkgConfig QUIET)
    IF(PKG_CONFIG_FOUND)

        pkg_check_modules(automotive-dlt automotive-dlt QUIET)
        IF(${automotive-dlt_FOUND})
            MARK_AS_ADVANCED(
                automotive-dlt_FOUND
                automotive-dlt_INCLUDE_DIRS
                automotive-dlt_LIBRARIES
                )
            SET(automotive-dlt_HAS_FILETRANSFER TRUE CACHE INTERNAL "")
        ENDIF()
    ENDIF()
ENDIF()
