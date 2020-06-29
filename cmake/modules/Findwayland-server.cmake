#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# use package config to resolve dependencies,
# this is default for wayland projects.

find_package(PkgConfig QUIET)

IF(PKG_CONFIG_FOUND)
    pkg_check_modules(wayland-server wayland-server>=1.13)
ENDIF()
