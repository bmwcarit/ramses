#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

FROM ubuntu:20.04

# export configuration flags
ENV RAMSES_SOURCE=/home/ramses-build/git \
    RAMSES_BUILD=/home/ramses-build/build

# define the volumes, which must be
# mounted during 'docker run' call
VOLUME  /home/ramses-build/git \
        /home/ramses-build/build

COPY entrypoint.sh /entrypoint.sh
RUN chmod ugo+rx /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

RUN apt-get update \
    && env DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        doxygen \
        gcc g++ \
        git \
        libdrm-dev \
        libgbm-dev \
        libgles2-mesa-dev \
        libpng-dev \
        libwayland-client0 \
        libwayland-dev \
        libweston-8-dev \
        mesa-utils \
        ninja-build \
        pkg-config \
        python3-pip \
        software-properties-common \
        weston \
    && apt-get purge -y software-properties-common \
    && apt-get autoremove -y \
    && apt-get autoclean

# add ramses user and groups
RUN groupadd --gid 1000 ramses-build \
    && useradd --create-home --uid 1000 --gid ramses-build ramses-build \
    && echo "%ramses-build ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# install python requirements
COPY requirements.txt /tmp/
RUN pip3 install -r /tmp/requirements.txt && rm -f /tmp/requirements.txt

# add link to build script to container (for convenience)
RUN ln -s $RAMSES_SOURCE/scripts/docker/runtime-files/build-ramses.sh /home/ramses-build/build-ramses.sh \
    && ln -s $RAMSES_SOURCE/scripts/docker/runtime-files/run-unittests.sh /home/ramses-build/run-unittests.sh
