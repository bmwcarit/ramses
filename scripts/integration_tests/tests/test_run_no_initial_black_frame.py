#  -------------------------------------------------------------------------
#  Copyright (C) 2017 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework import test_classes
from ramses_test_framework import log
from ramses_test_framework.ramses_test_extensions import with_ramses_process_check


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

        self.testSurface = 101

        # Start black background renderer
        self.rendererbackground = self.target.start_default_renderer("--wayland-socket-embedded wayland-12")
        self.checkThatApplicationWasStarted(self.rendererbackground)
        self.addCleanup(self.target.kill_application, self.rendererbackground)
        self.rendererbackground.send_ramsh_command("skipUnmodifiedBuffers 0", waitForRendererConfirmation=True)
        self.validateScreenshot(self.rendererbackground, "black_rgb.png", useSystemCompositorForScreenshot=False)

        applicationName = "ramses-local-client-test-{}".format(self.target.defaultPlatform)
        self.application = self.target.start_renderer(applicationName, args="-sid {} -tn 4".format(self.testSurface), automap=True)

        self.checkThatApplicationWasStarted(self.application)
        self.addCleanup(self.target.kill_application, self.application)

    def impl_tearDown(self):
        self.target.kill_application(self.application)
        log.info("all applications killed")

        self.save_application_output(self.application)
        log.info("output saved")

    def impl_test(self):
        assert self.application.wait_for_msg_in_stdout_from_beginning("Surface should be still invisible")

        self.validateScreenshot(self.application, "black_rgb.png", useSystemCompositorForScreenshot=True)

        # Wait for handle configuration message on test surface, because it indicates that a surface with the right buffer size
        # has been attached to the surface. This means that surface is already being rendered by system compositor.
        # It is more correct to wait for frame callback, but this is not possible because renderer runs in update only mode
        # (after rendering one and only one frame) so it does not poll for events on window connection socket to sys compositor, i.e., does
        # not handle events from sys compositor to window. Instead, renderer still runs SCC update in update only mode, which receives events
        # happening on SCC socket connection to sys compositor

        handleConfigurationMessage = "IVIControllerSurface::HandleConfigurationCallback ivi-surface:{} width: 1280 height: 480".format(self.testSurface)
        self.assertTrue(self.application.send_ramsh_command("step 1", response_message=handleConfigurationMessage))

        self.validateScreenshot(self.application, "red_triangle_on_blue_background.png", useSystemCompositorForScreenshot=True)
