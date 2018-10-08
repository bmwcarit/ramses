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
        self.ramsesDaemon = self.target.start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.target.kill_application, self.ramsesDaemon)
        self.renderer = self.target.start_default_renderer(args="-nomap")
        self.checkThatApplicationWasStarted(self.renderer)
        self.addCleanup(self.target.kill_application, self.renderer)
        self.testClient = self.target.start_client("ramses-test-client", "-tn 3 -ts 0 -cz 5")
        self.checkThatApplicationWasStarted(self.testClient)
        self.addCleanup(self.target.kill_application, self.testClient)

    def impl_tearDown(self):
        self.target.kill_application(self.testClient)
        self.target.kill_application(self.renderer)
        self.target.kill_application(self.ramsesDaemon)
        log.info("all applications killed")
        self.save_application_output(self.testClient)
        self.save_application_output(self.renderer)
        self.save_application_output(self.ramsesDaemon)
        log.info("output saved")

    def impl_test(self):
        self.renderer.showScene(23, blockUntilShown=False)
        self.renderer.showScene(24)
        self.validateScreenshot(self.renderer, "testClient_destroyedScene.png")
