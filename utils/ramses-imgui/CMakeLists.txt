#  -------------------------------------------------------------------------
#  Copyright (C) 2022 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-imgui
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_PATHS           include
    SRC_FILES               include/*
                            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-client
                            ramses-renderer-api
                            imgui
)
