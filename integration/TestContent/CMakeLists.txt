#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    TestContent
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           include
    SRC_FILES               include/TestEffects/*.h
                            include/TestScenes/*.h
                            include/TestScenes/TextFake/*.h
                            include/*.h
                            src/*.cpp
                            src/TextFake/*.cpp
    RESOURCE_FOLDERS        res
    DEPENDENCIES            ramses-client
                            ramses-framework
                            ramses-renderer-lib
)
