#!/bin/bash

#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

RENDERER_PLATFORM="LINUX"

if [ $# -eq 2 ]; then
    TEST_DIR=$1
    INSTALL_DIR=$2
elif [ $# -eq 3 ]; then
    TEST_DIR=$1
    INSTALL_DIR=$2
    RENDERER_PLATFORM=$3
else
    echo "Usage: $0 <build-dir> <install-dir> [<renderer-platform: LINUX-X11|LINUX-WAYLAND>]"
    exit 1
fi

if [[ "$RENDERER_PLATFORM" != "LINUX-X11" && "$RENDERER_PLATFORM" != "LINUX-WAYLAND" ]]; then
    echo "ERROR: unknown renderer-platform $RENDERER_PLATFORM"
    exit 1
fi

set -e

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

echo "++++ Create test environment for install check of shared lib (${RENDERER_PLATFORM}) ++++"

# test here
mkdir -p $TEST_DIR
cd $TEST_DIR

# run cmake config
echo "++++ Build with cmake config for install check of shared lib ++++"
rm -rf test-cmake.config
mkdir test-cmake.config
pushd test-cmake.config > /dev/null

export LD_LIBRARY_PATH=${INSTALL_DIR}/lib:${LD_LIBRARY_PATH}

cmake -DCMAKE_PREFIX_PATH=$INSTALL_DIR -DRAMSES_RENDERER_PLATFORM=${RENDERER_PLATFORM} --build test-cmake.config $SCRIPT_DIR/shared-lib-check/
make run-all

popd > /dev/null

echo "++++ build check done for install check of shared lib ++++"
