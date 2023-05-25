#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

expandModuleByPlatform(
    NAME                ramses-example-local-datalink
    TYPE                BINARY
    LINKED_TYPE         DYNAMIC
    ENABLE_INSTALL      ON
    SRC_FILES           src/*
    RESOURCE_FOLDERS    res
)
