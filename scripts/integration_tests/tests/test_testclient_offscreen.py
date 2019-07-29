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
from ramses_test_framework.ramses_test_extensions import with_ramses_process_check
from ramses_test_framework.targets.target import DEFAULT_TEST_LAYER

##
## In this test we have to check that we can render an image without
## showing anything on the screen.
##
## So the strategy is to create some neutral background, which is clearly
## visible, and then render an image while checking that nothing overwrites
## the reference background
##
## As neutral background we create a renderer that shows a fullscreen green
## background. After that we create our test renderer and map the test scene.
## As proof that it actually runs we create a renderer screenshot of the scene
## and also create another screenshot of the whole screen.
## This second screenshot of the whole screen is expected to show the same image
## as in the beginning: a blank screen that is completely green.
## While the renderer image is expected to show the scene.
##

from ramses_test_framework.targets.target import DEFAULT_TEST_SURFACE

class TestOffscreenRenderer(test_classes.OnSelectedTargetsTest):


    @with_ramses_process_check
    def impl_setUp(self):

        # Map of created test surfaces and surface ivi-ids
        # Ensure, that every ivi-id is listed here, for which a surface is created in this test
        self.testSurfaceIVIIds = {"rendererGreenBackground": DEFAULT_TEST_SURFACE,
                                  "offscreenRenderer":       DEFAULT_TEST_SURFACE + 1}

        self.ramsesDaemon = self.target.start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.target.kill_application, self.ramsesDaemon)
        # rendererGreenBackground is only there to create some neutral background (hence the name)
        self.rendererGreenBackground = self.target.start_default_renderer(nameExtension='display', args="-nomap -w 1440 -h 540 --waylandIviSurfaceID {0}".format(self.testSurfaceIVIIds["rendererGreenBackground"]))
        self.checkThatApplicationWasStarted(self.rendererGreenBackground)
        self.addCleanup(self.target.kill_application, self.rendererGreenBackground)
        # Using test scene MultipleTrianglesScene (tn 5) in state THREE_TRIANGLES (ts 0)
        # the ramses-test-client creates this scene with ID 26
        self.testClient = self.target.start_client("ramses-test-client", "-tn 5 -ts 0 -cz 5")
        self.checkThatApplicationWasStarted(self.testClient)
        self.addCleanup(self.target.kill_application, self.testClient)

    def impl_tearDown(self):
        self.target.kill_application(self.testClient)
        self.target.kill_application(self.rendererGreenBackground)
        self.target.kill_application(self.ramsesDaemon)
        log.info("all applications killed")
        self.save_application_output(self.testClient)
        self.save_application_output(self.rendererGreenBackground)
        self.save_application_output(self.offscreenRenderer)
        self.save_application_output(self.ramsesDaemon)
        log.info("output saved")


    def impl_test(self):

        # call ramsh command to set background to green color
        # and also check that it worked
        self.rendererGreenBackground.send_ramsh_command("clc 0 0 100 0 255", waitForRendererConfirmation=True)
        self.validateScreenshot(self.rendererGreenBackground, "testClient_offscreen_background.png", useSystemCompositorForScreenshot=True)

        # now we start the actual renderer we want to test
        # important is the first argument "-off", which tells the renderer to use
        # offscreen rendering
        # Using a new surface 101 also we need to set a specific filename
        # for the embedded socket to avoid collisions with
        # test framework default setting of embedded socket name...
        self.offscreenRenderer = self.target.start_default_renderer(nameExtension='offscreen', args="-off -nomap --waylandIviSurfaceID {0} --wayland-socket-embedded wayland-123".format(self.testSurfaceIVIIds["offscreenRenderer"]))
        self.checkThatApplicationWasStarted(self.offscreenRenderer)
        self.addCleanup(self.target.kill_application, self.offscreenRenderer)
        # showing scene 26, i.e. MultipleTriangleScene::THREE_TRIANGLES from the ramses-test-client
        self.offscreenRenderer.showScene(26)

        # Ensure that offscreen renderer is before rendererGreenBackground
        self.target.ivi_control.setLayerRenderorder(DEFAULT_TEST_LAYER, "{0} {1}".format(self.testSurfaceIVIIds["rendererGreenBackground"], self.testSurfaceIVIIds["offscreenRenderer"]))
        self.target.ivi_control.flush()


        # check that the rendererer is rendering the test scene...
        self.validateScreenshot(self.offscreenRenderer, "testClient_threeTriangles.png")
        # ...while the screen is still completely green, i.e. not showing the test scene
        self.validateScreenshot(self.rendererGreenBackground, "testClient_offscreen_background.png", useSystemCompositorForScreenshot=True)

        self.target.kill_application(self.offscreenRenderer)
