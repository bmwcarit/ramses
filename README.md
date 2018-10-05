# RAMSES
RAMSES is an abbreviation for "Rendering Architecture for Multi-Screen
EnvironmentS". It implements a distributed system for rendering 3D content
with focus on bandwidth and resource efficiency.
Within the distributed architecture RAMSES defines 2 main roles for
applications.
- Clients provide graphical content using a scene graph interface.
- Renderers receive graphical content and render it while dealing with all the
  platform specific constraints.

# Copyright and License
RAMSES original code is copyright BMW Car IT;

```Copyright (C) 2018 BMW Car IT GmbH```

The source code is licensed under the Mozilla Public License 2.0, please find a 
copy of that license in the LICENSE.txt file.

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
