#  -------------------------------------------------------------------------
#  Copyright (C) 2018 Mentor Graphics Development GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------
            
ACME_MODULE(
    NAME                    ramses-citymodel
    TYPE                    STATIC_LIBRARY

    INCLUDE_BASE            ramses-citymodel/include
    FILES_PRIVATE_HEADER    ramses-citymodel/include/ramses-citymodel/*
    FILES_SOURCE            ramses-citymodel/src/*.cpp

    DEPENDENCIES            ramses-client
                            ramses-text
                            ramses-renderer-api
                            DisplayManager
                            ramses-capu
                            ramses-demoLib
                            lz4
                            OpenCTM
                            
    ENABLE_INSTALL          ON
                            
    FILES_RESOURCE          res/*.frag
                            res/*.vert
                            res/*.ttf
                            res/*.png
                            res/*.rex
    )

RENDERER_MODULE_PER_CONFIG_STATIC(ramses-citymodel-renderer
    TYPE                    BINARY
    ENABLE_INSTALL          ON
    FILES_SOURCE            ramses-citymodel-renderer/src/*.cpp
    DEPENDENCIES            ramses-citymodel
)

ACME_MODULE(
        NAME                    ramses-citymodel-client
        TYPE                    BINARY
        ENABLE_INSTALL          ON
        FILES_SOURCE            ramses-citymodel-client/src/*.cpp
        DEPENDENCIES            ramses-citymodel
)
