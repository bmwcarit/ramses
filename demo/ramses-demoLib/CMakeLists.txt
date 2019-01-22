#  -------------------------------------------------------------------------
#  Copyright (C) 2018 Mentor Graphics Development GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

ACME_MODULE(
    NAME                    ramses-demoLib
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ON

    FILES_SOURCE            src/*.cpp
    FILES_RESOURCE          res/*

    DEPENDENCIES            ramses-client
                            DisplayManager
)