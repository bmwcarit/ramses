#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

MODULE_WITH_SHARED_LIBRARY(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-example-interleaved-vertex-buffers
    TYPE                    BINARY
    ENABLE_INSTALL          ON

    #==========================================================================
    # files of this module
    #==========================================================================
    SRC_FILES               include/*.h
                            src/*.cpp
    RESOURCE_FOLDERS        res
)
