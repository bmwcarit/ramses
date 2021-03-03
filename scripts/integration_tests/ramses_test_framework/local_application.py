#  -------------------------------------------------------------------------
#  Copyright (C) 2016 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework.application import Application


class LocalApplication(Application):
    def __init__(self, popenApp, name, workingDirectoryAbsolute, nameExtension=""):
        if popenApp:
            stdin = popenApp.stdin
            stdout = popenApp.stdout
            stderr = popenApp.stderr
        else:
            stdin = None
            stdout = None
            stderr = None
        Application.__init__(self, stdin, stdout, stderr, name, workingDirectoryAbsolute, nameExtension)
        self.popenApp = popenApp

    def get_return_code_blocking(self):
        self.popenApp.wait()
        self.started = False
        self.stop_readers()
        return self.popenApp.returncode
