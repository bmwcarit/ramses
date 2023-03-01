#  -------------------------------------------------------------------------
#  Copyright (C) 2016 BMW Car IT GmbH
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
        self.testClient = self.target.start_client("ramses-test-client", "--test-nr 15 --test-state 0 --cz 8")
        self.checkThatApplicationWasStarted(self.testClient)
        self.addCleanup(self.target.kill_application, self.testClient)
        self.percentageOfWrongPixelsAllowed = 0.01
        self.percentageOfRGBDifferenceAllowedPerPixel = 0.01

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
        self.renderer.showScene(39)
        self.renderer.showScene(40)
        self.validateScreenshot(self.renderer, "testClient_dataLink_noLinks.png")

        self.renderer.send_ramsh_command("linkData -providerSceneId 39 -providerId 1251 -consumerSceneId 40 -consumerId 1252",
                                         waitForRendererConfirmation=True)
        self.renderer.send_ramsh_command("linkData -providerSceneId 39 -providerId 31251 -consumerSceneId 40 -consumerId 31252",
                                         waitForRendererConfirmation=True)
        self.renderer.send_ramsh_command("linkData -providerSceneId 40 -providerId 121 -consumerSceneId 39 -consumerId 122",
                                         waitForRendererConfirmation=True)

        self.validateScreenshot(self.renderer, "testClient_dataLink_linked.png")
