#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

FROM alpine:3.8

RUN apk add --no-cache \
        bash \
        cmake \
        g++ \
        git \
        ninja \
        mesa-dev \
        mesa-dri-swrast \
        wayland-dev \
        ragel # needed for harfbuzz build

COPY entrypoint.sh /entrypoint.sh
RUN chmod ugo+rx /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

# export configuration flags
ENV RAMSES_SOURCE=/home/ramses-build/git \
    RAMSES_BUILD=/home/ramses-build/build

# define the volumes, which must be
# mounted during 'docker run' call
VOLUME  /home/ramses-build/git \
        /home/ramses-build/build

# add link to build script to container (for convenience)
RUN ln -s $RAMSES_SOURCE/scripts/docker/runtime-files/build-ramses.sh /home/ramses-build/build-ramses.sh \
    && ln -s $RAMSES_SOURCE/scripts/docker/runtime-files/run-unittests.sh /home/ramses-build/run-unittests.sh
