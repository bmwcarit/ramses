#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

FIND_PACKAGE(OpenGL)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    Device_GL
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/Device_GL/*.h
    FILES_SOURCE            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-renderer-lib
                            OpenGL
)

# ACME doesn't pass on definitions, so forced to do it this way
IF(TARGET Device_GL)
    target_compile_definitions(Device_GL PUBLIC ${OpenGL_DEFINITIONS})
ENDIF()
