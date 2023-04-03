#!/bin/bash

#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if [ $# -eq 4 ]; then
    TEST_DIR_NAME=$1
    BUILD_DIR=$2
    SRC_DIR=$3
    RAMSES_INSTALL=$4
else
    echo "Usage: $0 <temp-build-dir> <test-dir-name> <src-dir> <ramses-install-dir>"
    exit 1
fi

set -e

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

echo "++++ Create test environment for building statically ++++"

rm -rf $BUILD_DIR

# Create temporary build directory
mkdir -p $BUILD_DIR/ramses-logic
cd $BUILD_DIR
# Base project setup
cp -r $SCRIPT_DIR/$TEST_DIR_NAME/* .
# Add ramses logic as submodule to folder ramses-logic
cp -r $SRC_DIR/* ./ramses-logic/

echo "++++ building and executing test binary ++++"

# Build with cmake
mkdir build
cd build
cmake -GNinja -DCMAKE_PREFIX_PATH=$RAMSES_INSTALL ../
cmake --build . --target run-all

echo "++++ build check done for static submodule build ++++"
