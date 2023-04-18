#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

expandModuleByPlatform(
    NAME            ResourceStressTests
    TYPE            BINARY
    LINKED_TYPE     STATIC
    ENABLE_INSTALL  ON
    SRC_FILES           src/*.cpp
                        src/*.h
    RESOURCE_FOLDERS    ../../TestContent/res
    DEPENDENCIES        TestContent FrameworkTestUtils RendererTestUtils ramses-cli
)

RENDERER_TEST_PER_CONFIG(ResourceStressTests RNDSANDWICHTEST)
