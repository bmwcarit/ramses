#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import unittest
import os
from ramses_test_framework import log


class IntegrationTest(unittest.TestCase):

    def __init__(self, methodName='runTest'):
        unittest.TestCase.__init__(self, methodName)
        self.percentageOfWrongPixelsAllowed = 0.0
        self.percentageOfRGBDifferenceAllowedPerPixel = 0.0
        self.numberOfRequiredUnequalPixels = 1000
        self.screenshotNumber = 0

    def fullClassName(self):
        return self.__module__+'.'+self.__class__.__name__

    def validateScreenshotOnTarget(self, renderer, imageName, target, displayNumber = 0, useSystemCompositorForScreenshot = False, compareForEquality = True):
        target.take_screenshot_and_compare(renderer, imageName, self.fullClassName(), self.name_test_run(),
            displayNumber, self.screenshotNumber, self.percentageOfWrongPixelsAllowed,
            self.percentageOfRGBDifferenceAllowedPerPixel, self.numberOfRequiredUnequalPixels,
            useSystemCompositorForScreenshot, compareForEquality)
        self.screenshotNumber += 1

    def save_application_output_on_target(self, application, target, nr=None):
        target.save_application_output(application, self.fullClassName(), self.name_test_run(), nr)

    def checkThatApplicationWasStarted(self, application):
        self.assertTrue(application.started, "Application {} could not be started".format(application.name))

    def setUp(self):
        #output separator
        print ""
        log.separator("=")

        log.default_file_logger = log.FileLogger(os.path.join(self._get_result_dir(), "test.log"))

        self.impl_setUp()

    def impl_setUp(self):
        pass

    def testRun(self):
        self.impl_test()

    def impl_test(self):
        pass

    def tearDown(self):
        self.impl_tearDown()
        log.default_file_logger.close()
        log.default_file_logger = None

    def impl_tearDown(self):
        pass

    def name_test_run(self):
        pass

    def _get_result_dir(self):
        pass

class OneConnectionTest(IntegrationTest):
    """ Test that contains a ssh connection to one target.

    This all components are started on this target.
    """

    def __init__(self, methodName='runTest'):
        IntegrationTest.__init__(self, methodName)
        self.target = None

    def set_target(self, target):
        self.target = target

    def id(self):
        """ id is overwritten to add name of target """
        orig = unittest.TestCase.id(self)
        if self.target is None:
            targetInfo = " (target: Target is not set )"
        else:
            targetInfo = " (target:" + self.target.name + ")"
        return orig + targetInfo

    def __str__(self):
        """ __str__ is overwritten to add name of target """
        orig = unittest.TestCase.__str__(self)
        return orig + " (target:" + self.target.name + ")"

    def validateScreenshot(self, renderer, imageName, displayNumber=0, useSystemCompositorForScreenshot=False, compareForEquality = True):
        log.info("Validating test via screenshot comparison")
        self.validateScreenshotOnTarget(renderer, imageName, self.target, displayNumber, useSystemCompositorForScreenshot, compareForEquality)

    def get_nr_targets(self):
        return 1

    def save_application_output(self, application, nr=None):
        self.save_application_output_on_target(application, self.target, nr)

    def name_test_run(self):
        return "testRun_target_{}".format(self.target.name)

    def _get_result_dir(self):
        return os.path.join(self.target.resultDir, self.fullClassName(), self.name_test_run())


class OnAllDefaultTargetsTest(OneConnectionTest):
    """ A OneConnectionTest that should be executed on all default targets  (marker class)"""

    def __init__(self, methodName='runTest'):
        OneConnectionTest.__init__(self, methodName)


class OnSelectedTargetsTest(OneConnectionTest):
    """ A OneConnectionTest that should be executed on selected targets"""

    def __init__(self, methodName='runTest'):
        OneConnectionTest.__init__(self, methodName)


class MultipleConnectionsTest(IntegrationTest):
    """ Test that contains ssh connections to multiple target.    """

    def __init__(self, methodName='runTest'):
        IntegrationTest.__init__(self, methodName)
        self.targets = []

    def add_target(self, target):
        self.targets.append(target)

    def get_nr_targets(self):
        """  This method has to be overriden by subclasses!"""
        return 0

    def id(self):
        """ id is overwritten to add name of targets """
        orig = unittest.TestCase.id(self)
        return orig + " (targets:" + self._targetNames() + ")"

    def __str__(self):
        """ id is overwritten to add name of targets """
        orig = unittest.TestCase.__str__(self)
        return orig + " (targets:" + self._targetNames() + ")"

    def _targetNames(self):
        targetNames = ""
        i = 0
        for target in self.targets:
            targetNames += " {0}: {1}".format(i, target.name)
            i += 1
        return targetNames

    def name_test_run(self):
        testRunName = "testRun_targets"
        i = 0
        for target in self.targets:
            testRunName += "_{0}-{1}".format(i, target.name)
            i += 1
        return testRunName

    def _get_result_dir(self):
        return os.path.join(self.targets[0].resultDir, self.fullClassName(), self.name_test_run())
