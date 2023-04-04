#!/usr/bin/env bash

#  -------------------------------------------------------------------------
#  Copyright (C) 2022 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set -e

usage() { echo "Usage: $0 -t <tag>]" 1>&2; exit 1; }

while getopts ":t:r:" o; do
    case "${o}" in
        t)
            tag=${OPTARG}
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

SCRIPT_DIR=$( cd "$( dirname $(realpath "${BASH_SOURCE[0]}") )" && pwd )
REPO_ROOT=$(realpath "${SCRIPT_DIR}/../..")

cmakelists="${REPO_ROOT}/CMakeLists.txt"
if [ -f "$cmakelists" ]; then
    VERSION_MAJOR=`cat $cmakelists | grep "set(RLOGIC_VERSION_MAJOR" | sed 's/^[^0-9]*\([0-9]*\).*$/\1/'`
    VERSION_MINOR=`cat $cmakelists | grep "set(RLOGIC_VERSION_MINOR" | sed 's/^[^0-9]*\([0-9]*\).*$/\1/'`
    VERSION_PATCH=`cat $cmakelists | grep "set(RLOGIC_VERSION_PATCH" | sed 's/^[^0-9]*\([0-9]*\).*$/\1/'`
    cmake_version="v$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH"

    if [ -z "${tag}" ]; then
        echo "No tag provided! Will use tag from CMakeLists.txt to check other files"
        tag=$cmake_version
    else
        echo "Comparing $tag against version declared in $cmakelists"

        if [ "$cmake_version" = "$tag" ]; then
            echo "Tag $tag matches version in $cmakelists"
        else
            echo "Tag $tag does not match the version in $cmakelists ($cmake_version)" 1>&2
            exit 1
        fi
    fi
else
    echo "Could not find CMakeLists ($cmakelists)!" 1>&2
    exit 1
fi

changelog="${REPO_ROOT}/CHANGELOG.md"
if [ -f "$changelog" ]; then
    echo "Checking if version $tag is mentioned in $changelog"

    if [ -n "$(cat $changelog | grep $tag)" ]; then
        echo "Tag $tag is mentioned in $changelog"
    else
        echo "Tag $tag is not mentioned in $changelog" 1>&2
        exit 1
    fi

else
    echo "Could not find changelog ($changelog)!" 1>&2
    exit 1
fi

readme="${REPO_ROOT}/README.md"
if [ -f "$readme" ]; then
    echo "Checking if version $tag is mentioned in $readme"

    if [ -n "$(cat $readme | grep $tag)" ]; then
        echo "Tag $tag is mentioned in $readme"
    else
        echo "Tag $tag is not mentioned in $readme" 1>&2
        exit 1
    fi

else
    echo "Could not find the readme ($readme)!" 1>&2
    exit 1
fi
