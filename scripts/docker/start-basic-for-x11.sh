#!/bin/bash

#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set -e

if [ -z "$DISPLAY" ]; then
    echo "DISPLAY is not set, abort container startup"
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

# allow current user access to x server
xhost +si:localuser:$(id -un)

# map x server socket into container and insert DISPLAY env variable,
# so container applications can connect to host x server
docker run \
    -i \
    -t \
    --rm \
    --init \
    --user=$(id -u):$(id -g) \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -e DISPLAY=$DISPLAY \
    -v $SRC_ROOT:/home/ramses-build/git:ro \
    -v $BUILD_ROOT:/home/ramses-build/build \
    ramses-basic

# reset access to x server
xhost -si:localuser:$(id -un)
