
#  -------------------------------------------------------------------------
#  Copyright (C) 2015 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import copy
import paramiko
import os
import xmlrunner
import itertools

from ramses_test_framework import helper
from ramses_test_framework import log
from ramses_test_framework.argparser import AdaptedArgParser
from ramses_test_framework.cores.core_impl import CoreImpl
from ramses_test_framework import test_classes
from ramses_test_framework.targets.targetInfo import BridgedTargetInfo


class RemoteCoreImpl(CoreImpl):
    def __init__(self, config):
        CoreImpl.__init__(self, config)
        self.ramsesVersion = ""
        self.gitCommitCount = ""
        self.gitCommitHash = ""
        self.noTransfer = False
        self.downloadFromCICache = False

    def read_arguments(self):
        parser = AdaptedArgParser()
        transferGroup = parser.add_mutually_exclusive_group(required=True)
        targetsGroup = parser.add_mutually_exclusive_group(required=False)

        self._add_arguments(parser, transferGroup, targetsGroup)

        args = parser.parse_args()
        self._interpret_arguments(args)

    def _add_arguments(self, parser, transferGroup, targetsGroup):
        parser.add_argument("basePath", help="Path where .tar.gz package files can be found and where test results "
                                             "should be stored")
        transferGroup.add_argument("--package", nargs=3, metavar=('ramsesVersion', 'gitCommitCount', 'gitCommitHash'), help="for package download. wildcards can be used, first found result will be taken")
        transferGroup.add_argument("--noTransfer", action="store_true", default=False, help="do not transfer binaries (for debugging)")
        parser.add_argument("--filter", help="test filter")
        targetsGroup.add_argument("--targets", default=None, help="target filter")

    def _interpret_arguments(self, args):
        self.basePath = args.basePath
        if args.package is not None:
            self.ramsesVersion = args.package[0]
            self.gitCommitCount = args.package[1]
            self.gitCommitHash = args.package[2]
        self.filter = args.filter
        self.noTransfer = args.noTransfer
        if args.targets:
            self.config.filterTargets(args.targets)

    def _createTarget(self, targetInfo):
        #create target object based on targetInfo and general config values
        target = targetInfo.classname(targetInfo, self.config.ramsesInstallDir, self.fullResultsDirPath,
            self.config.imagesDesiredDirs, self.config.imageDiffScaleFactor,
            self.basePath, self.ramsesVersion, self.gitCommitCount, self.gitCommitHash,
            self.config.sshConnectionNrAttempts, self.config.sshConnectionTimeoutPerAttempt,
            self.config.sshConnectionSleepPerAttempt, self.config.powerNrAttempts,
            self.config.ramsesApplicationLogLevel
        )
        return target

    def _createBridgedTarget(self, bridgeTarget, targetInfo):
        target = targetInfo.classname(bridgeTarget, targetInfo, self.config.ramsesInstallDir, self.fullResultsDirPath,
            self.config.imagesDesiredDirs, self.config.imageDiffScaleFactor,
            self.basePath, self.ramsesVersion, self.gitCommitCount, self.gitCommitHash,
            self.config.sshConnectionNrAttempts, self.config.sshConnectionTimeoutPerAttempt,
            self.config.sshConnectionSleepPerAttempt, self.config.powerNrAttempts,
            self.config.ramsesApplicationLogLevel
        )
        return target

    def createTargets(self):
        for targetInfo in self.config.allTargetsList:
            if isinstance(targetInfo, BridgedTargetInfo):
                # multiple targets can share a bridge, create target only once
                if targetInfo.targetInfoBridgeTarget not in self.bridgeTargets:
                    bridgeTarget = self._createTarget(targetInfo.targetInfoBridgeTarget)
                    self.bridgeTargets[targetInfo.targetInfoBridgeTarget] = bridgeTarget
                target = self._createBridgedTarget(self.bridgeTargets[targetInfo.targetInfoBridgeTarget], targetInfo)
            else:
                target = self._createTarget(targetInfo)
            self.targets[targetInfo] = target
            if targetInfo in self.config.defaultTestTargetsList:
                self.defaultTestTargets.append(target)

    def setup(self, resultsDir, transfer_binaries):
        self.fullResultsDirPath = helper.create_result_dir(self.basePath, resultsDir)
        paramiko.util.log_to_file(os.path.join(self.fullResultsDirPath, 'paramiko.log'))
        self.createTargets()
        log.info("turning on all power outlets...")
        for target in itertools.chain(self.bridgeTargets.itervalues(), self.targets.itervalues()):
            if target.powerDevice is not None:
                if not target.powerDevice.switch(target.powerOutletNr, True):
                    log.info("Could not turn on power outlet for target {0} because the power outlet is not available".format(target.name))
                    return False

        if self.downloadFromCICache:
            for target in self.targets.itervalues():
                downloadSuccessful, version, commitCount = helper.download_tar_from_ci_cache(self.gitCommitHash, target.buildJobName, self.config.ciCacheUrl, self.basePath)
                if not downloadSuccessful:
                    return False
                target.ramsesVersion = version
                target.gitCommitCount = commitCount

        return self.setupTargets(transfer_binaries and (not self.noTransfer))

    def _create_test_runner(self):
        return xmlrunner.XMLTestRunner(output=self.fullResultsDirPath, verbosity=2)

    def _post_process_test_results(self):
        for f in os.listdir(self.fullResultsDirPath):
            if f.endswith(".xml"):
                log.strip_color_codes_from_file(os.path.join(self.fullResultsDirPath, f))

    def _expand_test(self, test, expandedSuite):
        if isinstance(test, test_classes.OnAllDefaultTargetsTest):
            for target in self.defaultTestTargets:
                if target.isConnected:
                    #create a copy of test for each target
                    testForOneTarget = copy.deepcopy(test)
                    testForOneTarget.set_target(target)
                    expandedSuite.addTest(testForOneTarget)

        elif isinstance(test, test_classes.MultipleConnectionsTest) or isinstance(test, test_classes.OnSelectedTargetsTest):
            if self.config.testToTargetConfig.has_key(type(test).__name__):
                for targetInfoSet in self.config.testToTargetConfig[type(test).__name__]:
                    targetSet = [self.targets[i] for i in targetInfoSet]
                    if self._checkIfAllTargetsReady(targetSet) and self._checkIfValidConfig(test, targetSet):
                        # create a copy of test for each target set
                        testForOneTargetSet = copy.deepcopy(test)
                        for target in targetSet:
                            if isinstance(test, test_classes.MultipleConnectionsTest):
                                testForOneTargetSet.add_target(target)
                            else:
                                testForOneTargetSet.set_target(target)
                        expandedSuite.addTest(testForOneTargetSet)
                    else:
                        log.error("test '{0}' could not be added to the active tests due to missing target connection or "
                                "invalid src.".format(test.id()))
            else:
                log.warning("Missing testToTargetConfig for test {0}".format(test.id()))
        else:
            # other kinds of unit tests are executed as usual
            expandedSuite.addTest(test)
