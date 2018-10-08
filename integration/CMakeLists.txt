#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF(ramses-sdk_BUILD_SMOKE_TESTS)
    ADD_SUBDIRECTORY(TestContent)
    ADD_SUBDIRECTORY(SandwichTests)
    ADD_SUBDIRECTORY(SmokeTests)
    ADD_SUBDIRECTORY(StressTests)
    ADD_SUBDIRECTORY(PlatformTests)
    ADD_SUBDIRECTORY(PerformanceTests)
ENDIF()

