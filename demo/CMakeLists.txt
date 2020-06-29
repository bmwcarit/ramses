#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF(ramses-sdk_BUILD_DEMOS)
    ADD_SUBDIRECTORY(ramses-text-layout-demo)
    ADD_SUBDIRECTORY(ramses-text-shadow-demo)
    ADD_SUBDIRECTORY(ramses-dcsm-list)
    ADD_SUBDIRECTORY(android)
    ADD_SUBDIRECTORY(ramses-dcsm-scene-references-demo)
ENDIF()
