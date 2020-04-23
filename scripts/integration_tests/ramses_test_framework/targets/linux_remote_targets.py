#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework.targets.remote_target import RemoteTarget


class Linux_X11(RemoteTarget):

    def setup(self, transfer_binaries=True):
        self.defaultPlatform = "x11-egl-es-3-0"
        libPath = "~/{}/lib".format(self.ramsesInstallDir)
        self.defaultEnvironment = {"DISPLAY": ":0", "LD_LIBRARY_PATH": libPath}

        # base implementation that includes setup of connection and transfer of binaries
        return RemoteTarget.setup(self, transfer_binaries)

    def _shutdown(self):
        self.execute_on_target("sudo shutdown -h 0", False)
