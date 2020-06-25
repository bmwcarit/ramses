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


class TestLocalAndRemoteScenes(test_classes.MultipleConnectionsTest):
    def get_nr_targets(self):
        """
        this tests needs two connections
        first connection: 'sender' has two scenes, distributes one and shows the other locally
        second connection: 'receiver' has a local scene, and additionally shows incoming remote scene
        """
        return 2

    @with_ramses_process_check
    def impl_setUp(self):
        applicationName = "ramses-local-client-test-{}".format(self.targets[0].defaultPlatform)
        self.sender = self.targets[0].start_renderer(applicationName, nameExtension='sender')
        self.checkThatApplicationWasStarted(self.sender)
        self.addCleanup(self.targets[0].kill_application, self.sender)

        applicationName = "ramses-local-client-test-{}".format(self.targets[1].defaultPlatform)
        self.receiver = self.targets[1].start_renderer(applicationName, args='--wayland-socket-embedded wayland-13 -tn 2', nameExtension='receiver', ramsesDaemonTarget=self.targets[0])
        self.checkThatApplicationWasStarted(self.receiver)
        self.addCleanup(self.targets[1].kill_application, self.receiver)

        self.ramsesDaemon = self.targets[0].start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.targets[0].kill_application, self.ramsesDaemon)

    def impl_tearDown(self):
        self.targets[0].kill_application(self.sender)
        self.targets[0].kill_application(self.ramsesDaemon)
        self.targets[1].kill_application(self.receiver)
        log.info("all applications killed")

        self.save_application_output_on_target(self.sender, self.targets[0])
        self.save_application_output_on_target(self.ramsesDaemon, self.targets[0])
        self.save_application_output_on_target(self.receiver, self.targets[1])
        log.info("output saved")

    def impl_test(self):
        self.receiver.showScene(12)
        self.receiver.showScene(42)
        self.receiver.send_ramsh_command("linkData -providerSceneId 12 -providerId 52 -consumerSceneId 42 -consumerId 54", waitForRendererConfirmation=True)
        self.receiver.showScene(67)
        self.receiver.send_ramsh_command("linkData -providerSceneId 12 -providerId 53 -consumerSceneId 67 -consumerId 54", waitForRendererConfirmation=True)

        self.sender.showScene(42)

        self.validateScreenshotOnTarget(self.sender, imageName="testClient_triangleRed.png", target=self.targets[0])
        self.validateScreenshotOnTarget(self.receiver, imageName="testClient_triangleRedAndBlue.png", target=self.targets[1])
