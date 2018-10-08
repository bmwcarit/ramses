#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

MODULE_WITH_SHARED_LIBRARY(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-text-demo-shadow
    TYPE                    BINARY
    ENABLE_INSTALL          ON

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    src/*.h
    FILES_SOURCE            src/*.cpp
    FILES_RESOURCE          res/*
)
