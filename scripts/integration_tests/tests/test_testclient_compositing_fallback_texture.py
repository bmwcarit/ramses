#  -------------------------------------------------------------------------
#  Copyright (C) 2015 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework import test_classes
from ramses_test_framework import log
from ramses_test_framework.ramses_test_extensions import with_ramses_process_check


# The test starts a RAMSES client, that shows a cube with six stream textures and checks if the fallback textures are shown.
class TestCompositingFallbackTexture(test_classes.OnAllDefaultTargetsTest):

    @with_ramses_process_check
    def impl_setUp(self):
        applicationName = "ramses-local-client-test-{}".format(self.target.defaultPlatform)
        self.renderer = self.target.start_renderer(applicationName=applicationName,
                                                   args="--ec-display wayland-10 --ec-socket-group mgu_wayland --test-nr 10 --test-state 0")
        self.checkThatApplicationWasStarted(self.renderer)
        self.addCleanup(self.target.kill_application, self.renderer)
        self.percentageOfWrongPixelsAllowed = 0.01

    def impl_tearDown(self):
        self.target.kill_application(self.renderer)
        log.info("all applications killed")
        self.save_application_output(self.renderer)
        log.info("output saved")

    def impl_test(self):
        self.renderer.showScene(34)
        self.validateScreenshot(self.renderer, "testClient_compositing_fallbacktexture.png")
