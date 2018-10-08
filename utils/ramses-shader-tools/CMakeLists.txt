#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-shader-tools-lib
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_PRIVATE_HEADER    include/*
    FILES_SOURCE            src/*.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-client
                            ramses-utils-lib
)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-shader-from-glsl-shader
    TYPE                    BINARY
    ENABLE_INSTALL          ON

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_SOURCE            ramses-shader-from-glsl-shader/main.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-shader-tools-lib
)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-effect-from-glsl-shader
    TYPE                    BINARY
    ENABLE_INSTALL          ON

    #==========================================================================
    # files of this module
    #==========================================================================
    FILES_SOURCE            ramses-effect-from-glsl-shader/main.cpp

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-shader-tools-lib
)

ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-shader-tools-test
    TYPE                    TEST

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_BASE            test
    FILES_PRIVATE_HEADER    test/*.h
    FILES_SOURCE            test/*.cpp
    FILES_RESOURCE          test/res/*

    #==========================================================================
    # dependencies
    #==========================================================================
    DEPENDENCIES            ramses-shader-tools-lib
                            gmock_main
)
