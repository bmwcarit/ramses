#  -------------------------------------------------------------------------
#  Copyright (C) 2016 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------
import time, re

from ramses_test_framework import test_classes
from ramses_test_framework import log
from ramses_test_framework import helper
from ramses_test_framework.ramses_test_extensions import IVI_Control
from ramses_test_framework.targets.target import DEFAULT_TEST_LAYER
from ramses_test_framework.targets.target import DEFAULT_TEST_SURFACE
from ramses_test_framework.ramses_test_extensions import ensureSystemCompositorRoundTrip

# The test creates two different IVI layers on one screen
# side by side.
# Two renderes are started, each on its own layer
# Two scenes are created and mapped to the two renderes
# It is expected that the two rendered scenes appear
# side by side.
class TestMultipleIVILayers(test_classes.OnSelectedTargetsTest):

    def impl_setUp(self):
        self.percentageOfRGBDifferenceAllowedPerPixel = 0.004  #allows +/- 1 for rgb values (needed e.g. for ufo driver)

        if self.target.systemCompositorControllerSupported:
            firstLayerIviId = DEFAULT_TEST_LAYER
            secondLayerIviId = DEFAULT_TEST_LAYER + 1
            self.firstSurfaceIviId = DEFAULT_TEST_SURFACE
            self.secondSurfaceIviId = DEFAULT_TEST_SURFACE + 1

            self.target.ivi_control.createLayer(firstLayerIviId,640,480)
            self.target.ivi_control.createLayer(secondLayerIviId,640,480)
            self.target.ivi_control.setLayerVisibility(firstLayerIviId, visible=True)
            self.target.ivi_control.setLayerVisibility(secondLayerIviId, visible=True)
            self.target.ivi_control.setLayerDestRect(secondLayerIviId,640,0,640,480)
            self.target.ivi_control.appendLayerToScreenRenderorder(4, secondLayerIviId)
            self.target.ivi_control.appendLayerToScreenRenderorder(4, firstLayerIviId)
            self.target.ivi_control.flush()

            self.ramsesDaemon = self.target.start_daemon()
            self.checkThatApplicationWasStarted(self.ramsesDaemon)
            self.addCleanup(self.target.kill_application, self.ramsesDaemon)
            self.rendererLeft = self.target.start_default_renderer(nameExtension="left", args="--waylandIviLayerId {0} --waylandIviSurfaceID {1} --wayland-socket-embedded wayland-13 --disableAutoMapping -w 640".format(firstLayerIviId, self.firstSurfaceIviId))
            self.checkThatApplicationWasStarted(self.rendererLeft)
            self.addCleanup(self.target.kill_application, self.rendererLeft)
            self.rendererLeft.send_ramsh_command("skipUnmodifiedBuffers 0", waitForRendererConfirmation=True)
            self.rendererRight = self.target.start_default_renderer(nameExtension="right", args="--waylandIviLayerId {0} --waylandIviSurfaceID {1} --wayland-socket-embedded wayland-15 --disableAutoMapping -w 640".format(secondLayerIviId, self.secondSurfaceIviId))
            self.checkThatApplicationWasStarted(self.rendererRight)
            self.addCleanup(self.target.kill_application, self.rendererRight)
            self.rendererRight.send_ramsh_command("skipUnmodifiedBuffers 0", waitForRendererConfirmation=True)
            self.testClient3Triangles = self.target.start_client("ramses-test-client", "-tn 5 -ts 0 -cz 5")
            self.checkThatApplicationWasStarted(self.testClient3Triangles)
            self.addCleanup(self.target.kill_application, self.testClient3Triangles)
            self.testClientRedTriangles = self.target.start_client("ramses-test-client", "-tn 4 -ts 0 -cz 5")
            self.checkThatApplicationWasStarted(self.testClientRedTriangles)
            self.addCleanup(self.target.kill_application, self.testClientRedTriangles)

            time.sleep(1)

    def impl_tearDown(self):
        if self.target.systemCompositorControllerSupported:
            self.target.ivi_control.cleanup()
            self.target.kill_application(self.testClientRedTriangles)
            self.target.kill_application(self.testClient3Triangles)
            self.target.kill_application(self.rendererRight)
            self.target.kill_application(self.rendererLeft)
            self.target.kill_application(self.ramsesDaemon)
            log.info("all applications killed")
            self.save_application_output(self.testClientRedTriangles)
            self.save_application_output(self.testClient3Triangles)
            self.save_application_output(self.rendererRight)
            self.save_application_output(self.rendererLeft)
            self.save_application_output(self.ramsesDaemon)
            log.info("output saved")

    def impl_test(self):
        if self.target.systemCompositorControllerSupported:

            self.rendererLeft.showScene(25)
            self.rendererRight.showScene(26)
            ensureSystemCompositorRoundTrip(self.rendererLeft, self.firstSurfaceIviId)
            self.validateScreenshot(self.rendererLeft, "testClient_multiple_ivi_layers.png", useSystemCompositorForScreenshot=True)

        else:
            log.warning("TestMultipleIVILayers skipped as system compositor controller support is not configured for this target")
