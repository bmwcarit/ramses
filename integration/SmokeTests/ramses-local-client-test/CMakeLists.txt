#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

expandModuleByPlatform(
    NAME                ramses-local-client-test
    TYPE                BINARY
    LINKED_TYPE         STATIC
    ENABLE_INSTALL      ON
    SRC_FILES           src/*
    RESOURCE_FOLDERS    ../../TestContent/res
    DEPENDENCIES        TestContent
                        ramses-renderer-lib
                        ramses-cli
)
