#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import re

from ramses_test_framework import test_classes, log, helper


class TestClientResourceFileReading(test_classes.OnSelectedTargetsTest):

    def impl_test(self):

        # check if ltrace tool is available on the present target
        if not self.target.ltraceCommandSupported:
            self.skipTest("ltrace command not available")

        applicationToTrace = "./ramses-local-client-test-{} -tn 3".format(self.target.defaultPlatform)
        commandToExecute = "ltrace -f -e fopen+fclose+fseek {}".format(applicationToTrace)

        log.info("Start tracing of " + applicationToTrace)
        (stdoutdata, stderrdata, returncode) = self.target.execute_on_target(commandToExecute, env=self.target.defaultEnvironment, cwd=self.target.ramsesInstallDir+"/bin")
        self.assertEquals(returncode, 0, "Tracing failure, Execution of command \"{}\" returned error code {}\nstdout: {}\nstderr: {}".format(commandToExecute, returncode, stdoutdata, stderrdata))
        log.info("Tracing succeeded")

        hasError = self.hasFileReadingErrors(stderrdata)
        self.assertEquals(hasError, False, msg = "Found file opening/reading errors. Here is the trace output:\n\n" + "".join(stderrdata))


    def checkForIncreasingSeekOrder(self, seekPositions):
        return seekPositions == sorted(seekPositions)


    def hasFileReadingErrors(self, traceOutput):
        openFileSeekPositions = {}
        openFileNames = {}
        openedFileCounter = {}
        error = False

        openRegEx = re.compile('fopen\("(.+.ramres)".*, "rb"\) = (0x[\w]+)')
        seekRegEx = re.compile('fseek\((0x[\w]+), ([\w]+),')
        closeRegEx = re.compile('fclose\((0x[\w]+)\)')

        for line in traceOutput:

            fileOpenMatch = openRegEx.search(line)
            if fileOpenMatch:
                fileName = fileOpenMatch.groups()[0]
                fileHandle = fileOpenMatch.groups()[1]

                assert(fileHandle not in openFileSeekPositions)
                openFileSeekPositions[fileHandle] = []
                openFileNames[fileHandle] = fileName

                if fileName not in openedFileCounter:
                    openedFileCounter[fileName] = 1
                else:
                    openedFileCounter[fileName] += 1

                continue

            fileSeekMatch = seekRegEx.search(line)
            if fileSeekMatch:
                fileHandle = fileSeekMatch.groups()[0]

                if fileHandle in openFileSeekPositions:
                    seekPostion = int(fileSeekMatch.groups()[1], 0) # positions may also occur in hex
                    openFileSeekPositions[fileHandle].append(seekPostion)

                continue

            fileCloseMatch = closeRegEx.search(line)
            if fileCloseMatch:
                fileHandle = fileCloseMatch.groups()[0]

                if fileHandle in openFileSeekPositions:
                    if not self.checkForIncreasingSeekOrder(openFileSeekPositions[fileHandle]):
                        error |= True
                        log.error("no increasing seek/read actions in file {}: {}".format(openFileNames[fileHandle], openFileSeekPositions[fileHandle]))
                    del openFileNames[fileHandle]
                    del openFileSeekPositions[fileHandle]

        if len(openedFileCounter) == 0:
            error |= True
            log.warning("no ramses resource file (*.ramres) was opened during the test. Please check your test application.")
        else:
            for fileName, openCount in openedFileCounter.iteritems():
                if openCount > 1:
                    error |= True
                    log.error("file {} was opened/closed {} times (only once is allowed)".format(fileName, openCount))
                else:
                    log.info("file {} was opened only once".format(fileName))

        return error
