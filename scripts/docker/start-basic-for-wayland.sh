#!/bin/bash

#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set -e

if [ -z "$XDG_RUNTIME_DIR" ]; then
    echo "XDG_RUNTIME_DIR is not set, abort container startup"
    exit
fi

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
SRC_DIR=$(realpath "${SCRIPT_DIR}/../../")
SRC_ROOT=$(realpath "${SRC_DIR}")
BASE_PATH=$(realpath -s "${SRC_ROOT}/..")
BUILD_ROOT=$(realpath -s "${BASE_PATH}/build-basic-docker")

rm -rf "${BUILD_ROOT}"
mkdir -p "${BUILD_ROOT}"

echo "Build root:         $BUILD_ROOT"

# map wayland server socket located in XDG_RUNTIME_DIR into container and forward this env variable,
# so container applications can connect to host wayland server
docker run \
    -i \
    -t \
    --rm \
    --init \
    --user=$(id -u):$(id -g) \
    -e XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
    -v $XDG_RUNTIME_DIR:$XDG_RUNTIME_DIR \
    -v $SRC_ROOT:/home/ramses-build/git:ro \
    -v $BUILD_ROOT:/home/ramses-build/build \
    ramses-basic
