#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    ramses-example-text-languages
    TYPE                    BINARY
    ENABLE_INSTALL          OFF    # disabled installation to save space in release tarball
    SRC_FILES               src/main.cpp
    RESOURCE_FOLDERS        res
    DEPENDENCIES            ramses-shared-lib-headless
)
