#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    ramses-example-text-basic
    TYPE                    BINARY
    ENABLE_INSTALL          ON
    SRC_FILES               src/*.cpp
                            src/*.h
    RESOURCE_FOLDERS        res
    DEPENDENCIES            ramses-shared-lib-headless
)
