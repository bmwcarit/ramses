#  -------------------------------------------------------------------------
#  Copyright (C) 2016 BMW Car IT GmbH
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


class TestClass(test_classes.OnAllDefaultTargetsTest):

    @with_ramses_process_check
    def impl_setUp(self):
        standAloneRendererIviSurfaceId = DEFAULT_TEST_SURFACE + 1
        self.standAloneRenderer = self.target.start_default_renderer(args='-sid {0}'.format(standAloneRendererIviSurfaceId))
        self.checkThatApplicationWasStarted(self.standAloneRenderer)
        self.addCleanup(self.target.kill_application, self.standAloneRenderer)

        applicationName = "ramses-local-client-test-{}".format(self.target.defaultPlatform)
        self.combinedLocalClientRenderer = self.target.start_renderer(applicationName, dltAppID="LCLT")
        self.checkThatApplicationWasStarted(self.combinedLocalClientRenderer)
        self.addCleanup(self.target.kill_application, self.combinedLocalClientRenderer)

        self.ramsesDaemon = self.target.start_daemon()
        self.checkThatApplicationWasStarted(self.ramsesDaemon)
        self.addCleanup(self.target.kill_application, self.ramsesDaemon)

    def impl_tearDown(self):
        self.target.kill_application(self.combinedLocalClientRenderer)
        self.target.kill_application(self.ramsesDaemon)
        self.target.kill_application(self.standAloneRenderer)
        log.info("all applications killed")

        self.save_application_output(self.combinedLocalClientRenderer)
        self.save_application_output(self.ramsesDaemon)
        self.save_application_output(self.standAloneRenderer)
        log.info("output saved")

    def impl_test(self):
        self.combinedLocalClientRenderer.showScene(42)
        self.validateScreenshot(self.combinedLocalClientRenderer, "testClient_triangleRed.png")
        self.standAloneRenderer.showScene(67)
        self.validateScreenshot(self.standAloneRenderer, "testClient_triangleBlue.png")
