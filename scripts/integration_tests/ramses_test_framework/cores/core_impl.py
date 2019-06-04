#  -------------------------------------------------------------------------
#  Copyright (C) 2015 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


import unittest
import os
import time
import re
import random

from ramses_test_framework import helper
from ramses_test_framework import log
from ramses_test_framework import test_classes
from ramses_test_framework.argparser import AdaptedArgParser


class CoreImpl:
    def __init__(self, config):
        self.config = config
        self.targets = {}
        self.bridgeTargets = {}
        self.defaultTestTargets = []
        self.basePath = ""
        self.filter = None
        self.fullResultsDirPath = ""
        self.randomTestSeed = random.randint(0, 10000000)

    def read_arguments(self):
        pass

    def setup(self, resultsDir, transfer_binaries):
        pass

    def createTargets(self):
        pass

    def setupTargets(self, transfer_binaries):
        setupSuccessful = True

        for target in self.bridgeTargets.itervalues():
            # target setup (connection)
            log.info("setup bridge target {}".format(target.name))
            setupSuccessful &= target.setup(False)

        for target in self.targets.itervalues():
            # target setup (connection, transfer binaries etc.)
            log.info("setup target {}".format(target.name))
            setupSuccessful &= target.setup(transfer_binaries)

        return setupSuccessful

    def run_tests(self, testDirs):
        expandedSuite = unittest.TestSuite()

        for dir in testDirs:
            # discover
            # search for tests in given dir
            suite = unittest.TestLoader().discover(dir, pattern=self.config.testPattern, top_level_dir=self.config.scriptDir)
            #expand
            self._expand_test_suite(suite, expandedSuite)

        #filter
        testList = []
        if self.filter is None:
            testList = [t for t in expandedSuite]
        else:
            for test in expandedSuite:
                filter_re = re.compile(self.filter, re.IGNORECASE)
                if filter_re.search(test.id()):
                    testList.append(test)

        # sort by id for deterministic order
        testList = sorted(testList, key=lambda x: x.id())

        # use local rng to shuffle
        log.info("Seed used for test ordering: {}".format(self.randomTestSeed))
        rng = random.Random(self.randomTestSeed)
        random.shuffle(testList, lambda: rng.random())

        # add to final test suite
        filteredSuite = unittest.TestSuite()
        for t in testList:
            filteredSuite.addTest(t)

        # print tests to run in final order
        log.color_separator(log.light_cyan,"discovered tests:")
        for test in filteredSuite:
            log.info(test.id())
        print("\n")

        #small wait for output formatting
        time.sleep(0.01)
        #run
        runner = self._create_test_runner()
        result = runner.run(filteredSuite)
        self._post_process_test_results()

        for target in self.targets.itervalues():
            target.tests_finished()

        return len(result.errors) == 0 and len(result.failures) == 0

    def _create_test_runner(self):
        pass

    def _post_process_test_results(self):
        pass

    def tear_down(self, shutdownTargets):
        anyTargetWasConnected = False
        for target in self.targets.itervalues():
            anyTargetWasConnected |= target.isConnected
            target.target_specific_tear_down(shutdown=shutdownTargets)

        # turn off power
        if shutdownTargets:
            if anyTargetWasConnected:
                # wait time till all targets should have finished shutdown properly
                time.sleep(60)

            for target in self.targets.itervalues():
                if target.powerDevice is not None:
                    target.powerDevice.switch(target.powerOutletNr, False)

        #tear-down and shutdown bridges after test targets (otherwise connection will be lost)
        anyBridgeTargetWasConnected = False
        for target in self.bridgeTargets.itervalues():
            anyBridgeTargetWasConnected |= target.isConnected
            target.target_specific_tear_down(shutdown=shutdownTargets)

        #turn off power
        if shutdownTargets:
            if anyBridgeTargetWasConnected:
                #wait time till all bridgeTargets should have finished shutdown properly
                time.sleep(60)

            for target in self.bridgeTargets.itervalues():
                if target.powerDevice is not None:
                    target.powerDevice.switch(target.powerOutletNr, False)


    def _expand_test_suite(self, testSuite, expandedSuite):
        for item in testSuite:
            # recurse into sub-testSuites
            if isinstance(item, unittest.TestSuite):
                self._expand_test_suite(item, expandedSuite)
            else:
                self._expand_test(item, expandedSuite)

    def _expand_test(self, test, expandedSuite):
        pass

    def _checkIfAllTargetsReady(self, targetSet):
        for target in targetSet:
            if not target.isConnected:
                log.error("target '{0}' is not ready".format(target.name))
                return False

        return True

    def _checkIfValidConfig(self, test, targetSet):
        if test.get_nr_targets() != len(targetSet):
            log.error("Invalid configuration, number of connections defined in testToTargetConfig does not match with \
                    with number of connections expected in test (test: {0})".format(type(test).__name__))
            return False
        return True


class LocalCoreImpl(CoreImpl):
    def __init__(self, config):
        CoreImpl.__init__(self, config)
        self.platform = ""

        # ensure all tested applications inherit a sensible console log level or most tests will fail (e.b. if off)
        os.environ['CONSOLE_LOGLEVEL'] = 'info'

    def read_arguments(self):
        parser = AdaptedArgParser()
        parser.add_argument("path", help="path to ramses install directory and where test results should be stored")
        parser.add_argument("--platform", default=self.config.defaultPlatform,
            help="Platform to use as default, possibilities: '"+self.config.defaultPlatform+"' (default), 'x11-egl-es-3-0', 'wayland-ivi-egl-es-3-0', ...")
        parser.add_argument("--filter", help="test filter")
        parser.add_argument("--random-seed", default=self.randomTestSeed, help="random seed used for test ordering")
        args = parser.parse_args()
        self.basePath = os.path.normcase(args.path)
        self.platform = args.platform
        self.filter = args.filter
        self.randomTestSeed = args.random_seed

    def createTargets(self):
        CoreImpl.createTargets(self)
        for targetInfo in self.config.allTargetsList:
            #create target object based on targetInfo and general config values
            target = targetInfo.classname(targetInfo, self.basePath, self.fullResultsDirPath, self.config.imagesDesiredDirs,
                                          self.config.imageDiffScaleFactor, logLevel=self.config.ramsesApplicationLogLevel)
            self.targets[targetInfo] = target
            if targetInfo in self.config.defaultTestTargetsList:
                self.defaultTestTargets.append(target)
            #special settings for local case
            target.defaultPlatform = self.platform
            target.isConnected = True

    def setup(self, resultsDir, transfer_binaries):
        self.fullResultsDirPath = helper.create_result_dir(self.basePath, resultsDir)
        self.createTargets()
        return self.setupTargets(transfer_binaries)

    def _create_test_runner(self):
        return unittest.TextTestRunner(verbosity=2)

    def _expand_test(self, test, expandedSuite):
        for target in self.targets.itervalues():
            if isinstance(test, test_classes.OneConnectionTest) or isinstance(test, test_classes.OnSelectedTargetsTest):
                test.target = target
            if isinstance(test, test_classes.MultipleConnectionsTest):
                #testToTarget configuration is ignored as all applications are started locally
                for i in range(test.get_nr_targets()):
                    test.add_target(target)
            expandedSuite.addTest(test)
