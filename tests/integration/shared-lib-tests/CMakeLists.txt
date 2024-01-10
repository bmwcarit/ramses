#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME            ramses-shared-lib-tests
    TYPE            BINARY
    ENABLE_INSTALL  ${ramses-sdk_ENABLE_INSTALL}
    INCLUDE_PATHS   ${PROJECT_SOURCE_DIR}/tests/unittests/client/logic/shared
    SRC_FILES       LoggerTest.cpp
                    RendererEventGrabber.h
                    LogHandler.h
    #RESOURCE_FOLDERS res

    DEPENDENCIES    ramses-shared-lib
                    ramses-gmock
                    ramses-gmock-main
                    fmt::fmt
)

MakeTestFromTarget(
    TARGET ramses-shared-lib-tests
    SUFFIX RNDSANDWICHTEST_SWRAST)
