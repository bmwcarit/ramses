#!/bin/bash
#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set -e

cd $RAMSES_BUILD

export GTEST_COLOR=yes

if [ -z "$XDG_RUNTIME_DIR" ]; then
    RENDERER_TEST_FILTER=".*x11.*_RNDSANDWICHTEST_SWRAST"
else
    RENDERER_TEST_FILTER=".*wayland-shell.*_RNDSANDWICHTEST_SWRAST"
fi

ctest --os test -V --tests-regex ".*UNITTEST|${RENDERER_TEST_FILTER}"
