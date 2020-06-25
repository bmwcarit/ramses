#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework import test_classes
from ramses_test_framework import log
from ramses_test_framework.ramses_test_extensions import with_ramses_process_check


class TestClass(test_classes.OnAllDefaultTargetsTest):

    @with_ramses_process_check
    def impl_setUp(self):
        self.ramsesDaemon = self.target.start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.target.kill_application, self.ramsesDaemon)
        self.renderer = self.target.start_default_renderer()
        self.checkThatApplicationWasStarted(self.renderer)
        self.addCleanup(self.target.kill_application, self.renderer)
        self.testClient1 = self.target.start_client("ramses-test-client", "-tn 5 -ts 0 -cx 2 -cz 5", nameExtension='1')
        self.checkThatApplicationWasStarted(self.testClient1)
        self.addCleanup(self.target.kill_application, self.testClient1)
        self.testClient2 = self.target.start_client("ramses-test-client", "-tn 4 -ts 0 -cx -2 -cz 5", nameExtension='2')
        self.checkThatApplicationWasStarted(self.testClient2)
        self.addCleanup(self.target.kill_application, self.testClient2)

    def impl_tearDown(self):
        self.target.kill_application(self.testClient2)
        self.target.kill_application(self.testClient1)
        self.target.kill_application(self.renderer)
        self.target.kill_application(self.ramsesDaemon)
        log.info("all applications killed")
        self.save_application_output(self.testClient2)
        self.save_application_output(self.testClient1)
        self.save_application_output(self.renderer)
        self.save_application_output(self.ramsesDaemon)
        log.info("output saved")

    def impl_test(self):
        self.renderer.showScene(25)
        self.renderer.showScene(26)

        self.validateScreenshot(self.renderer, "testClient_multipleClients.png")
