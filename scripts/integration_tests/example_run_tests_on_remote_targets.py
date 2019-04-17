#!/usr/bin/env python
#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework.cores import core
from configuration.base_remote_config import BaseRemoteConfig
from ramses_test_framework import log

print("NOTE: To make this script do something useful you have to first create a subclass of BaseRemoteConfig and add your "
      "target definitions there, then reference this configuration below")

_core = core.Core(core.REMOTE, BaseRemoteConfig())

_core.read_arguments()
if _core.setup():
    #tests should only run if setup was successful
    result = _core.run_tests()
    _core.tear_down(shutdownTargets=False)
    if not result:
        exit(1)
else:
    log.info("Setup failed: Aborting tests")
    exit(2)
