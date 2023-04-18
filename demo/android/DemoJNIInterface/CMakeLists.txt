#  -------------------------------------------------------------------------
#  Copyright (C) 2019 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

MODULE_WITH_SHARED_LIBRARY(
    NAME                    DemoRamsesJNIInterface
    TYPE                    SHARED_LIBRARY
    ENABLE_INSTALL          ON
    INCLUDE_PATHS           include
    SRC_FILES               include/*.h
                            src/*.cpp

    DEPENDENCIES            AndroidSDK
)

