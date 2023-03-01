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
        # Start client for the 1st time and show scene on the renderer
        self.testClient1 = self.target.start_client("ramses-test-client", "--test-nr 5 --test-state 0 --cx -2 --cz 5", nameExtension='1st')
        self.checkThatApplicationWasStarted(self.testClient1)
        self.renderer.showScene(26)
        self.validateScreenshot(self.renderer, "testClient_disconnectedClient.png")

        # Kill client and wait for the scene being unpublished on the renderer
        watchId = self.renderer.start_watch_stdout()
        self.target.kill_application(self.testClient1)
        self.renderer.wait_for_msg_in_stdout(watchId, "Scene 26 is in state UNPUBLISHED, scene was in state Rendered", 30)
        self.validateScreenshot(self.renderer, "black.png")

        # Start client for the 2nd time and show scene on the renderer
        watchId = self.renderer.start_watch_stdout()
        self.testClient2 = self.target.start_client("ramses-test-client", "--test-nr 5 --test-state 0 --cx -2 --cz 5", nameExtension='2nd')
        self.checkThatApplicationWasStarted(self.testClient2)
        self.addCleanup(self.target.kill_application, self.testClient2)
        self.renderer.showScene(26)  # must explicitly trigger new ramp up to rendered
        self.renderer.wait_for_msg_in_stdout(watchId, "Scene 26 is in state RENDERED caused by command SHOW", 30)
        self.validateScreenshot(self.renderer, "testClient_disconnectedClient.png")
