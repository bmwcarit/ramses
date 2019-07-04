
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
    'proprietary',
    'scripts/integration_tests/proprietary',
    'scripts/integration_tests/run_smoke_tests_as_bat_tests.py',
    'zuul.d'
]

G_LICENSE_TEMPLATE_OPEN = """
  -------------------------------------------------------------------------
  Copyright (C) [YYYY] BMW XXXX
  -------------------------------------------------------------------------
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at https://mozilla.org/MPL/2.0/.
  -------------------------------------------------------------------------
"""

G_LICENSE_TEMPLATE_PROP = """
  -------------------------------------------------------------------------
  Copyright (C) [YYYY] BMW XXXX
  All rights reserved.
  -------------------------------------------------------------------------
  This document contains proprietary information belonging to BMW XXXX.
  Passing on and copying of this document, use and communication of its
  contents is not permitted without prior written authorization.
  -------------------------------------------------------------------------
"""
