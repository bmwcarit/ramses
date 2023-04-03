#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework import test_classes
from ramses_test_framework import log
from ramses_test_framework.ramses_test_extensions import with_ramses_process_check


class EmbeddedCompositorBase(test_classes.OnSelectedTargetsTest):
    def __init__(self, methodName='runTest', testSceneState=0):
        test_classes.OnSelectedTargetsTest.__init__(self, methodName)
        self._testSceneState = testSceneState

    @with_ramses_process_check
    def impl_setUp(self):
        if not self.target.embeddedCompositingSupported:
            self.skipTest("Embedded compositor support is not configured for this target")

        self.percentageOfRGBDifferenceAllowedPerPixel = 0.004   # allows +/- 1 for rgb values (needed e.g. for ufo driver)
        self.percentageOfWrongPixelsAllowed = 0.0004            # allows few wrong pixels, same as above...

        # start local test client that has renderer with wayland backend to use embedded compositing features
        applicationName = "ramses-local-client-test-wayland-ivi-egl-es-3-0"
        args = "--ec-display wayland-10 --ec-socket-group mgu_wayland --test-nr 10 --test-state {}".format(self._testSceneState)
        self.renderer = self.target.start_renderer(applicationName=applicationName, args=args)
        self.checkThatApplicationWasStarted(self.renderer)
        self.addCleanup(self.target.kill_application, self.renderer)

    def impl_tearDown(self):
        if self.target.systemCompositorControllerSupported:
            # ivi control is supported only on systems with SCC support
            self.target.ivi_control.cleanup()
        self.target.kill_application(self.renderer)
        log.info("all applications killed")
        self.save_application_output(self.renderer)
        log.info("output saved")

    def _startIviGears(self, iviID, alternateColors=False):
        self.watchSurfaceFound = self.renderer.start_watch_stdout()
        gearsArguments = "-I {} --still".format(iviID)
        if alternateColors:
            gearsArguments += " --alternateColors"
        self.wlClientIviGears = self.target.start_application("ivi-gears", gearsArguments,
                                                              binaryDirectoryOnTarget=self.target.baseWorkingDirectory,
                                                              env={"WAYLAND_DISPLAY": "wayland-10"})
        self.checkThatApplicationWasStarted(self.wlClientIviGears)
        self.addCleanup(self.target.kill_application, self.wlClientIviGears)
        surfaceFound = self.renderer.wait_for_msg_in_stdout(self.watchSurfaceFound,
                                                            "embedded-compositing client surface found for existing streamtexture: {}".format(iviID))
        self.assertTrue(surfaceFound, msg="Surface was not found by renderer")

    def _startDmaBufExample(self, iviID, alternateColors=False):
        self.watchSurfaceFound = self.renderer.start_watch_stdout()
        dmaBufExampleArguments = "-I {} --still".format(iviID)
        if alternateColors:
            dmaBufExampleArguments += " --mandelbrot"
        self.wlClientDmaBuf = self.target.start_application("ivi-simple-dmabuf-egl", dmaBufExampleArguments,
                                                            binaryDirectoryOnTarget=self.target.baseWorkingDirectory,
                                                            env={"WAYLAND_DISPLAY": "wayland-10"})
        self.checkThatApplicationWasStarted(self.wlClientDmaBuf)
        self.addCleanup(self.target.kill_application, self.wlClientDmaBuf)
        surfaceFound = self.renderer.wait_for_msg_in_stdout(self.watchSurfaceFound,
                                                            "embedded-compositing client surface found for existing streamtexture: {}".format(iviID))
        self.assertTrue(surfaceFound, msg="Surface was not found by renderer")

    def _killIviGears(self):
        self.watchSurfaceIsGone = self.renderer.start_watch_stdout()
        self.target.kill_application(self.wlClientIviGears)
        surfaceIsGone = self.renderer.wait_for_msg_in_stdout(self.watchSurfaceIsGone,
                                                             "embedded-compositing client surface destroyed")
        self.assertTrue(surfaceIsGone, msg="Surface was not destroyed")

    def _killDmaBufExample(self):
        self.watchSurfaceIsGone = self.renderer.start_watch_stdout()
        self.target.kill_application(self.wlClientDmaBuf)
        surfaceIsGone = self.renderer.wait_for_msg_in_stdout(self.watchSurfaceIsGone,
                                                             "embedded-compositing client surface destroyed")
        self.assertTrue(surfaceIsGone, msg="Surface was not destroyed")

    def _stopIviGears(self):
        self.watchSurfaceIsGone = self.renderer.start_watch_stdout()
        self.target.execute_on_target("killall -SIGINT " + self.wlClientIviGears.name)
        surfaceIsGone = self.renderer.wait_for_msg_in_stdout(self.watchSurfaceIsGone,
                                                             "embedded-compositing client surface destroyed")
        self.assertTrue(surfaceIsGone, msg="Surface was not destroyed")
