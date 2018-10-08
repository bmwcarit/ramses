#  -------------------------------------------------------------------------
#  Copyright (C) 2016 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import time

from ramses_test_framework import test_classes
from ramses_test_framework import log
from ramses_test_framework.ramses_test_extensions import with_ramses_process_check


class ListUseCaseTest(test_classes.MultipleConnectionsTest):
    def get_nr_targets(self):
        """
        this tests needs two connections
        first connection: 'HU' has two scenes, distributes one and shows the other locally
        second connection: 'KOMBI' has a local scene, and additionally shows incoming remote scene
        """
        return 2

    @with_ramses_process_check
    def impl_setUp(self):
        applicationName = "ramses-local-client-test-{}".format(self.targets[0].defaultPlatform)
        self.HU_HMI = self.targets[0].start_renderer(applicationName, args="-nomap")
        self.checkThatApplicationWasStarted(self.HU_HMI)
        self.addCleanup(self.targets[0].kill_application, self.HU_HMI)

        applicationName = "ramses-local-client-test-{}".format(self.targets[1].defaultPlatform)
        self.KOMBI = self.targets[1].start_renderer(applicationName, args='--wayland-socket-embedded wayland-13 -tn 2 -nomap',  ramsesDaemonTarget=self.targets[0])
        self.checkThatApplicationWasStarted(self.KOMBI)
        self.addCleanup(self.targets[1].kill_application, self.KOMBI)

        self.ramsesDaemon = self.targets[0].start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.targets[0].kill_application, self.ramsesDaemon)

    def impl_tearDown(self):
        self.targets[0].kill_application(self.HU_HMI)
        self.targets[0].kill_application(self.ramsesDaemon)
        self.targets[1].kill_application(self.KOMBI)
        log.info("all applications killed")

        self.save_application_output_on_target(self.HU_HMI, self.targets[0], nr=1)
        self.save_application_output_on_target(self.ramsesDaemon, self.targets[0])
        self.save_application_output_on_target(self.KOMBI, self.targets[1], nr=2)
        log.info("output saved")

    def impl_test(self):
        self.KOMBI.showScene(12)
        self.KOMBI.showScene(42)
        self.KOMBI.send_ramsh_command("linkData -providerSceneId 12 -providerId 52 -consumerSceneId 42 -consumerId 54", waitForRendererConfirmation=True)
        self.KOMBI.showScene(67)
        self.KOMBI.send_ramsh_command("linkData -providerSceneId 12 -providerId 53 -consumerSceneId 67 -consumerId 54", waitForRendererConfirmation=True)

        self.HU_HMI.showScene(42)

        self.validateScreenshotOnTarget(self.HU_HMI, imageName="testClient_triangleRed.png", target=self.targets[0])
        self.validateScreenshotOnTarget(self.KOMBI, imageName="testClient_triangleRedAndBlue.png", target=self.targets[1])
