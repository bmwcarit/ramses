#  -------------------------------------------------------------------------
#  Copyright (C) 2021 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                ramses-example-local-dma-offscreenbuffer
    TYPE                BINARY
    ENABLE_INSTALL      ON
    SRC_FILES           src/*
    RESOURCE_FOLDERS    res
    DEPENDENCIES        ramses-shared-lib
                        gbm
                        libdrm
)
