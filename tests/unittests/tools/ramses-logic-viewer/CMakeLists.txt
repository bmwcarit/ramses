#  -------------------------------------------------------------------------
#  Copyright (C) 2022 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    ramses-logic-viewer-test
    TYPE                    BINARY
    ENABLE_INSTALL          ON
    INCLUDE_PATHS           ${PROJECT_SOURCE_DIR}/tools/ramses-logic-viewer
                            ${PROJECT_SOURCE_DIR}/tests/unittests/client/logic/shared
    SRC_FILES               LogicViewerTestBase.h
                            LogicViewerTestBase.cpp
                            LogicViewerTest.cpp
                            LogicViewerLuaTest.cpp
    DEPENDENCIES            ramses-logic-viewer-lib
                            ramses-client
                            ramses-gmock
                            ramses-gmock-main
)

MakeTestFromTarget(
    TARGET ramses-logic-viewer-test
    SUFFIX UNITTEST)
