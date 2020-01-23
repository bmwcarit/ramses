#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import time

from ramses_test_framework import test_classes
from ramses_test_framework import log
from ramses_test_framework.ramses_test_extensions import with_ramses_process_check


class TestClass(test_classes.OnAllDefaultTargetsTest):

    @with_ramses_process_check
    def impl_setUp(self):
        applicationName = "ramses-local-client-test-{}".format(self.target.defaultPlatform)
        self.app = self.target.start_renderer(applicationName, dltAppID="LCLT")
        self.checkThatApplicationWasStarted(self.app)
        self.addCleanup(self.target.kill_application, self.app)

    def impl_tearDown(self):
        self.target.kill_application(self.app)
        log.info("all applications killed")
        self.save_application_output(self.app)
        log.info("output saved")

    def impl_test(self):
        self.app.showScene(42)
        self.validateScreenshot(self.app, "testClient_triangleRed.png")
