#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_subdirectory(ramses-daemon)

add_subdirectory(test-asset-producer)
add_subdirectory(ramses-viewer)

if(TARGET ramses-renderer)
    add_subdirectory(ramses-renderer-standalone)
    add_subdirectory(ramses-imgui)
    add_subdirectory(ramses-stream-viewer)
endif()
