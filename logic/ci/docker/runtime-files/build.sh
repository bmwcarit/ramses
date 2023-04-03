#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

TARGETS="L64_GCC L64_LLVM CHECK_FLATBUF_GEN CLANG_TIDY THREAD_SANITIZER ADDRESS_SANITIZER UB_SANITIZER L64_GCC_LTO ANDROID_LIB_x86 ANDROID_LIB_x86_64 ANDROID_LIB_arm64-v8a ANDROID_LIB_armeabi-v7a L64_LLVM_COVERAGE L64_LLVM_SHUFFLE L64_LLVM_HEADLESS"

# preset defaults
CONFIG=Release

if [ $# -eq 2 ]; then
    TARGET=$1
    CONFIG=$2
elif [ $# -eq 1 ]; then
    TARGET=$1
else
    echo "Usage: $0 <target> [config (default Release)]"
    echo ""
    echo "target             one of: $TARGETS"
    echo "config             one of: Debug Release (or any other supported CMake build type)"
    echo ""
    exit 0
fi

if [ "$BUILD_DIR" = "" ]; then
    echo "$0 error: ENV[BUILD_DIR] is undefined!"
    exit -1
fi

SCRIPT_DIR=$( cd "$( dirname $(realpath "${BASH_SOURCE[0]}") )" && pwd )
export RL_SRC=$(realpath "${SCRIPT_DIR}/../../..")

case $TARGET in
    L64_GCC)
             TOOLCHAIN=$RL_SRC/cmake/toolchain/Linux_X86_64.toolchain
             ;;
    L64_GCC_LTO)
             TOOLCHAIN=$RL_SRC/cmake/toolchain/Linux_X86_64.toolchain
             ;;
    L64_LLVM|L64_LLVM_COVERAGE|L64_LLVM_SHUFFLE|CHECK_FLATBUF_GEN|L64_LLVM_HEADLESS)
            TOOLCHAIN=$RL_SRC/cmake/toolchain/Linux_X86_64_llvm.toolchain
            ;;
    L64_GCC_CLIENT_ONLY)
             TOOLCHAIN=$RL_SRC/cmake/toolchain/Linux_X86_64.toolchain
             ;;
    CLANG_TIDY)
             TOOLCHAIN=$RL_SRC/cmake/toolchain/Linux_X86_64_llvm.toolchain
             CONFIG=Debug
             ;;
    THREAD_SANITIZER)
             TOOLCHAIN=$RL_SRC/cmake/toolchain/Linux_X86_64_llvm_sanitize_thread.toolchain
             ;;
    ADDRESS_SANITIZER)
             TOOLCHAIN=$RL_SRC/cmake/toolchain/Linux_X86_64_llvm_sanitize_address.toolchain
             ;;
    UB_SANITIZER)
             TOOLCHAIN=$RL_SRC/cmake/toolchain/Linux_X86_64_llvm_sanitize_ub.toolchain
             ;;
    ANDROID_LIB_x86 | ANDROID_LIB_x86_64 | ANDROID_LIB_arm64-v8a | ANDROID_LIB_armeabi-v7a)
             ;;
    MGU22)
             ;;
    *)
             echo "$0 error: unknown target '$TARGET', valid targets = $TARGETS"
             exit 1
esac

set -e

BUILD_DIR=$BUILD_DIR/$TARGET-$CONFIG
INSTALL_DIR=$BUILD_DIR/install
INSTALL_CHECK_DIR=$BUILD_DIR/install-check
mkdir -p $BUILD_DIR &> /dev/null
mkdir -p $BUILD_DIR/install &> /dev/null

pushd $BUILD_DIR

source $RL_SRC/ci/docker/runtime-files/runTests.sh

# TODO Violin refactor argument passing to build script and make this an argument
if [ -z "$BUILD_OSS" ]; then
    echo "Running CMake with pedantic warning config"
    CMAKE_WARNINGS="-Wdev -Werror=dev -Wdeprecated -Werror=deprecated"
    IGNORE_INSTALL_FILES=""
else
    echo "Allow CMake warnings"
    CMAKE_WARNINGS=""
    # TODO Violin offer upstream sol change to disable installation
    IGNORE_INSTALL_FILES=" --ignore ^include/sol --ignore ^share/pkgconfig/sol2 --ignore ^lib/cmake/sol2"
fi

# TODO Violin/Tobias this needs more cleanup!

if [ "$TARGET" = "ANDROID_LIB_x86" ] || [ "$TARGET" = "ANDROID_LIB_x86_64" ] || [ "$TARGET" = "ANDROID_LIB_arm64-v8a" ] || [ "$TARGET" = "ANDROID_LIB_armeabi-v7a" ]; then
    abi=$(echo $TARGET | cut -d"_" -f 3-)
    echo "Building android lib with abi $abi"
    cmake \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
        -DCMAKE_BUILD_TYPE=$CONFIG \
        -DCMAKE_TOOLCHAIN_FILE=/opt/Android/Sdk/ndk-bundle/build/cmake/android.toolchain.cmake \
        # Can't generate code on android
        -Dramses-sdk_ENABLE_FLATBUFFERS_GENERATION=OFF \
        -DANDROID_PLATFORM=18 \
        -DANDROID_ABI=$abi \
        -G Ninja \
        $CMAKE_WARNINGS \
        $RL_SRC

    cmake --build $BUILD_DIR --config $CONFIG --target install

elif [ "$TARGET" = "MGU22" ]; then

    export MGU_SDKROOT=${MGU_SDKROOT:-/opt/mgu22}
    source ${MGU_SDKROOT}/environment-setup-aarch64-poky-linux

    cmake \
        -DCMAKE_BUILD_TYPE=$CONFIG \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
        -Dramses-sdk_ENABLE_FLATBUFFERS_GENERATION=OFF \
        -Dramses-logic_PLATFORM=LINUX-WAYLAND \
        -Dramses-logic_BUILD_DOCUMENTATION=0 \
        -G Ninja \
        $RL_SRC

    cmake --build $BUILD_DIR --config $CONFIG --target install

elif [ "$TARGET" = "CLANG_TIDY" ]; then

    cmake \
     -DCMAKE_BUILD_TYPE=$CONFIG \
     -Dramses-logic_USE_IMAGEMAGICK=ON \
     -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN \
     -DCMAKE_INSTALL_PREFIX=$BUILD_DIR/install \
     -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
     -G Ninja \
     $CMAKE_WARNINGS \
     $RL_SRC

    pushd $RL_SRC
    $RL_SRC/ci/scripts/clang-tidy-wrapper.py --config $RL_SRC/ci/scripts/config/clang-tidy-wrapper.yaml $BUILD_DIR/compile_commands.json
    popd

elif [ "$TARGET" == "THREAD_SANITIZER" ] || [ "$TARGET" == "ADDRESS_SANITIZER" ] || [ "$TARGET" == "UB_SANITIZER" ]; then
    cmake \
        -DCMAKE_BUILD_TYPE=$CONFIG \
        -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        -G Ninja \
        $CMAKE_WARNINGS \
        $RL_SRC

    cmake --build $BUILD_DIR --config $CONFIG --target all
    export UBSAN_OPTIONS=report_error_type=1:symbolize=1:print_stacktrace=1
    export ASAN_OPTIONS=symbolize=1:alloc_dealloc_mismatch=0
    export LSAN_OPTIONS=symbolize=1:print_stacktrace=1:suppressions=$RL_SRC/ci/config/sanitizer/lsan_suppressions.txt
    export TSAN_OPTIONS=symbolize=1:print_stacktrace=1:suppressions=$RL_SRC/ci/config/sanitizer/tsan_suppressions.txt
    run_tests

# Check that generated files are consistent with checked-in ones
# Attention: when running this locally, the source tree has to be mounted as rw
# see start_container.sh for details how to do that
elif [ "$TARGET" == "CHECK_FLATBUF_GEN" ]; then
    cmake \
        -DCMAKE_BUILD_TYPE=$CONFIG \
        -Dramses-sdk_ENABLE_FLATBUFFERS_GENERATION=ON \
        -G Ninja \
        $CMAKE_WARNINGS \
        $RL_SRC

    # Explicitly call FlatbufGen to make sure the _gen.h files are overwritten
    cmake --build $BUILD_DIR --config $CONFIG --target FlatbufGen
    #If any file is dirty (i.e. different than the checked-in version), report error
    git -C $RL_SRC diff --exit-code .

    # build the test asset producer
    cmake --build $BUILD_DIR --config $CONFIG --target testAssetProducer

    # Check that test binary generation produces the same result when called twice
    pushd $BUILD_DIR/bin
    ./testAssetProducer . f1.ramses f1.rlogic
    ./testAssetProducer . f2.ramses f2.rlogic

    if cmp f1.ramses f2.ramses ; then
        echo "Ramses produces deterministic binary files!"
    else
        exit 1
    fi

    if cmp f1.rlogic f2.rlogic ; then
        echo "Ramses logic produces deterministic binary files!"
    else
        exit 1
    fi

    popd

else
    # enable LTO build when requested by target type
    if [ "$TARGET" = "L64_GCC_LTO" ]; then
        ENABLE_LTO=ON
    else
        ENABLE_LTO=OFF
    fi

    # Don't run benchmarks generally, except on LLVM Release build (gate job)
    RUN_BENCHMARKS=OFF
    # enable docs generation, but only in one build to save build resources
    if [ "$TARGET" = "L64_LLVM" ]; then
        BUILD_DOCS=ON

        if [ "$CONFIG" = "Release" ]; then
            RUN_BENCHMARKS=ON
        fi
    else
        BUILD_DOCS=OFF
    fi

    # enable coverage build when requested by target type
    if [ "$TARGET" = "L64_LLVM_COVERAGE" ]; then
        ENABLE_COVERAGE=ON
    else
        ENABLE_COVERAGE=OFF
    fi

    if [ "$TARGET" = "L64_LLVM_HEADLESS" ]; then
        BUILD_EXAMPLES=OFF
        BUILD_RENDERER=OFF
    else
        BUILD_EXAMPLES=ON
        BUILD_RENDERER=ON
    fi

    # enable test shuffling when requested by target type
    if [ "$TARGET" = "L64_LLVM_SHUFFLE" ]; then
        export GTEST_REPEAT=20
        export GTEST_SHUFFLE=1
    fi

    cmake \
        -DCMAKE_BUILD_TYPE=$CONFIG \
        -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
        -Dramses-logic_BUILD_EXAMPLES=$BUILD_EXAMPLES \
        -Dramses-logic_BUILD_RAMSES_RENDERER=$BUILD_RENDERER \
        -Dramses-logic_FORCE_BUILD_DOCS=$BUILD_DOCS \
        -Dramses-logic_BUILD_WITH_LTO=${ENABLE_LTO} \
        -Dramses-logic_USE_IMAGEMAGICK=ON \
        -Dramses-sdk_ENABLE_FLATBUFFERS_GENERATION=OFF \
        -Dramses-logic_ENABLE_TEST_COVERAGE=${ENABLE_COVERAGE} \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        -G Ninja \
        $CMAKE_WARNINGS \
        $RL_SRC

    cmake --build $BUILD_DIR --config $CONFIG --target install

    if [ "$BUILD_DOCS" = "ON" ]; then
        ninja rlogic-sphinx
    fi

    pushd $BUILD_DIR
    run_tests
    popd

    # process coverage information
    if [ "$TARGET" = "L64_LLVM_COVERAGE" ] && [ -n "${GITHUB_PR}" ] && [ -n "${GITHUB_URL}" ]; then
        echo "Generate PR coverage"
        $RL_SRC/ci/docker/runtime-files/collect-coverage.py --prof-dir $BUILD_DIR/bin --export-file $BUILD_DIR/coverage.json
        $RL_SRC/ci/scripts/annotate-coverage.py \
            --coverage $BUILD_DIR/coverage.json \
            --github-url ${GITHUB_URL} \
            --repo-owner ramses \
            --repo-name ramses-logic \
            --pr ${GITHUB_PR} \
            --html-output $BUILD_DIR/pr-coverage.html \
            --post-log-link-in-issue "${ZUUL_LOG_URL}"
    fi

    # Perform install checks
    # They are quick, so we execute them always
    echo "Checking installed headers"
    if [ "$TARGET" = "L64_LLVM_HEADLESS" ]; then
        python3 $RL_SRC/ci/scripts/installation-check/check-installation.py --headless --install-dir $INSTALL_DIR/ $IGNORE_INSTALL_FILES --src-dir $RL_SRC/
    else
        python3 $RL_SRC/ci/scripts/installation-check/check-installation.py --install-dir $INSTALL_DIR/ $IGNORE_INSTALL_FILES --src-dir $RL_SRC/
    fi
    echo "Building against shared library"
    bash $RL_SRC/ci/scripts/installation-check/check-build-with-install-shared-lib.sh $BUILD_DIR/install-check/ $INSTALL_DIR
    echo "Building against ramses logic as a source tree (submodule) and linking statically"
    if [ "$TARGET" = "L64_LLVM_HEADLESS" ]; then
        bash $RL_SRC/ci/scripts/installation-check/check-build-with-submodule.sh submodule-check-headless $BUILD_DIR/build-with-submodule/ $RL_SRC $INSTALL_DIR/
    else
        bash $RL_SRC/ci/scripts/installation-check/check-build-with-submodule.sh submodule-check $BUILD_DIR/build-with-submodule/ $RL_SRC $INSTALL_DIR/
    fi

    if [ "$RUN_BENCHMARKS" = "ON" ]; then
        pushd $BUILD_DIR/bin
        ./benchmarks
        popd
    fi
fi

popd
