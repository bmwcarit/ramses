#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import subprocess
import re
import argparse
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description='Script to verify that installation of the project has no unexpected or \
        missing files, and that all installed includes compile stand-alone')
    parser.add_argument('--src-dir', required=True, help='Ramses logic source dir')
    parser.add_argument('--install-dir', required=True, help='Directory where ramses logic was installed')
    parser.add_argument('--ignore', required=False, action='append', nargs='*', help='Ignore file patterns from the installation folder')
    parser.add_argument('--headless', required=False, action='store_true', help='Check for headless binaries (no ramses renderer)')
    args = parser.parse_args()

    # Expect exactly these files after installation (don't list header files here, they are cross-checked with source tree)
    expectNonheaderFiles = [
        # Ramses
        r"^lib/libramses-shared-lib-[\w-]+\.so$",
        r"^lib/libramses-shared-lib-[\w-]+\.so\.\d+\.\d+$",
        r"^share/doc/ramses-sdk-\d+\.\d+\.\d+/LICENSE\.txt$",
        r"^share/doc/ramses-sdk-\d+\.\d+\.\d+/CHANGELOG\.md$",
        r"^share/doc/ramses-sdk-\d+\.\d+\.\d+/README\.md$",
        # Ramses Logic
        r"^bin/ramses-logic-viewer-headless$",
        r"^lib/libramses-logic\.so$",
        r"^lib/libramses-logic\.so\.\d+$",
        r"^lib/cmake/ramses-logic-\d+\.\d+/ramses-logicConfig\.cmake$",
        r"^lib/cmake/ramses-logic-\d+\.\d+/ramses-logicConfigVersion\.cmake$",
        r"^share/doc/RamsesLogic-\d+\.\d+\.\d+/CHANGELOG\.md$",
        r"^share/doc/RamsesLogic-\d+\.\d+\.\d+/README\.md$",
        r"^share/doc/RamsesLogic-\d+\.\d+\.\d+/LICENSE\.txt$",
    ]

    if args.headless:
        expectNonheaderFiles += [
            r"^lib/cmake/ramses-shared-lib-client-only-\d+\.\d+/ramses-shared-lib-client-onlyConfigVersion\.cmake$",
            r"^lib/cmake/ramses-shared-lib-client-only-\d+\.\d+/ramses-shared-lib-client-onlyConfig\.cmake$",
        ]
    else:
        expectNonheaderFiles += [
            r"^bin/ramses-logic-viewer$",
            r"^bin/ramses-renderer-[\w-]+$",
            r"^lib/cmake/ramses-shared-lib-\d+\.\d+/ramses-shared-libConfigVersion\.cmake$",
            r"^lib/cmake/ramses-shared-lib-\d+\.\d+/ramses-shared-libConfig\.cmake$",
        ]

    installPath = Path(args.install_dir)
    includePath = installPath / "include"

    installedHeaders = []
    unexpectedFiles = []

    for path in installPath.rglob("*"):
        if path.is_dir():
            continue

        relPathStr = str(path.relative_to(installPath))

        # Handle all cases, don't skip anything
        if re.match(r'^include/ramses-\d+', relPathStr):
            # Ignore include file, it belongs to RAMSES and is checked by RAMSES already
            pass
        elif re.match(r'^include/ramses-logic/', relPathStr):
            # Ramses logic header file - add to special list to check compilation later
            installedHeaders.append(str(path.relative_to(includePath)))
        else:
            for f in expectNonheaderFiles:
                if re.match(f, relPathStr):
                    expectNonheaderFiles.remove(f)
                    break
            else:
                unexpectedFiles.append(relPathStr)

    print("Checking install non-header files")

    if args.ignore:
        # This is required because we use the more compatible 'append' option of argparse in favor of 'extend'
        patterns = [i[0] for i in args.ignore]
        print(f"Ignoring file patterns: {', '.join(patterns)}")
        unexpectedFiles = [f for f in unexpectedFiles if not any([re.search(p, f) for p in patterns])]
        expectNonheaderFiles = [f for f in expectNonheaderFiles if not any([re.search(p, f) for p in patterns])]

    # If all "expected" files were found, the list should be empty now - if not, report error
    if expectNonheaderFiles:
        print("Couldn't find some files in the install folder:\n  " + "\n  ".join(expectNonheaderFiles))
        return 1
    # Found a file that's not expected in the list? Error!
    if unexpectedFiles:
        print("Found following unexpected files in the install folder:\n  " + "\n  ".join(unexpectedFiles))
        return 1

    # Extract header files from the source tree
    srcIncludeDir = Path(args.src_dir) / 'include'
    srcApiHeaders = [str(f.relative_to(srcIncludeDir)) for f in srcIncludeDir.rglob("*") if f.suffix == '.h']

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
        temp_file = "/tmp/rlogic_pedantic_header.cpp"
        with open(temp_file, "w") as file:
            file.writelines(f"#include \"{h}\"\n\n")
            file.writelines("int main() {return 0;}")

        cmd = f'g++ -std=c++17 -Werror -pedantic -I"{str(includePath)}" "{temp_file}" -o /tmp/rlogic-pedantic-header.o'
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
