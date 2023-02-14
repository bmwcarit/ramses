#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework import test_classes
from ramses_test_framework import log
from ramses_test_framework.ramses_test_extensions import with_ramses_process_check
from ramses_test_framework.targets.target import DEFAULT_TEST_SURFACE


class TestRendererWithDstRect(test_classes.OnSelectedTargetsTest):

    @with_ramses_process_check
    def impl_setUp(self):
        if not self.target.systemCompositorScreenshotSupported:
            self.skipTest("System compositor controller screenshot support is not configured for this target")

        # Start black background renderer
        self.rendererbackground = self.target.start_default_renderer("--ivi-surface {}".format(DEFAULT_TEST_SURFACE + 1))
        self.checkThatApplicationWasStarted(self.rendererbackground)
        self.addCleanup(self.target.kill_application, self.rendererbackground)
        self.rendererbackground.send_ramsh_command("skipUnmodifiedBuffers 0", waitForRendererConfirmation=True)

        # Start renderer with non-default dest rectangle
        self.renderer = self.target.start_default_renderer("--xpos 100 --ypos 70 --width 150 --height 200 --clear 0.5 0.0 0.5 1.0")
        self.checkThatApplicationWasStarted(self.renderer)
        self.addCleanup(self.target.kill_application, self.renderer)

        self.testClient = self.target.start_client("ramses-test-client", "-tn 22 -ts 0 -cz 5", nameExtension='1')
        self.checkThatApplicationWasStarted(self.testClient)
        self.addCleanup(self.target.kill_application, self.testClient)

        self.ramsesDaemon = self.target.start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.target.kill_application, self.ramsesDaemon)

    def impl_tearDown(self):
        self.target.kill_application(self.testClient)
        self.target.kill_application(self.ramsesDaemon)
        self.target.kill_application(self.renderer)
        self.target.kill_application(self.rendererbackground)
        log.info("all applications killed")

        self.save_application_output(self.testClient)
        self.save_application_output(self.ramsesDaemon)
        self.save_application_output(self.renderer)
        log.info("output saved")

    def impl_test(self):
        self.renderer.showScene(34)
        self.validateScreenshot(self.renderer, "renderer_with_dest_rectangle.png", useSystemCompositorForScreenshot=True)
