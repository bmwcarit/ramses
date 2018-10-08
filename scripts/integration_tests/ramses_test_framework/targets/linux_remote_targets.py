#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import time

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


class Linux_X11_SeparateXServer(Linux_X11):

    def setup(self, transfer_binaries=True):
        # base implementation that includes setup of connection and transfer of binaries
        baseSetupSuccessful = Linux_X11.setup(self, transfer_binaries)
        if not baseSetupSuccessful:
            return False

        #check if second X-Server is already running (first X-Server might be blocked by login screen)
        _, _, returnCode =  self.execute_on_target("test -e /tmp/.X1-lock")
        #start is if not already running
        if returnCode != 0:
            print("starting x server")
            time.sleep(5)
            self.execute_on_target("startx", block=False)
            time.sleep(5)

        self.defaultEnvironment["DISPLAY"] = ":1"

        return True

    def _shutdown(self):
        self.execute_on_target("sudo shutdown -h 0", False)
