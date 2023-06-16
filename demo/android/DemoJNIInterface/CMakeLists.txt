#  -------------------------------------------------------------------------
#  Copyright (C) 2019 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    DemoRamsesJNIInterface
    ENABLE_INSTALL          ON
    TYPE                    SHARED_LIBRARY
    INCLUDE_PATHS           include
    SRC_FILES               include/*.h
                            src/*.cpp

    DEPENDENCIES            AndroidSDK
                            ramses-shared-lib
)

