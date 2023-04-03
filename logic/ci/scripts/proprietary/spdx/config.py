#  -------------------------------------------------------------------------
#  Copyright (C) 2021 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

spdx_path = 'ci/scripts/proprietary/spdx/generated'
oss_archive_svn = 'https://asc-repo.bmwgroup.net/svn/sup045/Components/mgu-distrender-ramses/trunk/ramses-logic-oss/v${RAMSES_LOGIC_VERSION}'
oss_archive_mgu22_artifactory = 'https://mgu22.artifactory.cc.bmwgroup.net/artifactory/mgu22-hmi-bin/ramses-logic-oss/v${RAMSES_LOGIC_VERSION}'
extra_self_licenses = ['CLOSED', 'MPL-2.0']   # logic itself is part closed source, part MPL 2.0

configs = [
    # Google flatbuffers
    {
        'name': 'Flatbuffers',
        'description': 'Cross platform serialization library',
        'pathname': 'flatbuffers',
        'version': '1.12.0-modified',
        'license_path': 'LICENSE.txt',
        'license_id': 'Apache-2.0',
        'copyright': 'Copyright 2021 Google Inc. All rights reserved.',
        'originator': 'Google, Inc.',
        'cpe-vendor': 'google',
        'filters': ['mguTarget', 'apinext'],
    },
    # Fmt lib
    {
        'name': 'fmt',
        'description': 'fmt is an open-source formatting library for C++',
        'pathname': 'fmt',
        'version': '7.1.3',
        'license_path': 'LICENSE.rst',
        'license_id': 'MIT',
        'copyright': 'Copyright (c) 2012 - present, Victor Zverovich',
        'originator': None,
        'cpe-vendor': 'fmtlib',
        'filters': ['mguTarget', 'apinext'],
    },
    # Google benchmarks
    {
        'name': 'Google Benchmark',
        'description': 'A microbenchmark support library',
        'pathname': 'google-benchmark',
        'version': '1.5.2-modified',
        'license_path': 'LICENSE',
        'license_id': 'Apache-2.0',
        'copyright': 'Copyright 2015 Google Inc. All rights reserved.',
        'originator': 'Google, Inc.',
        'cpe-vendor': 'google',
    },
    # googletest
    {
        'name': 'googletest',
        'description': "Google's C++ test framework.",
        'pathname': 'googletest',
        'version': 'v1.11.0-modified',
        'license_path': 'LICENSE',
        'copyright': 'Copyright 2008, Google Inc.',
        'license_id': 'BSD-3-Clause',
        'originator': 'Google, Inc.',
        'cpe-vendor': 'google',
    },
    # ImGui
    {
        'name': 'ImGui',
        'description': 'Dear ImGui: Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies',
        'pathname': 'imgui',
        'version': '1.74-modified',
        'license_path': 'LICENSE.txt',
        'license_id': 'MIT',
        'copyright': 'Copyright (c) 2014-2019 Omar Cornut',
        'originator': None,
        'cpe-vendor': 'imgui',
    },
    # Lua
    {
        'name': 'Lua',
        'description': 'Lua - An Extensible Extension Language',
        'pathname': 'lua',
        'version': '5.1.1-modified',
        'license_path_snippet': ['lua.h', 361, 381],
        'license_id': 'MIT',
        'copyright': 'Copyright (C) 1994-2006 Lua.org, PUC-Rio.  All rights reserved.',
        'originator': 'Lua.org, PUC-Rio, Brazil (http://www.lua.org)',
        'filters': ['mguTarget', 'apinext'],
        'cpe-vendor': 'lua',
    },
    # Sol
    {
        'name': 'Sol',
        'description': 'Sol - a C++ library binding to Lua',
        'pathname': 'sol',
        'version': '3.2.2-modified',
        'license_path': 'LICENSE.txt',
        'license_id': 'MIT',
        'copyright': 'Copyright (c) 2013-2020 Rapptz, ThePhD, and contributors.',
        'originator': None,
        'cpe-vendor': 'sol',
        'filters': ['mguTarget', 'apinext'],
    },
    # CLI11
    {
        'name': 'CLI11',
        'description': 'CLI11: Command line parser for C++11',
        'pathname': 'cli11',
        'version': '2.2.0',
        'license_path': 'LICENSE',
        'license_id': 'BSD-3-Clause',
        'copyright': 'Copyright (c) 2017-2022 University of Cincinnati',
        'originator': None,
        'cpe-vendor': 'cli11',
        'filters': ['mguTarget'],
    },
]
