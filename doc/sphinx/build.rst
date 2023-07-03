..
    -------------------------------------------------------------------------
    Copyright (C) 2020 BMW AG
    -------------------------------------------------------------------------
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
    -------------------------------------------------------------------------

.. _build-instructions:

The following sections describe different build scenarios with detailed
options and explanations. If you are a versed C++ developer and are familiar
with CMake, consider using the quickstart guide.

========================================
Cloning
========================================

The ``RAMSES`` project uses `Git submodules <https://git-scm.com/book/en/v2/Git-Tools-Submodules>`_
to bundle its dependencies. Git doesn't automatically
clone these, the user has to clone them explicitly like this:

.. code-block:: bash

    git submodule update --init --recursive

The "recursive" option makes sure that sub-dependencies are cloned as well. Beware that using
the ``download as ZIP`` option in ``GitHub`` also doesn't package dependencies, so if you go for
this option, make sure that you download the submodules yourself, otherwise the build will fail.

========================================
Build requirements
========================================

``RAMSES`` requires CMake 3.13 or newer and a C++17-compatible compiler:

* Clang6 or newer
* GCC7 or newer
* MSVC2017 (_MSC_VER >= 1914)

``RAMSES`` requires the following
additional build dependencies:

* Windows
    * A graphics card and drivers which supports OpenGL 4.2 or newer
* Linux
    * X11 or Wayland (or both) developer libraries
    * OpenGL + EGL developer libraries

For example, for ``Ubuntu 18.04 LTS`` these can be installed like this:

.. code-block:: bash

    sudo apt-get install \
            libx11-dev \
            libwayland-dev \
            libgles2-mesa-dev

========================================
Build options
========================================

A standard CMake build will create the shared library version of ``Ramses``. You can optinally
modify some of the build settings:


* -Dramses-sdk_WARNINGS_AS_ERRORS
    * options: ON/OFF
    * default: ON
    * treats compiler warnings as errors and aborts the build. Use this option if your compiler generates warnings which are not fixed yet.

* -Dramses-sdk_ENABLE_INSTALL
    * options: ON/OFF
    * default: ON
    * Set this to OFF to prevent Ramses installing its targets, headers, and documentation. Use this if you link statically and don't
      have to expose any Ramses headers or ship the shared library with your installation.

* -Dramses-sdk_BUILD_WITH_LTO
    * options: ON/OFF
    * default: OFF
    * turns clang's link-time optimizations on (details `here <https://llvm.org/docs/LinkTimeOptimization.html>`_)

* -DCMAKE_TOOLCHAIN_FILE=<file>
    * options: any of the files in `cmake/toolchain <https://github.com/bmwcarit/ramses/tree/master/cmake/toolchain>`_ or your custom cross-compilation toolchain file
    * default: not set
    * This is a standard CMake feature. We provide several toolchain files for popular compilers, use them or create your own

* -Dramses-sdk_PACKAGE_TYPE=<type>
    * options: any of the `supported CPack generators <https://cmake.org/cmake/help/latest/manual/cpack-generators.7.html>`_
    * default: TGZ
    * Allows to control which type of package is built by CMake/CPack when the 'package' target is built. See CPack docs for 'CPACK_GENERATOR' for details


``Ramses`` can produce two types of shared libs, and both types can be enabled or disabled independently:

* -Dramses-sdk_BUILD_FULL_SHARED_LIB
    * options: ON/OFF
    * default: ON
    * Builds a version of the ramses shared library **with** renderer support. At least one window type must be enabled for build.

* -Dramses-sdk_BUILD_HEADLESS_SHARED_LIB
    * options: ON/OFF
    * default: OFF
    * Builds a version of the ramses shared library **without** renderer support.


``Ramses`` needs (at least) one platform-specific window type to support rendering. Several window types can be included into one build (e.g. X11 and Wayland). By default one suitable window type will be auto-detected.
Build will fail if dependencies for an enabled window type are not found.

Supported window types can be controlled with the cmake options below:

* -Dramses-sdk_ENABLE_DEFAULT_WINDOW_TYPE
    * options: ON/OFF
    * default: ON
    * Enables a default (auto-detected) window type IF (and ONLY IF) no window type is explicitly enabled. It makes sense to disable this option if you only need a headless build.

* -Dramses-sdk_ENABLE_WINDOW_TYPE_WINDOWS
    * options: ON/OFF
    * default: ON
    * Enables building window type *Windows*.

* -Dramses-sdk_ENABLE_WINDOW_TYPE_X11
    * options: ON/OFF
    * default: ON
    * Enables building window type *X11*.

* -Dramses-sdk_ENABLE_WINDOW_TYPE_ANDROID
    * options: ON/OFF
    * default: ON
    * Enables building window type *Android*.

* -Dramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI
    * options: ON/OFF
    * default: ON
    * Enables building window type *Wayland with ivi_application*.

* -Dramses-sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL
    * options: ON/OFF
    * default: ON
    * Enables building window type *Wayland with wl_shell*.


You can use the following options to disable some of the Ramses features:

* -Dramses-sdk_ENABLE_LOGIC
    * options: ON/OFF
    * default: ON
    * Enables the logic subsystem of ramses alongside its dependencies (Lua, Sol, ...)

* -Dramses-sdk_BUILD_DAEMON
    * options: ON/OFF
    * default: ON
    * Builds the ramses-daemon executable (for distributed rendering)

* -Dramses-sdk_TEXT_SUPPORT
    * options: ON/OFF
    * default: ON
    * enables the text subsystem of ramses alongside its dependencies (freetype, harfbuzz...)

* -Dramses-sdk_ENABLE_TCP_SUPPORT
    * options: ON/OFF
    * default: ON
    * Toggle support for TCP as communication medium between the ramses components. Disables asio when turned off.

* -Dramses-sdk_ENABLE_DLT
    * options: ON/OFF
    * default: ON
    * Enables DLT support - a logging library for automotive/embedded logging.

Additionally, you can disable additional examples, demos and tools:

* -Dramses-sdk_BUILD_EXAMPLES
    * options: ON/OFF
    * default: ON if ``Ramses`` is a top level project, otherwise OFF by default
    * set to OFF if you don't need the examples and want to reduce building time

* -Dramses-sdk_BUILD_DEMOS
    * options: ON/OFF
    * default: ON if ``Ramses`` is a top level project, otherwise OFF by default
    * set to OFF if you don't need the demos and want to reduce building time

* -Dramses-sdk_BUILD_TOOLS
    * options: ON/OFF
    * default: ON if ``Ramses`` is a top level project, otherwise OFF by default
    * set to OFF if you don't need the tools (e.g. imgui-based viewer) and want to reduce building time

* -Dramses-sdk_BUILD_TESTS
    * options: ON/OFF
    * default: ON if ``Ramses`` is a top level project, otherwise OFF by default
    * Build ramses tests.

* -Dramses-sdk_ENABLE_TEST_COVERAGE
    * options: ON/OFF
    * default: OFF
    * enables clang's options to generate code coverage from test executables


For other supported cmake options, please refer to CMakeLists.txt.


=======================================
Project version
=======================================

You can check the project version at build time and at runtime. At build time it is available
as a CMake cache variable named ``ramses_VERSION``. At runtime, you can use the
:func:`ramses::GetRamsesVersion` function to get the version as integer or as string.

========================================
Building on Windows
========================================

- Start CMake GUI
- Select <ramses-logic> as source path, choose arbitrary <build> folder.
- Click 'Configure'
- Optionally, set some of the options above
- Click 'Generate'
- Open solution in Visual Studio


========================================
Building on Linux natively
========================================

Assuming you have met the :ref:`build requirements`, you can build ``RAMSES`` like this:

.. code-block:: bash

    cd <ramses-src>
    mkdir build && cd build
    export OPTIONS="-Dramses-sdk_OPTION_1=ON ..." # see above for a list of options
    cmake $OPTIONS ../
    make

