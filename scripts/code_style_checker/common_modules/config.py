
#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

"""
Contains configurations constants

"""
G_WARNING_COUNT = 0

G_PROP_FILES = [
    'CHANGELOG.txt',
    'README.txt',
    'proprietary',
    'scripts/integration_tests/proprietary',
    'scripts/integration_tests/run_smoke_tests_as_bat_tests.py',
    '.gitignore',
    'zuul.d'
]

G_LICENSE_TEMPLATES_OPEN = ["""
  -------------------------------------------------------------------------
  Copyright (C) [YYYY] BMW Car IT GmbH
  -------------------------------------------------------------------------
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at https://mozilla.org/MPL/2.0/.
  -------------------------------------------------------------------------
"""
]
G_LICENSE_TEMPLATES_PROP = ["""
  -------------------------------------------------------------------------
  Copyright (C) [YYYY] BMW Car IT GmbH
  All rights reserved.
  -------------------------------------------------------------------------
  This document contains proprietary information belonging to BMW Car IT.
  Passing on and copying of this document, use and communication of its
  contents is not permitted without prior written authorization.
  -------------------------------------------------------------------------
"""
]
