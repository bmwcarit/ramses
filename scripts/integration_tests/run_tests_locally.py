#!/usr/bin/env python
#  -------------------------------------------------------------------------
#  Copyright (C) 2015 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework.cores import core
from configuration.local_config import LocalConfig

_core = core.Core(core.LOCAL, LocalConfig())
_core.read_arguments()
_core.setup()
result = _core.run_tests()
_core.tear_down(shutdownTargets=False)
if not result:
    exit(1)
