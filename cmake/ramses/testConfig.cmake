#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


# configure valgrind memcheck
set(MEMORYCHECK_COMMAND_OPTIONS
    "--log-fd=1 --error-exitcode=1 --leak-check=full --show-leak-kinds=definite,possible --errors-for-leak-kinds=definite,possible --undef-value-errors=yes --track-origins=no --child-silent-after-fork=yes --trace-children=yes --num-callers=50 --fullpath-after=${PROJECT_SOURCE_DIR}/ --gen-suppressions=all"
    CACHE INTERNAL "")

# enable ctest
enable_testing()
include(CTest)

