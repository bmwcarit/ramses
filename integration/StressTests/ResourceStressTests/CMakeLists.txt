#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

RENDERER_MODULE_PER_CONFIG_STATIC(ResourceStressTests
    TYPE           TEST
    TEST_SUFFIX    RNDSANDWICHTEST

    FILES_SOURCE   src/*.cpp
                   src/*.h

    FILES_RESOURCE ../../TestContent/res/*.vert
                   ../../TestContent/res/*.frag

    DEPENDENCIES   TestContent FrameworkTestUtils RendererTestUtils
)
