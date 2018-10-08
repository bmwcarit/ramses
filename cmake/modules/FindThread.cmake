#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

SET(Thread_INCLUDE_DIRS "")
IF("${TARGET_OS}" STREQUAL "Linux")
    FIND_PACKAGE(Threads REQUIRED)
    SET(Thread_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")
ELSE()
    SET(Thread_LIBRARIES "")
ENDIF()
SET(Thread_FOUND "TRUE")
