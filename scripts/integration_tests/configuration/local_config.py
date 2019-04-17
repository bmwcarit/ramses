#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import platform

from ramses_test_framework.targets.local_target import LocalTarget
from ramses_test_framework.targets.targetInfo import TargetInfo
from common_config import Config

class LocalConfig(Config):
    def __init__(self):
        Config.__init__(self)
        #Default system settings

        if platform.system() == "Windows":
            self.defaultPlatform = "windows-wgl-4-5"
        else:
            self.defaultPlatform = "x11-egl-es-3-0"

        self.localTarget = TargetInfo(LocalTarget, "local-target", "", "", "")
        self.allTargetsList = [self.localTarget]
        self.defaultTestTargetsList = self.allTargetsList
