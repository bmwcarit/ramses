#  -------------------------------------------------------------------------
#  Copyright (C) 2015 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


from builtins import object
from builtins import range
LOCAL, REMOTE, REMOTE_INTERNAL = list(range(3))

class Core(object):
    def __init__(self, typ, config):
        if typ == LOCAL:
            from ramses_test_framework.cores.core_impl import LocalCoreImpl
            self.coreImpl = LocalCoreImpl(config)
        elif typ == REMOTE:
            from ramses_test_framework.cores.remote_core_impl import RemoteCoreImpl
            self.coreImpl = RemoteCoreImpl(config)
        elif typ == REMOTE_INTERNAL:
            self.coreImpl = RemoteCoreInternalImpl(config)

    def read_arguments(self):
        self.coreImpl.read_arguments()

    def setup(self, resultsDir=None, transfer_binaries=True):
        if resultsDir is None:
            resultsDir = self.coreImpl.config.testResultsDir
        return self.coreImpl.setup(resultsDir, transfer_binaries)

    def run_tests(self, testDirs=None):
        if testDirs is None:
            testDirs = self.coreImpl.config.testDirs
        return self.coreImpl.run_tests(testDirs)

    def tear_down(self, shutdownTargets=True):
        self.coreImpl.tear_down(shutdownTargets)

