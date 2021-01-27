#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from __future__ import print_function
import re
from future.utils import iteritems
from configuration.local_config import Config

class BaseRemoteConfig(Config):
    def __init__(self):
        Config.__init__(self)

        # Paths on target
        self.ramsesInstallDir = 'ramses/install'

        # Other settings
        self.sshConnectionNrAttempts = 180
        self.sshConnectionTimeoutPerAttempt = 1  # (seconds)
        self.sshConnectionSleepPerAttempt = 1  # (seconds)
        self.powerNrAttempts = 3

        # To describe the test targets TargetInfo objects have to be created
        # e.g. self.my_linux_target = TargetInfo(Linux_X11_SeparateXServer, "My-Linux-target", "192.168.1.234",
        #                                         "user", "RAMSES_SDK_MASTER_L32_GCC_R", privateKey="path/to/private_key")
        # add this TargetInfo objects the to the lists below,
        # e.g. self.allTargetsList = [self.my_linux_target] and self.defaultTestTargetsList = self.allTargetsList

        # list of all targets that should be started
        self.allTargetsList = []
        # list of all targets on which tests of type OnAllDefaultTargetsTest should be executed
        self.defaultTestTargetsList = []

        #configuration for OnSelectedTargetsTest and MultipleConnectionsTest test instaces
        #format:  Dict(Targetname ->  List( List(TargetInfo) )
        #the outer list defines the number of tests instanciated from this test class
        #the inner list contains all targets used to instanciate one test instance (i.e. length = 1 for a
        # OnSelectedTargetsTest, for a MultipleConnectionsTest length = get_nr_targets() of the test)
        self.testToTargetConfig = {}

    def filterTargets(self, filter_str):
        # filter target list
        filter_re = re.compile(filter_str, re.IGNORECASE)
        self.allTargetsList = [t for t in self.allTargetsList if filter_re.search(t.name)]
        self.defaultTestTargetsList = [t for t in self.defaultTestTargetsList if filter_re.search(t.name)]

        # filter testTotargetConfig
        filtered_testToTargetConfig = {}
        all_targets_set = set(self.allTargetsList)
        for target, configs in iteritems(self.testToTargetConfig):
            filtered_configs = [c for c in configs if len(set(c) - all_targets_set) == 0]
            if len(filtered_configs) > 0:
                filtered_testToTargetConfig[target] = filtered_configs
        self.testToTargetConfig = filtered_testToTargetConfig

        print("Filtered list of targets:", [t.name for t in self.allTargetsList])
        print("Filtered list of default test targets:", [t.name for t in self.defaultTestTargetsList])
