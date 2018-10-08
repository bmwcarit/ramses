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

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$RAMSES_BUILD/install \
    -Dramses-sdk_BUILD_TESTS=1 \
    -Dramses-sdk_BUILD_SMOKE_TESTS=1 \
    -Dramses-sdk_BUILD_EXAMPLES=1 \
    -Dramses-sdk_ENABLE_WAYLAND_SHELL=1 \
    -Dramses-sdk_ENABLE_DLT=0 \
    -G Ninja \
    -Wno-dev \
    $RAMSES_SOURCE

cmake --build $RAMSES_BUILD --target install
