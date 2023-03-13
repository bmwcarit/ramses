#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    Context_WGL
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_PATHS           include
    SRC_FILES               include/Context_WGL/*.h
                            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-renderer-lib
                            Window_Windows
                            OpenGL
)

# This module is tested in the renderer sandwich tests (<ramses>/integration/ramses-renderer-tests)
