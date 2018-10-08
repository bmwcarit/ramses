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


class TestRendererTwoTargets(test_classes.MultipleConnectionsTest):

    def get_nr_targets(self):
        """
        this tests needs two connections
        first connection: test-client, daemon and renderer
        second connection: renderer
        """
        return 2

    @with_ramses_process_check
    def impl_setUp(self):
        self.ramsesDaemon = self.targets[0].start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.targets[0].kill_application, self.ramsesDaemon)
        self.renderer = self.targets[0].start_default_renderer(ramsesDaemonTarget=self.targets[0], args="-nomap")
        self.checkThatApplicationWasStarted(self.renderer)
        self.addCleanup(self.targets[0].kill_application, self.renderer)
        self.renderer2 = self.targets[1].start_default_renderer(ramsesDaemonTarget=self.targets[0], args="-nomap")
        self.checkThatApplicationWasStarted(self.renderer2)
        self.addCleanup(self.targets[1].kill_application, self.renderer2)
        self.testClient = self.targets[0].start_client("ramses-test-client", "-tn 5 -ts 0 -cz 5", ramsesDaemonTarget=self.targets[0])
        self.checkThatApplicationWasStarted(self.testClient)
        self.addCleanup(self.targets[0].kill_application, self.testClient)

    def impl_tearDown(self):
        self.targets[0].kill_application(self.testClient)
        self.targets[1].kill_application(self.renderer2)
        self.targets[0].kill_application(self.renderer)
        self.targets[0].kill_application(self.ramsesDaemon)
        log.info("all applications killed")
        self.save_application_output_on_target(self.testClient, self.targets[0])
        self.save_application_output_on_target(self.renderer2, self.targets[1])
        self.save_application_output_on_target(self.renderer, self.targets[0])
        self.save_application_output_on_target(self.ramsesDaemon, self.targets[0])
        log.info("output saved")

    def impl_test(self):
        log.info("making screenshot on target 0")
        self.renderer.showScene(26)
        self.validateScreenshotOnTarget(self.renderer, "testClient_threeTriangles.png", self.targets[0])

        log.info("making screenshot on target 1")
        self.renderer2.showScene(26)
        self.validateScreenshotOnTarget(self.renderer2, "testClient_threeTriangles.png", self.targets[1])
