#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModuleWithRenderer(
    NAME            resource-stress-tests
    TYPE            BINARY
    ENABLE_INSTALL  ON
    SRC_FILES           *.cpp
                        *.h
    DEPENDENCIES        test-content framework-test-utils RendererTestUtils ramses-cli
)

makeTestPerWindowTypeFromTarget(
    TARGET resource-stress-tests
    SUFFIX RNDSANDWICHTEST
)
