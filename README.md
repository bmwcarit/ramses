<!-- RAMSES -->
# RAMSES
## Table of Contents
* [What is RAMSES](#what-is-ramses)
* [Obtaining the source code](#obtaining-the-source-code)
* [Building and testing](#building-and-testing)
* [License](#license)

## What is RAMSES

RAMSES is 3D rendering engine with focus on bandwidth and resource efficiency.
For a broader overview and introduction to the ecosystem and tools, visit our
[homepage](https://ramses3d.org/).

Have a look at our showcase video: [![RAMSES Distributed Rendering Engine Showcase Video](https://img.youtube.com/vi/tyzvEI25BMg/0.jpg)](https://www.youtube.com/watch?v=tyzvEI25BMg)

## Obtaining the source code
RAMSES can be cloned from its BMW Car IT repository using git:

```
git clone --recurse-submodules https://github.com/bmwcarit/ramses <ramses-sdk>
```

## Building and testing
General building tips: RAMSES's build system is based on CMake. It has
mandatory components and optional components which are built only if
required dependencies and/or CMake flags are present. The CMake log will
provide info what was built and what not - a 'plus' indicating that something
was built, and 'minus' that it wasn't. If an optional component was not built,
CMake will list the missing dependencies which were not found or not built.
Check the CMake logs! Typical build errors:
- not able to find a compiler -> Check that you have a valid compiler!
- not able to find something in external/ folder -> Check that you downloaded the submodules as shown in section 2.)
- CMake can't identify the compiler of Visual studio Community edition -> You need to download a Windows SDK 8.1
- No renderer was built on linux -> Check that you have installed some of the platform packages (x11-dev, egl, openGL, wayland)

Building RAMSES on Windows:
- start CMake GUI
- select <ramses-sdk> as source path, choose arbitrary <build> folder.
Configure
If you want to build the tests, set 'ramses-sdk_BUILD_TESTS' to true in the CMake cache.
generate -> open solution in Visual Studio.

Building RAMSES on Linux with docker:

We prefer to build RAMSES in Docker because it abstracts the dependency installation
and the CMake invocations away from the user. Docker is installed slightly differently
on different distros, check the docker manual for your distro. The instructions below
are for Ubuntu 18.04 LTS:

```
apt install docker.io
groupadd docker             #can fail if already exists
usermod –aG docker $USER
newgrp docker               #logs you into the new group in the current terminal session

docker run hello-world      #check that docker works

# build RAMSES docker container
cd <ramses-sdk>/scripts/docker
./build-basic-container.sh
# Start RAMSES docker
./start-basic-for-x11.sh    # (on wayland-enabled systems, optionally: ./start-basic-for-wayland.sh)
# Inside docker container
./build-ramses.sh
# optionally - check if all RAMSES features work on your system
./run-unittests.sh
```

Building RAMSES on Linux (without docker):

-Install dependencies using Distro of choice package manager. For example, for Ubuntu:
```
sudo apt-get install libx11-dev libgles2-mesa-dev
mkdir <build>
cd <build>
cmake <ramses-sdk>
```

You can also check the docker container setup scripts for a reference how to build:
```
<ramses-sdk>/scripts/docker/ramses-basic/Dockerfile         -> contains info about build dependencies
<ramses-sdk>/scripts/docker/runtime-files/build-ramses.sh   -> contains CMake command for building
```

## License
RAMSES original code is copyright BMW Car IT or BMW AG

```Copyright (C) 20xx BMW Car IT GmbH``` or ```Copyright (C) 20xx BMW AG```

The source code is licensed under the Mozilla Public License 2.0, please find a
copy of that license in the [LICENSE.txt](https://github.com/bmwcarit/ramses/blob/master/LICENSE.txt) file.

RAMSES makes use of several open source libraries which can be found in the folder 'external'.
Some of these are shipped directly with the sourcecode, others are included as git submodule references.
RAMSES also includes some assets (e.g. font files) which are licensed under different open source licenses.

Directly included:
- cityhash (Licensed under MIT License)
- Khronos Headers (Licensed under Khronos Group License)
- lodepng (Licensed under zlib License)
- Wayland-IVI Extension (Licensed under MIT License)
- wayland-zwp-linux-dmabuf-v1-extension (Licensed under MIT License)
- Wayland-IVI example client (Licensed under MIT License)

Submodule reference:
- Freetype 2 (Licensed under FTL, also containing code under BSD and ZLib)
- GLSLang (Licensed under BSD-3 and Khronos Group License)
- OpenGL Mathematics (Licensed under MIT License)
- Googletest (Licensed under BSD-3)
- Harfbuzz (Licensed under MIT and ISC; see external/harfbuzz/COPYING)
- Asio (Boost Software License - Version 1.0)
- LZ4 (Licensed under BSD-2; see also external/lz4/LICENSE for more details)
- {fmt} (Licensed under MIT License)
- ImGui (Licensed under MIT License)
- Abseil (Licensed under Apache 2.0; see also external/abseil/LICENSE for more details)
- CLI11 (Licensed under BSD-3)

Included Assets:
- Roboto Font (Licensed under Apache 2.0)
- M+ Font (Licensed under Public Domain)
- WenQuanYi MicroHei (Licensed under Apache 2.0)
- Arimo Font (Licensed under Apache 2.0)
- Droid Kufi Font (Licensed under Apache 2.0)
- Droid Naskh Font (Licensed under Apache 2.0)
- Satisfy Font (Licensed under Apache 2.0)
