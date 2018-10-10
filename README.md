RAMSES

TOC
-------------------------------------------------
1. What is RAMSES
2. Building and testing
3. License / Open source / Third Party

1 What is RAMSES
--------------
RAMSES is an abbreviation for "Rendering Architecture for Multi-Screen
EnvironmentS". It implements a distributed system for rendering 3D content
with focus on bandwidth and resource efficiency.
Within the distributed architecture RAMSES defines 2 main roles for
applications.
- Clients provide graphical content using a scene graph interface.
- Renderers receive graphical content and render it while dealing with all the
  platform specific constraints.

2 Building and testing
-----------------------------------
RAMSES can be cloned from its Genivi repository using git:

```
git clone https://github.com/GENIVI/ramses <ramses-sdk>
cd <ramses-sdk>
git submodule update --init --recursive
```

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

#re-login or restart for the group changes to take effect

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

3 License / Open Source / Third Party
-----------------------------------
Attention: This software uses and contains open-source software!
RAMSES itself is license under the Mozilla Public License Version 2.0 (see LICENSE.txt).
RAMSES makes use of several open source libraries which can be found in the folder 'external'.
Some of these are shipped directly with the sourcecode, others are included as git submodule references.
RAMSES also includes some assets (e.g. font files) which are licensed under different open source licenses.

Directly included:
- ACME2 (Licensed under Apache 2.0)
- CAPU (Licensed under Apache 2.0)
- cityhash (Licensed under MIT License)
- Khronos Headers (Licensed under Khronos Group License)
- lodepng (Licensed under zlib License)
- LZ4 (Licensed under BSD-2)
- Wayland-IVI Extension (Licensed under MIT License)
- Wayland-IVI example client (Licensed under MIT License)

Submodule reference:
- Freetype 2 (Licensed under FTL, also containing code under BSD and ZLib)
- GLSLang (Licensed under BSD-3 and Khronos Group License)
- Googletest (Licensed under BSD-3)
- Harfbuzz (Licensed under MIT and ISC; see external/harfbuzz/COPYING)

Included Assets:
- Roboto Font (Licensed under Apache 2.0)
- M+ Font (Licensed under Public Domain)
- WenQuanYi MicroHei (Licensed under Apache 2.0)
- Arimo Font (Licensed under Apache 2.0)
- Droid Kufi Font (Licensed under Apache 2.0)
- Droid Naskh Font (Licensed under Apache 2.0)
- Satisfy Font (Licensed under Apache 2.0)
