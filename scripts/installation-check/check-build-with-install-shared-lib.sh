#!/bin/bash

#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

RENDERER_PLATFORM="LINUX"
RAMSES_VERSION=""
GL_VERSION=""

if [ $# -eq 2 ]; then
    TEST_DIR=$1
    INSTALL_DIR=$2
elif [ $# -eq 3 ]; then
    TEST_DIR=$1
    INSTALL_DIR=$2
    RENDERER_PLATFORM=$3
elif [ $# -eq 4 ]; then
    TEST_DIR=$1
    INSTALL_DIR=$2
    RENDERER_PLATFORM=$3
    GL_VERSION=$4
elif [ $# -eq 5 ]; then
    TEST_DIR=$1
    INSTALL_DIR=$2
    RENDERER_PLATFORM=$3
    GL_VERSION=$4
    RAMSES_VERSION=$5
else
    echo -e "Usage: $0 <build-dir> <install-dir> [<renderer-platform: LINUX-X11|LINUX-WAYLAND>] [<gl-version> (optional)] [<ramses-version (default none)>]\n"
    exit 0
fi

if [[ "$RENDERER_PLATFORM" != "LINUX-X11" && "$RENDERER_PLATFORM" != "LINUX-WAYLAND" ]]; then
    echo "ERROR: unknown renderer-platform $RENDERER_PLATFORM"
    exit 1
fi

set -e

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

echo "++++ Create test environment for install check of shared lib (${RENDERER_PLATFORM}, ${GL_VERSION}) ++++"

# test here
mkdir -p $TEST_DIR
cd $TEST_DIR

# run cmake config
echo "++++ Build with cmake config for install check of shared lib ++++"
rm -rf test-cmake.config
mkdir test-cmake.config
pushd  test-cmake.config > /dev/null

cmake -DCMAKE_PREFIX_PATH=$INSTALL_DIR -DRAMSES_RENDERER_PLATFORM=${RENDERER_PLATFORM} -DRAMSES_VERSION=${RAMSES_VERSION} -DGL_VERSION=$GL_VERSION --build test-cmake.config $SCRIPT_DIR/shared-lib-check/
make

if [[ "$RENDERER_PLATFORM" == "LINUX" ]]; then
    env LD_LIBRARY_PATH=${INSTALL_DIR}/lib:${LD_LIBRARY_PATH} ./ramses-shared-lib-check
fi

popd > /dev/null

echo "++++ build check done for install check of shared lib ++++"
