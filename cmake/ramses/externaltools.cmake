#  -------------------------------------------------------------------------
#  Copyright (C) 2019 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

### find python3

# ensure python3 can be found (when python2 was searched before)
unset(PYTHONINTERP_FOUND CACHE)
unset(PYTHON_EXECUTABLE CACHE)
FIND_PACKAGE(PythonInterp 3.6)

if (PYTHONINTERP_FOUND AND PYTHON_EXECUTABLE)
    message(STATUS "+ python3")
    set(ramses-sdk_PYTHON3 "${PYTHON_EXECUTABLE}")
else()
    message(STATUS "- python3")
endif()
