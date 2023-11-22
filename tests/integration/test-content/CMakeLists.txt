#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    test-content
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           .
    SRC_FILES               TestScenes/*.h
                            *.h
                            *.cpp
    RESOURCE_FOLDERS        res
    DEPENDENCIES            ramses-client
                            ramses-framework
                            ramses-renderer-internal
)
