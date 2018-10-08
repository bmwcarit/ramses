#!/bin/bash

#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

RAMSES_VERSION=""

if [ $# -eq 2 ]; then
    TEST_DIR=$1
    INSTALL_DIR=$2
elif [ $# -eq 3 ]; then
    TEST_DIR=$1
    INSTALL_DIR=$2
    RAMSES_VERSION=$3
else
    echo -e "Usage: $0 <build-dir> <install-dir> [<ramses-version (default none)>]\n"
    exit 0
fi

set -e

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

echo "++++ Create test environment for install check of shared lib client only ++++"

# test here
mkdir -p $TEST_DIR
cd $TEST_DIR

# run cmake config
echo "++++ Build with cmake config for install check of shared lib client only ++++"
rm -rf test-cmake.config
mkdir test-cmake.config
pushd  test-cmake.config > /dev/null

cmake -DCMAKE_PREFIX_PATH=$INSTALL_DIR -DRAMSES_VERSION=${RAMSES_VERSION} --build test-cmake.config $SCRIPT_DIR/shared-lib-client-only-check/
make

env LD_LIBRARY_PATH=${INSTALL_DIR}/lib:${LD_LIBRARY_PATH} ./ramses-shared-lib-check

popd > /dev/null

echo "++++ build check done for install check of shared lib client only ++++"
