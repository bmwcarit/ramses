#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_subdirectory(test-content)
add_subdirectory(renderer-test-utils)
add_subdirectory(renderer-tests)
add_subdirectory(smoke-tests)
add_subdirectory(resource-stress-tests)
add_subdirectory(render-backend-tests)

if(ramses-sdk_ENABLE_LOGIC AND ramses-sdk_USE_IMAGEMAGICK AND ramses-sdk_BUILD_FULL_SHARED_LIB)
    add_subdirectory(logic-viewer-tests)
endif()
