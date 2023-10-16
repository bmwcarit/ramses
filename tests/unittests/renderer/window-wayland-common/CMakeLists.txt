#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    window-wayland-common
    TYPE                    STATIC_LIBRARY
    INCLUDE_PATHS           .
    SRC_FILES               *.h
    DEPENDENCIES            Platform
                            renderer-test-common
                            wayland-test-utils
)
