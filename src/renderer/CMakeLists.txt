#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_subdirectory(internal/RendererLib)
add_subdirectory(internal/Platform)

createModule(
    NAME                    ramses-renderer
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF

    SRC_FILES               impl/*.h
                            impl/*.cpp

    DEPENDENCIES            Platform
                            ramses-framework
)
