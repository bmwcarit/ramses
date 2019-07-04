#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

RENDERER_MODULE_PER_CONFIG_STATIC(ramses-local-client-test
    TYPE           BINARY
    ENABLE_INSTALL ON
    FILES_SOURCE   src/*
    FILES_RESOURCE ../../TestContent/res/*
    DEPENDENCIES   TestContent
                   ramses-renderer-lib
)
