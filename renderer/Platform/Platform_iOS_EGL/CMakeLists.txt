#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    ACME_MODULE(

        #==========================================================================
        # general module information
        #==========================================================================
        # Attention: extra care is needed in case of renaming this module
        # The name of this module (and all CONCRETE platform modules) must align with
        # the postfix of generated RAMSES libs' names. The platform name is used in
        # cmake modules and templates for the mentioned purposes.
        # The module naming convention is broken here knowledgeably
        NAME                    platform-ios-egl-es-3-0
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          OFF

        #==========================================================================
        # files of this module
        #==========================================================================
        FILES_PRIVATE_HEADER    include/Platform_iOS_EGL/*.h
        FILES_SOURCE            src/*.cpp
                                src/*.mm

        #==========================================================================
        # dependencies
        #==========================================================================
        DEPENDENCIES            Platform_EGL
                                QuartzCore
    )
endif()