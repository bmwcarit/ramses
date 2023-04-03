#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import subprocess
import re
import os
import argparse
from pathlib import Path


def get_files_in_directory_recursive(path, ext):
    res = []
    for (dir, _, files) in os.walk(path):
        for f in files:
            if os.path.splitext(f)[1] in ext:
                res.append(os.path.join(dir, f))
    return res


def get_source_api_headers(srcDir, pattern):
    headers = []
    pattern = re.compile(pattern)
    for path in subprocess.check_output(['git', 'ls-files'], cwd=srcDir, shell=False).decode('utf-8').split():
        match = pattern.match(path)
        if match:
            rel_path = Path(path).relative_to(Path(match.group()))
            headers.append(str(rel_path))

    return headers


def main():
    parser = argparse.ArgumentParser(description='Script to verify that installation of the project has no unexpected or \
        missing files, and that all installed includes compile stand-alone')
    parser.add_argument('--src-dir', required=True, help='Ramses source dir')
    parser.add_argument('--install-dir', required=True, help='Directory where ramses was installed')
    parser.add_argument('--ignore', required=False, action='append', help='Ignore file patterns from the installation folder')
    parser.add_argument('--platform', required=True, action='append', help='Platforms (x11, wayland, ...)')
    parser.add_argument('--headless', required=False, action='store_true', help='Check for headless binaries (no ramses renderer)')
    args = parser.parse_args()

    # Expect exactly these files after installation (don't list header files here, they are cross-checked with source tree)
    expectNonHeaderFiles = [
        r"^bin/ramses-daemon$",
        r"^share/doc/ramses-sdk-\d+\.\d+\.\d+/LICENSE\.txt$",
        r"^share/doc/ramses-sdk-\d+\.\d+\.\d+/CHANGELOG\.md$",
        r"^share/doc/ramses-sdk-\d+\.\d+\.\d+/CONTRIBUTING\.rst$",
        r"^share/doc/ramses-sdk-\d+\.\d+\.\d+/README\.md$",
        # TODO: These files should be packaged separately - maybe as a tools package?
        r"^bin/ramses-logic-viewer$",
        # TODO: remove, these are a tests/demos, not needed in the package
        r"^bin/ramses-test-client$",
        r"^bin/ramses-client-test$",
        r"^bin/ramses-framework-test$",
        r"^bin/ramses-renderer-lib-test$",
        r"^bin/ramses-cli-test$",
        r"^bin/ramses-logic-viewer-unittests$",
        r"^bin/ramses-logic-viewer-swrast-tests$",
    ]

    known_platforms = ['x11-egl-es-3-0', 'wayland-shell-egl-es-3-0', 'wayland-ivi-egl-es-3-0']
    for p in args.platform:
        if p not in known_platforms:
            raise Exception(f"Unrecognized platform {p}, must choose one from {known_platforms}")

        expectNonHeaderFiles += [
            r"^lib/libramses-shared-lib-" + p + r"\.so$",
            r"^lib/libramses-shared-lib-" + p + r"\.so\.\d+\.\d+$",
            # TODO: These files should be packaged separately - maybe as a tools package?
            r"^bin/ramses-renderer-" + p + r"$",
            r"^bin/ramses-scene-viewer-" + p + r"$",
            r"^bin/ramses-stream-viewer-" + p + r"$",
            # TODO: remove, these are a tests/demos, not needed in the package
            r"^bin/ramses-local-client-test-" + p + r"$",
            r"^bin/ResourceStressTests-" + p + r"$",
            r"^bin/RenderBackendTests-" + p + r"$",
            r"^bin/RenderingTests-" + p + r"$",
            r"^bin/RendererLifecycleTests-" + p + r"$",
        ]

        # Everything else below should also not be in the package
        expectNonHeaderFiles += [r"^bin/DmaOffscreenBufferRenderingTests-" + p + r"$"]

        if p == 'wayland-ivi-egl-es-3-0':
            expectNonHeaderFiles += [
                r"^bin/ivi-gears$",
                r"^bin/ivi-simple-dmabuf-egl$",
                r"^bin/SystemCompositorController_Wayland_IVI_Test$",
                r"^bin/Window_Wayland_IVI_Test$",
                r"^bin/EmbeddedCompositor_Wayland_Test$",
                r"^bin/EmbeddedCompositingTests-wayland-ivi-egl-es-3-0$",
            ]

        if p == 'wayland-shell-egl-es-3-0':
            expectNonHeaderFiles += [r"^bin/Window_Wayland_Shell_Test$"]

        if p == 'x11-egl-es-3-0':
            expectNonHeaderFiles += [r"^bin/platform-x11-test$"]

    expectNonHeaderFiles += [
        r"^lib/cmake/ramses-shared-lib-\d+\.\d+/ramses-shared-libConfigVersion\.cmake$",
        r"^lib/cmake/ramses-shared-lib-\d+\.\d+/ramses-shared-libConfig\.cmake$",
    ]

    if args.headless:
        expectNonHeaderFiles += [
            r"^lib/cmake/ramses-shared-lib-client-only-\d+\.\d+/ramses-shared-lib-client-onlyConfigVersion\.cmake$",
            r"^lib/cmake/ramses-shared-lib-client-only-\d+\.\d+/ramses-shared-lib-client-onlyConfig\.cmake$",
            r"^bin/ramses-logic-viewer-headless$",
        ]

    installPath = Path(args.install_dir)
    includePath = installPath / 'include'

    installedHeaders = []
    unexpectedFiles = []

    for path in installPath.rglob("*"):
        if path.is_dir():
            continue

        relPathStr = str(path.relative_to(installPath))

        # Handle all cases below, don't skip anything!

        # Ramses header file - add to special list to check compilation later
        if re.match(r'^include/', relPathStr):
            installedHeaders.append(str(path.relative_to(includePath)))
        elif re.match(r'^bin/res', relPathStr):
            # Ignore resource files
            # TODO Violin: don't pollute installation packages with test resources! Don't install resources unless explicitly requested
            pass
        # ramses examples start by "ramses-example", rlogic examles start by two digits, optionally a lowercase letter then an underscore
        elif re.match(r'^bin/(ramses-example|\d\d[a-z]?_)', relPathStr):
            # Ignore examples
            # TODO Violin: don't pollute installation packages with examples
            pass
        else:
            for f in expectNonHeaderFiles:
                if re.match(f, relPathStr):
                    expectNonHeaderFiles.remove(f)
                    break
            else:
                unexpectedFiles.append(relPathStr)

    print("Checking install non-header files")

    if args.ignore:
        # This is required because we use the more
        print(f"Ignoring file patterns: {', '.join(args.ignore)}")
        unexpectedFiles = [f for f in unexpectedFiles if not any([re.search(p, f) for p in args.ignore])]
        expectNonHeaderFiles = [f for f in expectNonHeaderFiles if not any([re.search(p, f) for p in args.ignore])]

    # If all "expected" files were found, the list should be empty now - if not, report error
    if expectNonHeaderFiles:
        print("Couldn't find some files in the install folder:\n  " + "\n  ".join(expectNonHeaderFiles))
        return 1
    # Found a file that's not expected in the list? Error!
    if unexpectedFiles:
        print("Found following unexpected files in the install folder:\n  " + "\n  ".join(unexpectedFiles))
        return 1

    # Extract header files from the source tree
    srcApiHeaders = get_source_api_headers(args.src_dir, '.*/((ramses-[^/]+-api)|logic)/include')

    # check which headers are unexpected and which are missing
    unexpectedHeaders = list(set(installedHeaders) - set(srcApiHeaders))
    missingHeaders = list(set(srcApiHeaders) - set(installedHeaders))

    if len(unexpectedHeaders) > 0:
        print('ERROR: Headers should not be installed\n  ' + '\n  '.join(unexpectedHeaders))
        return 1
    if len(missingHeaders) > 0:
        print('ERROR: Headers are missing from installation\n  ' + '\n  '.join(missingHeaders))
        return 1

    # check that installed headers compile standalone with "-pedantic"
    print("Checking strict header compilation")
    numPedanticErrors = 0
    for h in installedHeaders:
        temp_file = "/tmp/ramses_pedantic_header.cpp"
        with open(temp_file, "w") as file:
            file.writelines(f"#include \"{h}\"\n\n")
            file.writelines("int main() {return 0;}")

        cmd = f'g++ -std=c++17 -Werror -pedantic -I"{str(includePath)}" "{temp_file}" -o /tmp/ramses-pedantic-header.o'
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out = p.communicate()
        if p.returncode != 0:
            print(f'Header check failed for: {h}')
            print(cmd)
            print(out[0])
            print(out[1])
            numPedanticErrors += 1

    if numPedanticErrors > 0:
        print("ERROR: found errors with strict compilation in installed headers")
        return 1

    print("Done")

    return 0


if __name__ == "__main__":
    exit(main())
