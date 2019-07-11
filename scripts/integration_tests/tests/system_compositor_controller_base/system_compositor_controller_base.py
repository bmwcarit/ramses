#  -------------------------------------------------------------------------
#  Copyright (C) 2017 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework import test_classes
from ramses_test_framework import log
from ramses_test_framework import helper
from ramses_test_framework import application
from ramses_test_framework.ramses_test_extensions import with_ramses_process_check, IVI_Control
from ramses_test_framework.targets.target import DEFAULT_TEST_LAYER
from ramses_test_framework.targets.target import DEFAULT_TEST_SURFACE

class SystemCompositorControllerBase(test_classes.OnSelectedTargetsTest):

    @with_ramses_process_check
    def impl_setUp(self):
        if not self.target.systemCompositorControllerSupported:
            self.skipTest("System compositor controller support is not configured for this target")

        self.percentageOfRGBDifferenceAllowedPerPixel = 0.008
        self.percentageOfWrongPixelsAllowed = 0.001

        # Map of created test surfaces and surface ivi-ids
        # Ensure, that every ivi-id is listed here, for which a surface is created in this test
        # wlClient1 and wlClient4 shall share the same ivi-id
        self.testSurfaceIVIIds = {"renderer":           DEFAULT_TEST_SURFACE ,
                                  "rendererbackground": DEFAULT_TEST_SURFACE + 1,
                                  "wlClient1":          DEFAULT_TEST_SURFACE + 2,
                                  "wlClient2":          DEFAULT_TEST_SURFACE + 3,
                                  "wlClient3":          DEFAULT_TEST_SURFACE + 4,
                                  "wlClient4":          DEFAULT_TEST_SURFACE + 5}

        # Destroy maybe exisiting old ivi-surfaces, needed to reset stored state for visibility, opacity and position in the compositor
        for surface, iviId in self.testSurfaceIVIIds.items():
            self.target.ivi_control.destroySurface(iviId)
        self.target.ivi_control.flush()

        # The surfaces of this tests (ivi-gears + renderer) are put on layer DEFAULT_TEST_LAYER + 1.
        # For having a black background, a second renderer is started on layer DEFAULT_TEST_LAYER, which just shows
        # black content without any scene. Needed to cover the HMI, which runs in the background.

        self.testLayer = DEFAULT_TEST_LAYER + 1

        # Get list of current present surface id's from IVI_Control
        self.expectedSurfaceIds = set(self.target.ivi_control.getSurfaceIds())

        # Start black background renderer
        self.rendererbackground = self.target.start_default_renderer("--waylandIviLayerId {0} --waylandIviSurfaceID {1} --wayland-socket-embedded wayland-12 --disableAutoMapping".format(DEFAULT_TEST_LAYER, self.testSurfaceIVIIds["rendererbackground"]))
        self.checkThatApplicationWasStarted(self.rendererbackground)
        self.addCleanup(self.target.kill_application, self.rendererbackground)
        self.expectedSurfaceIds.add("{0}".format(self.testSurfaceIVIIds["rendererbackground"]))
        self.rendererbackground.send_ramsh_command("skipUnmodifiedBuffers 0", waitForRendererConfirmation=True)

        # Create testLayer
        self.target.ivi_control.createLayer(self.testLayer, 1280, 480)
        self.target.ivi_control.setLayerVisibility(self.testLayer, visible=True)
        self.target.ivi_control.appendLayerToScreenRenderorder(4, self.testLayer)
        self.target.ivi_control.flush()

        # Start ivi-gears No. 1 (colors: red, blue, green)
        self.wlClient1 = self.target.start_application("ivi-gears", "-I {0} --still".format(self.testSurfaceIVIIds["wlClient1"]), binaryDirectoryOnTarget=self.target.baseWorkingDirectory)
        self.addCleanup(self.target.kill_application, self.wlClient1)
        self.wlClient1.initialisation_message_to_look_for("time since startup until first eglSwapBuffers done");
        self.expectedSurfaceIds.add("{0}".format(self.testSurfaceIVIIds["wlClient1"]))

        # Start ivi-gears No. 2 (colors: white, blue, green)
        self.wlClient2 = self.target.start_application("ivi-gears", "-I {0} --still -a".format(self.testSurfaceIVIIds["wlClient2"]), binaryDirectoryOnTarget=self.target.baseWorkingDirectory)
        self.addCleanup(self.target.kill_application, self.wlClient2)
        self.wlClient2.initialisation_message_to_look_for("time since startup until first eglSwapBuffers done");
        self.expectedSurfaceIds.add("{0}".format(self.testSurfaceIVIIds["wlClient2"]))

        # Wait until ivi-gears have rendered a frame
        # The two ivi-gears are started before the renderer, to cover the case, that the scc gets the notification of the already
        # exisiting surfaces at it's startup time. In addition there is also a test which covers the case, when the renderer gets notified
        # about a newly created ivi-surface.
        self.assertTrue(self.wlClient1.is_initialised(timeout=application.Application.DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT))
        self.assertTrue(self.wlClient2.is_initialised(timeout=application.Application.DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT))

        # Start daemon
        self.ramsesDaemon = self.target.start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.target.kill_application, self.ramsesDaemon)

        # Start renderer
        self.renderer = self.target.start_default_renderer("--waylandIviLayerId {0} --waylandIviSurfaceID {1} --wayland-socket-embedded wayland-11 --disableAutoMapping".format(self.testLayer, self.testSurfaceIVIIds["renderer"]))
        self.checkThatApplicationWasStarted(self.renderer)
        self.addCleanup(self.save_application_output, self.renderer)
        self.addCleanup(self.target.kill_application, self.renderer)
        self.renderer.send_ramsh_command("skipUnmodifiedBuffers 0", waitForRendererConfirmation=True)

        self.expectedSurfaceIds.add("{0}".format(self.testSurfaceIVIIds["renderer"]))

        # Start client
        self.testClient = self.target.start_client("ramses-test-client", "-tn 10 -ts 0 -cz 5")
        self.checkThatApplicationWasStarted(self.testClient)
        self.addCleanup(self.target.kill_application, self.testClient)

        # make sure renderer added its surface to layer before applying renderorder
        self.renderer.wait_for_msg_in_stdout_from_beginning("IVIControllerSurface::HandleLayerCallback: surface {} added to layer".format(self.testSurfaceIVIIds["renderer"]), timeout=application.Application.DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT)

        # Put renderer, and ivi-gears No. 1 & 2 on the test layer
        self.target.ivi_control.setLayerRenderorder(self.testLayer, "{0} {1} {2}".format(self.testSurfaceIVIIds["renderer"], self.testSurfaceIVIIds["wlClient1"], self.testSurfaceIVIIds["wlClient2"]))
        self.target.ivi_control.flush()

        # Make cube in renderer visible, and check with screenshot
        self.renderer.showScene(34)
        self.validateScreenshot(self.renderer, "testClient_compositing_fallbacktexture.png")

        # wait until all started applications have been registered in scc
        self.assertTrue(self.wait_on_surfaces_beeing_registered_in_scc())

    def impl_tearDown(self):
        self.target.ivi_control.cleanup()
        self.target.kill_application(self.wlClient1)
        self.target.kill_application(self.wlClient2)
        if hasattr(self, 'wlClient3'):
            self.target.kill_application(self.wlClient3)
        if hasattr(self, 'wlClient4'):
            self.target.kill_application(self.wlClient4)
        self.target.kill_application(self.testClient)
        self.target.kill_application(self.renderer)
        self.target.kill_application(self.rendererbackground)
        self.target.kill_application(self.ramsesDaemon)
        log.info("all applications killed")
        self.save_application_output(self.testClient)
        self.save_application_output(self.renderer)
        self.save_application_output(self.ramsesDaemon)
        log.info("output saved")

    def wait_on_surfaces_beeing_registered_in_scc(self):
        surfaceIdsList = list(self.expectedSurfaceIds)
        surfaceIdsList.sort(key=int)
        surfaceIdSearchRegEx = "SystemCompositorController_Wayland_IVI::listIVISurfaces Known ivi-ids are: {0}\n".format(" (([0-9])* )*".join(surfaceIdsList))
        log.info("waiting on surfaces beeing registered in scc " + surfaceIdSearchRegEx)
        for i in xrange(1, 30):
            if self.renderer.send_ramsh_command("scl", response_message=surfaceIdSearchRegEx, timeout=1):
                return 1
        return 0
