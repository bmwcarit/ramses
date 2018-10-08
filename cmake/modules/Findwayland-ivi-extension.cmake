#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

INCLUDE(FindPackageHandleStandardArgs)

FIND_PATH(wayland-ivi-extension_INCLUDE_1 ivi-application-server-protocol.h)
FIND_PATH(wayland-ivi-extension_INCLUDE_2 ivi-application-client-protocol.h)
FIND_PATH(wayland-ivi-extension_INCLUDE_3 ivi-controller-client-protocol.h)

FIND_LIBRARY(wayland-ivi-extension_LIB_1 NAMES ivi-application)
FIND_LIBRARY(wayland-ivi-extension_LIB_2 NAMES ivi-controller)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(wayland-ivi-extension
    FOUND_VAR wayland-ivi-extension_FOUND
    REQUIRED_VARS   wayland-ivi-extension_INCLUDE_1
                    wayland-ivi-extension_INCLUDE_2
                    wayland-ivi-extension_INCLUDE_3
                    wayland-ivi-extension_LIB_1
                    wayland-ivi-extension_LIB_2)

SET(wayland-ivi-extension_INCLUDE_DIRS
    ${wayland-ivi-extension_INCLUDE_1}
    ${wayland-ivi-extension_INCLUDE_2}
    ${wayland-ivi-extension_INCLUDE_3})
SET(wayland-ivi-extension_LIBRARIES
    ${wayland-ivi-extension_LIB_1}
    ${wayland-ivi-extension_LIB_2})

MARK_AS_ADVANCED(
    wayland-ivi-extension_INCLUDE_DIRS
    wayland-ivi-extension_LIBRARIES
    wayland-ivi-extension_FOUND
)

