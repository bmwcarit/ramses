#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import sys
import os
import tempfile
import json
import pytest

sys.path.append(os.path.realpath(os.path.dirname(__file__) + "/../common"))
import compilationdb  # noqa E402 module level import not at top of file


# test entry including all supported features
TEST_ENTRY_FULL = {
    "directory": "/base/build",
    "command": "/usr/bin/clang++  -DFMT_EXCEPTIONS=0 -DHAS_TCP_COMM=1 -DRAMSES_LINK_STATIC -D__CLANG_SUPPORT_DYN_ANNOTATION__ -IBuildConfig -I../framework/FrameworkTestUtils/include -I../external/cityhash/src -I../external/lodepng -I../framework/Animation/Animation/include -I../framework/Animation/AnimationAPI/include -I../framework/Communication/TransportCommon/include -I../framework/Communication/TransportTCP/include -isystem ../external/googletest/googlemock/include -isystem ../external/googletest/googlemock -isystem ../external/googletest/googletest/include -isystem ../external/googletest/googletest  -m64 -fdiagnostics-color  -std=c++14  -fPIC -pthread -fvisibility=hidden  -g  -ggdb -D_DEBUG -fno-omit-frame-pointer -O0   -Wall -Wextra -Wcast-align -Wshadow -Wformat -Wformat-security -Wvla -Wmissing-include-dirs -Wnon-virtual-dtor -Woverloaded-virtual -Wold-style-cast -Wunused -Werror -Wimplicit-fallthrough -Winconsistent-missing-override -Wmove -std=c++14 -o framework/CMakeFiles/FrameworkTestUtils.dir/FrameworkTestUtils/src/CommunicationSystemMock.cpp.o -c /base/framework/FrameworkTestUtils/src/CommunicationSystemMock.cpp",  # noqa E501 allow long string
    "file": "/base/framework/FrameworkTestUtils/src/CommunicationSystemMock.cpp"
}

# test entry that lacks output file and system includes
TEST_ENTRY_REDUCED = {
    "directory": "/base/build",
    "command": "/usr/bin/clang -DRAMSES_LINK_STATIC -IBuildConfig -I../external/linux--extension  -m64 -fdiagnostics-color  -std=c11  -fPIC -pthread -fvisibility=hidden  -g  -ggdb -D_DEBUG -fno-omit-frame-pointer -O0    -c /base/external/wayland-zwp-linux-dmabuf-v1-extension/linux-dmabuf-unstable-v1-protocol.c",  # noqa E501 allow long string
    "file": "/base/external/protocol.c"
}


def test_can_create_from_full_entry_():
    e = compilationdb.CompilationEntry(TEST_ENTRY_FULL, compdb_path=__file__)
    assert e.directory == TEST_ENTRY_FULL['directory']
    assert e.command == TEST_ENTRY_FULL['command']
    assert e.file == TEST_ENTRY_FULL['file']
    with pytest.raises(RuntimeError):
        e.relative_file
    assert e.includes == ["BuildConfig",
                          "../framework/FrameworkTestUtils/include",
                          "../external/cityhash/src",
                          "../external/lodepng",
                          "../framework/Animation/Animation/include",
                          "../framework/Animation/AnimationAPI/include",
                          "../framework/Communication/TransportCommon/include",
                          "../framework/Communication/TransportTCP/include"]
    assert e.absolute_includes == ["/base/build/BuildConfig",
                                   "/base/framework/FrameworkTestUtils/include",
                                   "/base/external/cityhash/src",
                                   "/base/external/lodepng",
                                   "/base/framework/Animation/Animation/include",
                                   "/base/framework/Animation/AnimationAPI/include",
                                   "/base/framework/Communication/TransportCommon/include",
                                   "/base/framework/Communication/TransportTCP/include"]
    assert e.system_includes == ["../external/googletest/googlemock/include",
                                 "../external/googletest/googlemock",
                                 "../external/googletest/googletest/include",
                                 "../external/googletest/googletest"]
    assert e.absolute_system_includes == ["/base/external/googletest/googlemock/include",
                                          "/base/external/googletest/googlemock",
                                          "/base/external/googletest/googletest/include",
                                          "/base/external/googletest/googletest"]
    assert e.defines == ["FMT_EXCEPTIONS=0",
                         "HAS_TCP_COMM=1",
                         "RAMSES_LINK_STATIC", "__CLANG_SUPPORT_DYN_ANNOTATION__",
                         "_DEBUG"]
    assert e.output_file == "framework/CMakeFiles/FrameworkTestUtils.dir/FrameworkTestUtils/src/CommunicationSystemMock.cpp.o"
    assert e.absolute_output_file == "/base/build/framework/CMakeFiles/FrameworkTestUtils.dir/FrameworkTestUtils/src/CommunicationSystemMock.cpp.o"
    assert e.project_root is None
    assert e.compdb_path == __file__


def test_can_create_from_reduced_entry():
    e = compilationdb.CompilationEntry(TEST_ENTRY_REDUCED, compdb_path=__file__, project_root='/base')
    assert e.directory == TEST_ENTRY_REDUCED['directory']
    assert e.command == TEST_ENTRY_REDUCED['command']
    assert e.file == TEST_ENTRY_REDUCED['file']
    assert e.relative_file == 'external/protocol.c'
    assert e.includes == ["BuildConfig", "../external/linux--extension"]
    assert e.absolute_includes == ["/base/build/BuildConfig", "/base/external/linux--extension"]
    assert e.system_includes == []
    assert e.absolute_system_includes == []
    assert e.defines == ["RAMSES_LINK_STATIC", "_DEBUG"]
    with pytest.raises(RuntimeError):
        e.output_file
    with pytest.raises(RuntimeError):
        e.absolute_output_file
    assert e.project_root == "/base"
    assert e.compdb_path == __file__


def test_can_load_compdb_from_file():
    with tempfile.NamedTemporaryFile(mode='w') as f:
        json.dump([TEST_ENTRY_FULL, TEST_ENTRY_REDUCED], f)
        f.flush()
        entries = compilationdb.load_from_file(f.name)

        assert len(entries) == 2
        assert entries[0].file == "/base/framework/FrameworkTestUtils/src/CommunicationSystemMock.cpp"
        assert entries[0].compdb_path == f.name
        assert entries[1].file == "/base/external/protocol.c"
        assert entries[1].compdb_path == f.name
