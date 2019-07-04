#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import time

from ramses_test_framework import test_classes
from ramses_test_framework import log
from ramses_test_framework.ramses_test_extensions import with_ramses_process_check
from ramses_test_framework.targets.target import DEFAULT_TEST_SURFACE
from ramses_test_framework.ramses_test_extensions import ensureSystemCompositorRoundTrip

class TestRendererTwoDisplays(test_classes.OnAllDefaultTargetsTest):

    @with_ramses_process_check
    def impl_setUp(self):
        self.ramsesDaemon = self.target.start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.target.kill_application, self.ramsesDaemon)
        #The test needs 3 displays on wayland in order to use a display as a dummy black background
        #those wayland ivi surface ids depend on workaround in standalone renderer that increments the ivi surface id by 1 for every created display
        self.displaysIviSurfaceIds = [DEFAULT_TEST_SURFACE, DEFAULT_TEST_SURFACE + 1, DEFAULT_TEST_SURFACE + 2]
        self.renderer = self.target.start_default_renderer("--numDisplays 3 --disableAutoMapping -sid {0}".format(self.displaysIviSurfaceIds[0]))
        self.renderer.send_ramsh_command("skub 0", waitForRendererConfirmation=True)

        self.checkThatApplicationWasStarted(self.renderer)
        self.addCleanup(self.target.kill_application, self.renderer)
        self.testClient = self.target.start_client("ramses-test-client", "-tn 2 -ts 0")
        self.checkThatApplicationWasStarted(self.testClient)
        self.addCleanup(self.target.kill_application, self.testClient)

    def impl_tearDown(self):
        self.target.kill_application(self.testClient)
        self.target.kill_application(self.renderer)
        self.target.kill_application(self.ramsesDaemon)
        log.info("all applications killed")
        self.save_application_output(self.testClient)
        self.save_application_output(self.renderer)
        self.save_application_output(self.ramsesDaemon)
        log.info("output saved")

    def impl_test(self):
        self.renderer.showScene(21, 1)
        self.renderer.showScene(22, 2)

        # Can't rely on system compositor behavior here. Especially given the fact that
        # its scene is layouted with integers, which inevitably must be converted to
        # floats and vertices somewhere -> float logic on target platforms can be different.
        self.percentageOfWrongPixelsAllowed = 0.001
        self.validateScreenshot(self.renderer, "testClient_twoScenes_displ0.png", displayNumber=1)
        self.validateScreenshot(self.renderer, "testClient_twoScenes_displ1.png", displayNumber=2)

        if self.target.systemCompositorControllerSupported:
            #dummy display covers whole screen
            self.renderer.send_ramsh_command("screct {0} 000 0 1900 800".format(self.displaysIviSurfaceIds[0]), waitForRendererConfirmation=True)
            #test displays are next to each other
            self.renderer.send_ramsh_command("screct {0} 0 0 200 600".format(self.displaysIviSurfaceIds[1]), waitForRendererConfirmation=True)
            self.renderer.send_ramsh_command("screct {0} 200 0 200 600".format(self.displaysIviSurfaceIds[2]), waitForRendererConfirmation=True)
            ensureSystemCompositorRoundTrip(self.renderer, self.displaysIviSurfaceIds[2])
            self.validateScreenshot(self.renderer, "testClient_twoDisplaysPlacedTogether.png", useSystemCompositorForScreenshot=True)
