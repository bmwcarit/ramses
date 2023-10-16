#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    ramses-renderer-lib-test
    TYPE                    BINARY
    SRC_FILES               RendererFramework/*.cpp
                            RendererFramework/*.h
                            RendererLib/*.cpp
                            RendererLib/*.h
                            RendererCommands/*.cpp
                            RendererCommands/*.h
    DEPENDENCIES            renderer-test-common
                            ramses-gmock-main
)

makeTestFromTarget(
    TARGET ramses-renderer-lib-test
    SUFFIX UNITTEST)
