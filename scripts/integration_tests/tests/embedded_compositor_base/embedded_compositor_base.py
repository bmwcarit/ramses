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
from ramses_test_framework import application


class EmbeddedCompositorBase(test_classes.OnSelectedTargetsTest):
    def __init__(self, methodName ='runTest', testSceneState=0):
        test_classes.OnSelectedTargetsTest.__init__(self, methodName)
        self._testSceneState = testSceneState

    @with_ramses_process_check
    def impl_setUp(self):
        if not self.target.embeddedCompositingSupported:
            self.skipTest("Embedded compositor support is not configured for this target")

        self.percentageOfRGBDifferenceAllowedPerPixel = 0.004   #allows +/- 1 for rgb values (needed e.g. for ufo driver)
        self.percentageOfWrongPixelsAllowed = 0.0004            #allows few wrong pixels, same as above...

        self.ramsesDaemon = self.target.start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.target.kill_application, self.ramsesDaemon)
        self.renderer = self.target.start_default_renderer(args="--wayland-socket-embedded wayland-10 --wayland-socket-embedded-groupname wayland -nomap")
        self.checkThatApplicationWasStarted(self.renderer)
        self.addCleanup(self.target.kill_application, self.renderer)
        self.testClient = self.target.start_client("ramses-test-client", "-tn 10 -ts {} -cz 5".format(self._testSceneState))
        self.checkThatApplicationWasStarted(self.testClient)
        self.addCleanup(self.target.kill_application, self.testClient)
        self.flushname = 44

    def impl_tearDown(self):
        self.target.ivi_control.cleanup()
        self.target.kill_application(self.testClient)
        self.target.kill_application(self.renderer)
        self.target.kill_application(self.ramsesDaemon)
        log.info("all applications killed")
        self.save_application_output(self.testClient)
        self.save_application_output(self.renderer)
        self.save_application_output(self.ramsesDaemon)
        log.info("output saved")

    def _startIviGears(self, iviID, alternateColors = False):
        self.watchSurfaceFound = self.renderer.start_watch_stdout()
        gearsArguments = "-I {} --still".format(iviID)
        if alternateColors:
            gearsArguments += " --alternateColors"
        self.wlClient = self.target.start_application("ivi-gears", gearsArguments,
                                                      binaryDirectoryOnTarget=self.target.baseWorkingDirectory,
                                                      env={"WAYLAND_DISPLAY" : "wayland-10"})
        self.checkThatApplicationWasStarted(self.wlClient)
        self.addCleanup(self.target.kill_application, self.wlClient)
        surfaceFound = self.renderer.wait_for_msg_in_stdout(self.watchSurfaceFound,
                                                            "embedded-compositing client surface found for existing streamtexture: {}".format(iviID),
                                                            timeout=application.Application.DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT)
        self.assertTrue(surfaceFound, msg="Surface was not found by renderer")

    def _killIviGears(self):
        self.watchSurfaceIsGone = self.renderer.start_watch_stdout()
        self.target.kill_application(self.wlClient)
        surfaceIsGone = self.renderer.wait_for_msg_in_stdout(self.watchSurfaceIsGone,
                                                            "embedded-compositing client surface destroyed",
                                                            timeout=application.Application.DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT)
        self.assertTrue(surfaceIsGone, msg="Surface was not destroyed")

    def _stopIviGears(self):
        self.watchSurfaceIsGone = self.renderer.start_watch_stdout()
        self.target.execute_on_target("killall -SIGINT " + self.wlClient.name)
        surfaceIsGone = self.renderer.wait_for_msg_in_stdout(self.watchSurfaceIsGone,
                                                            "embedded-compositing client surface destroyed",
                                                            timeout=application.Application.DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT)
        self.assertTrue(surfaceIsGone, msg="Surface was not destroyed")

    def syncSceneOnClientAndRenderer(self, sceneId):
        self.flushname += 1
        self.flushIsReceived = self.renderer.start_watch_stdout()
        self.testClient.send_ramsh_command("sceneversion {} {}".format(self.flushname, sceneId), waitForRendererConfirmation=False)
        flushWasReceived = self.renderer.wait_for_msg_in_stdout(self.flushIsReceived,
                                                            "Named flush applied on scene {} with sceneVersionTag {}".format(sceneId, self.flushname),
                                                            timeout=30)
        self.assertTrue(flushWasReceived, msg="Renderer didn't receive named flush")

