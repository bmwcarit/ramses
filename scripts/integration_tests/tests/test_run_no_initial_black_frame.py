#  -------------------------------------------------------------------------
#  Copyright (C) 2017 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------
import time

from ramses_test_framework import test_classes
from ramses_test_framework import log
from ramses_test_framework import helper
from ramses_test_framework.ramses_test_extensions import with_ramses_process_check
from ramses_test_framework.targets.target import DEFAULT_TEST_LAYER
from ramses_test_framework import application

# The test start a RAMSES renderer, those surface is initially invisible. The test application then
# creates and shows a scene with a red triangle. After the sceneShown event is received from the renderer
# it switches the surface to visible by using the system compositor controller. The test application does
# not do any frame rendering anymore after the sceneShown event has been received, so this proves that
# the scene content has already been rendered before the surface was made visible, and that hence
# no intermediate black frame can be visible on the screen.
class TestNoInitialBlackFrame(test_classes.OnSelectedTargetsTest):

    @with_ramses_process_check
    def impl_setUp(self):
        self.percentageOfRGBDifferenceAllowedPerPixel = 0.008  # allows +/- 2 for rgb values (needed e.g. for ufo driver)

        self.testLayer = DEFAULT_TEST_LAYER + 1

        # Start black background renderer
        self.rendererbackground = self.target.start_default_renderer("--wayland-socket-embedded wayland-12 --disableAutoMapping")
        self.checkThatApplicationWasStarted(self.rendererbackground)
        self.addCleanup(self.target.kill_application, self.rendererbackground)

        applicationName = "ramses-local-client-test-{}".format(self.target.defaultPlatform)
        self.application = self.target.start_renderer(applicationName, "-sid 101 -tn 4")

        self.checkThatApplicationWasStarted(self.application)
        self.addCleanup(self.target.kill_application, self.application)

    def impl_tearDown(self):
        self.target.kill_application(self.application)
        log.info("all applications killed")

        self.save_application_output(self.application)
        log.info("output saved")

    def impl_test(self):
        surfaceMadeVisible = self.application.wait_for_msg_in_stdout_from_beginning("Surface should be still invisible", timeout=application.Application.DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT)

        self.validateScreenshot(self.application, "black_rgb.png", useSystemCompositorForScreenshot=True)

        self.assertTrue(self.application.send_ramsh_command("step 1", response_message="Surface with scene should now be visible", timeout=application.Application.DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT))

        self.validateScreenshot(self.application, "red_triangle_on_blue_background.png", useSystemCompositorForScreenshot=True)
