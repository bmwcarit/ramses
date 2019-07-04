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

class TestMoveBetweenIVILayers(test_classes.OnSelectedTargetsTest):

    def impl_setUp(self):
        if not self.target.systemCompositorControllerSupported:
            self.skipTest("System compositor controller support is not configured for this target")

        self.backgroundRendererLayerIviId = DEFAULT_TEST_LAYER
        self.firstLayerIviId = DEFAULT_TEST_LAYER + 1
        self.secondLayerIviId = DEFAULT_TEST_LAYER + 2

        self.backgroundRendererSurfaceIviId = DEFAULT_TEST_SURFACE
        self.rendererSurfaceIviId = DEFAULT_TEST_SURFACE + 1

        self.target.ivi_control.createLayer(self.firstLayerIviId,640,480)
        self.target.ivi_control.createLayer(self.secondLayerIviId,640,480)
        self.target.ivi_control.setLayerVisibility(self.firstLayerIviId, visible=True)
        self.target.ivi_control.setLayerVisibility(self.secondLayerIviId, visible=True)
        self.target.ivi_control.setLayerDestRect(self.secondLayerIviId,640,0,640,480)
        self.target.ivi_control.appendLayerToScreenRenderorder(4, self.firstLayerIviId)
        self.target.ivi_control.appendLayerToScreenRenderorder(4, self.secondLayerIviId)
        self.target.ivi_control.flush()

        self.ramsesDaemon = self.target.start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.target.kill_application, self.ramsesDaemon)
        self.renderer = self.target.start_default_renderer("--waylandIviLayerId {0} --waylandIviSurfaceID {1} --wayland-socket-embedded wayland-13 --disableAutoMapping -w 640".format(self.firstLayerIviId, self.rendererSurfaceIviId))
        self.checkThatApplicationWasStarted(self.renderer)
        self.addCleanup(self.target.kill_application, self.renderer)
        self.rendererbackground = self.target.start_default_renderer("--waylandIviLayerId {0} --waylandIviSurfaceID {1} --wayland-socket-embedded wayland-12 --disableAutoMapping -w 640".format(self.backgroundRendererLayerIviId, self.backgroundRendererSurfaceIviId))
        self.checkThatApplicationWasStarted(self.rendererbackground)
        self.addCleanup(self.target.kill_application, self.rendererbackground)
        self.testClient3Triangles = self.target.start_client("ramses-test-client", "-tn 5 -ts 0 -cz 5")
        self.checkThatApplicationWasStarted(self.testClient3Triangles)
        self.addCleanup(self.target.kill_application, self.testClient3Triangles)

        time.sleep(1)

    def impl_tearDown(self):
        self.target.ivi_control.cleanup()
        self.target.kill_application(self.testClient3Triangles)
        self.target.kill_application(self.rendererbackground)
        self.target.kill_application(self.renderer)
        self.target.kill_application(self.ramsesDaemon)
        log.info("all applications killed")
        self.save_application_output(self.testClient3Triangles)
        self.save_application_output(self.renderer)
        self.save_application_output(self.ramsesDaemon)
        log.info("output saved")

    def impl_test(self):
        self.renderer.showScene(26)
        self.renderer.send_ramsh_command("skipUnmodifiedBuffers 0", waitForRendererConfirmation=True)

        ## Add surface to second layer
        self.renderer.send_ramsh_command("scAddSurfaceToLayer {0} {1}".format(self.rendererSurfaceIviId, self.secondLayerIviId), waitForRendererConfirmation=True)
        ensureSystemCompositorRoundTrip(self.renderer, self.rendererSurfaceIviId)
        self.validateScreenshot(self.renderer, "testClient_move_between_IVI_layers_A.png", useSystemCompositorForScreenshot=True)

        ## Remove surface from first layer
        self.renderer.send_ramsh_command("scRemoveSurfaceFromLayer {0} {1}".format(self.rendererSurfaceIviId, self.firstLayerIviId), waitForRendererConfirmation=True)
        ensureSystemCompositorRoundTrip(self.renderer, self.rendererSurfaceIviId)
        self.validateScreenshot(self.renderer, "testClient_move_between_IVI_layers_B.png", useSystemCompositorForScreenshot=True)
