#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework.targets.remote_target import RemoteTarget
from ramses_test_framework import log

class WindowsCygwinTarget(RemoteTarget):

    def setup(self, transfer_binaries=True):
        self.defaultPlatform = "windows-wgl-es-3-0"
        self.defaultEnvironment = {"PATH" : "${HOME}/" + self.ramsesInstallDir + "/bin:${PATH}"}

        # base implementation that includes setup of connection and transfer of binaries
        baseSetupSuccessful = RemoteTarget.setup(self, transfer_binaries)

        if not baseSetupSuccessful:
            return False

        # must set dll executable to prevent error when running executables using these dlls. cmake/cpack
        # is not able to do this on windows. without you get following errors:
        # return code 127 (file not found) on cygwin. error code 0xc000022 in gui mode
        self.execute_on_target("cd '" + self.ramsesInstallDir + "/bin/'; chmod +x *.dll")

        # Path conversion to windows-style (/c/folder -> C:/folder)
        # Conversion is needed because screenshopts in RAMSES are using Capu files, which use Windows APIs directly
        # and require windows-style paths
        stdout, stderr, returncode = self.execute_on_target("cygpath -am ${HOME}")
        if returncode == 0:
            self.tmpDir = stdout[0].strip()
        else:
            log.error("Error getting tmp directory: returncode {} stdout: {} stderr: {}".format(returncode, stdout, stderr))

        return True

    def get_install_dir(self):
        return "${HOME}/{}".format(self.ramsesInstallDir)

    def _shutdown(self):
        self.execute_on_target("shutdown -t 3 -s", False)

    def kill_application(self, application):
        self.execute_on_target("taskkill -F -T -IM {0}.exe ".format(application.name))
        application.started = False
        application.stop_readers()

    def _get_full_process_list(self):
        (out, _, retCode) = self.execute_on_target("tasklist")
        assert retCode == 0, "_get_full_process_list failed"
        return out
