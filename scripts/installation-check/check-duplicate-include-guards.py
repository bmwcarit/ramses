#!/usr/bin/env python

#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import os
import re
import sys

def main():
    sdkroot = os.path.realpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", ".."))

    print('Check for duplicate include guards')

    guard = re.compile(r'#define (RAMSES_\w+_H)')
    matches = {}
    for dirpath, _, files in os.walk(sdkroot):
        relpath = os.path.relpath(dirpath, sdkroot)
        if not relpath.startswith('.'):
            for fn in files:
                # Exclude patch files - they will always emit duplicate header guards
                if not fn.endswith('.patch'):
                    full_path = os.path.join(dirpath, fn)
                    with open(full_path, 'r') as f:
                        m = guard.search(f.read())
                        if m:
                            g = m.group(1)
                            m = matches.get(g, [])
                            m.append(full_path)
                            matches[g] = m

    dups = []
    for k, v in matches.iteritems():
        if len(v) > 1 and not k in ['RAMSES_SDKBUILDCONFIG_H', 'RAMSES_CAPU_CONFIG_H']:
            dups.append(k)

    if dups:
        print("Error: Found duplicate include guard(s):")
        for d in dups:
            print("\tFound {0} occurances of header guard \"{1}\" in files:".format(len(matches[d]), d))
            for f in matches[d]:
                print("\t\t{0}".format(f))
        return 1
    else:
        print("done")
        return 0

sys.exit(main())
