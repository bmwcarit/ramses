#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2021 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


import sys
import os
import subprocess
from pathlib import Path


def main(args):
    """Wraps the ramses spdx generator script and overrides paths and config for the logic engine"""

    # Forge logic-specific paths and file locations
    sdkroot = Path(subprocess.check_output(
        ['git', 'rev-parse', '--show-toplevel'], shell=False, cwd=os.path.dirname(os.path.realpath(__file__))).decode('utf-8').strip())
    license_script = sdkroot / 'external/ramses/proprietary/scripts/license-tool/license-tool.py'

    wrapped_command = [sys.executable,
                       str(license_script),
                       '--spdx-config', str(sdkroot / 'ci/scripts/proprietary/spdx/config.py'),
                       '--spdx-sdkroot', str(sdkroot),
                       '--spdx-output-dir', str(sdkroot / 'ci/scripts/proprietary/spdx/generated/'),
                       *args]

    subprocess.check_call(wrapped_command, cwd=sdkroot)

    return 0


if __name__ == '__main__':
    # Forward args to wrapped ramses script
    exit(main(sys.argv[1:]))
