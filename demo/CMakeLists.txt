#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if(CMAKE_SYSTEM_NAME  STREQUAL "Android")
    add_subdirectory(android)
elseif(CMAKE_SYSTEM_NAME  STREQUAL "iOS")
    add_subdirectory(iOS)
endif()
