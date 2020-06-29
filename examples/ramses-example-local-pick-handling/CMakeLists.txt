#  -------------------------------------------------------------------------
#  Copyright (C) 2019 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

RENDERER_MODULE_PER_CONFIG_DYNAMIC(ramses-example-local-pick-handling
    TYPE                    BINARY
    ENABLE_INSTALL          ON
    FILES_SOURCE            src/*
    FILES_RESOURCE          res/*
    DEPENDENCIES            ramses-framework
                            ramses-client
)
