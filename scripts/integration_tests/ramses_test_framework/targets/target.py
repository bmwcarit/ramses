#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import os
import posixpath
import time
import glob
import random
import string
from PIL import Image
from abc import ABCMeta, abstractmethod

from ramses_test_framework import helper
from ramses_test_framework import log

SYSTEM_COMPOSITOR_SCREENSHOT_TIMEOUT = 30
CUSTOM_DAEMON_PORT = 6001
DEFAULT_TEST_LAYER = 1000000
DEFAULT_TEST_SURFACE = 1000000

class Target:
    __metaclass__ = ABCMeta

    def __init__(self, targetInfo, ramsesInstallDir, resultDir, imagesDesiredDirs, imageDiffScaleFactor, logLevel):
        """ To use no password for the authentication (key-based) leave password to None!
            If a empty string is passed as password it will be used as password """
        self.isConnected = False
        self.sshClient = None
        self.systemMonitor = None

        self.name = targetInfo.name
        self.hostname = targetInfo.hostname
        self.username = targetInfo.username
        self.buildJobName = targetInfo.buildJobName
        self.powerDevice = targetInfo.powerDevice
        self.powerOutletNr = targetInfo.powerOutletNr
        self.password = targetInfo.password
        self.sshPort = targetInfo.sshPort
        self.tcpTestsInterfaceIp = targetInfo.tcpTestsInterfaceIp
        self.tcpAliveIntervalMs = None
        self.tcpAliveTimeoutMs = None

        self.ramsesInstallDir = ramsesInstallDir
        self.resultDir = resultDir
        self.imagesDesiredDirs = imagesDesiredDirs
        self.imageDiffScaleFactor = imageDiffScaleFactor

        # settings can be overwritten in target_specific_setup
        self.defaultEnvironment = {}
        self.defaultPlatform = "x11-egl-es-3-0"
        self.percentageOfWrongPixelsAllowedOnTarget = 0.0
        self.percentageOfRGBDifferenceAllowedPerPixelOnTarget = 0.0
        self.dltSupported = False
        self.embeddedCompositingSupported = False
        self.systemCompositorControllerSupported = False
        self.binaryShaderCompilerSupported = False
        self.ltraceCommandSupported = False
        self.baseWorkingDirectory = self.ramsesInstallDir + "/bin"
        self.tmpDir = "/tmp"
        self.fixed_screenshot_prefix = 'ramsestlst_'
        self.unique_screenshot_prefix = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(10))
        self.target_screenshot_counter = 0
        self.main_screen_id = -1

        if targetInfo.systemMonitorClassname is not None:
            self.systemMonitor = targetInfo.systemMonitorClassname(resultDir)
        self.logLevel = logLevel

    def setup(self, transfer_binaries=True):
        return False #can be overwritten by sub-classes

    def tests_finished(self):
        """
        Callback when tests are finished
        """
        pass

    def target_specific_tear_down(self, shutdown=True):
        # delete all screenshots at once
        if self.isConnected:
            self._delete_files_on_target("{}*".format(self.fixed_screenshot_prefix), self.tmpDir)

    @abstractmethod
    def execute_on_target(self, commandToExecute, block=True, env=None, cwd=None):
        """
        Function to execute a simple command on the target.
        If you need more control over the application or get the application output, use the start_application function
        @:return Tuple (stdout output as list of lines, stderr output as list of lines, return code) if block=True
        """
        pass

    @abstractmethod
    def copy_file_from_target(self, targetFileName, destFile, targetWorkingDirectory=None):
        """
        Copy a file from target to test host
        :param targetFileName: filename on target
        :param destFile: name of the destination file (optionally with path)
        :param targetWorkingDirectory: directory where to search for the file (relative to baseWorkingDirectory
        configured for the target). If None the baseWorkingDirectory is used
        """
        pass

    @abstractmethod
    def copy_file_to_target(self, sourceFile, targetFileName, targetWorkingDirectory=None):
        """
        Copy a file from test host to target
        :param sourceFile: name of the source file (optionally with path)
        :param targetFileName: filename on target
        :param targetWorkingDirectory: directory where to write the file to (relative to baseWorkingDirectory
        configured for the target). If None the baseWorkingDirectory is used
        """
        pass

    def start_daemon(self,  args="", workingDirectory=None, ramsesDaemonTarget=None, nameExtension="", env={}, dltAppID='SMGR'):
        #use custom daemon port for all ramses applications to avoid connections to other applications running on the system (e.g. the HMI)
        extendedArgs = args + " -myport {}".format(CUSTOM_DAEMON_PORT)
        daemon = self._start_ramses_application("ramses-daemon", extendedArgs, workingDirectory,  nameExtension, env, dltAppID)
        daemon.initialisation_message_to_look_for("Ramsh commands registered")
        return daemon

    def start_renderer(self, applicationName, args="", workingDirectory=None, ramsesDaemonTarget=None, nameExtension="", env={}, dltAppID='REND', waitForDisplayManagerRamsh=True):
        extendedArgs = "--startVisible -scc " + args
        if "--waylandIviLayerId" not in args and "-lid" not in args:
                extendedArgs += " -lid {}".format(DEFAULT_TEST_LAYER)

        if "--waylandIviSurfaceID" not in args and "-sid" not in args:
                extendedArgs += " -sid {}".format(DEFAULT_TEST_SURFACE)

        renderer = self._start_ramses_application(applicationName, extendedArgs, workingDirectory, nameExtension, env, dltAppID)
        if (waitForDisplayManagerRamsh):
            renderer.initialisation_message_to_look_for("Ramsh commands registered from DisplayManager")
        else:
            renderer.initialisation_message_to_look_for("Ramsh commands registered from RamsesRenderer")
        return renderer

    def start_default_renderer(self,  args="", workingDirectory=None, ramsesDaemonTarget=None, nameExtension="", env={}):
        applicationName = "ramses-renderer-{}".format(self.defaultPlatform)
        return self.start_renderer(applicationName, args, workingDirectory, ramsesDaemonTarget, nameExtension, env)

    def start_client(self, applicationName, args="", workingDirectory=None, ramsesDaemonTarget=None, nameExtension="", env={}, dltAppID=None):
        return self._start_ramses_application(applicationName, args, workingDirectory, nameExtension, env, dltAppID)

    def get_install_dir(self):
        return self.ramsesInstallDir

    def _start_ramses_application(self, applicationName, args, workingDirectory, nameExtension, env, dltAppID):
        """ Starts a ramses application
        :param workingDirectory working directory to find and start the application (relative to baseWorkingDirectory
        configured for the target). If None the baseWorkingDirectory is used
        :param dltAppID dlt app id that will be set for the ramses application
        :return: A Application object that can be used to access the stdin, stdout and stderr
        """
        binaryDirectoryOnTarget = self._get_merged_working_directory(workingDirectory)
        extendedArgs = args
        if dltAppID:
            extendedArgs += " -dai "+dltAppID
        extendedArgs += " -l " + str(self.logLevel) +" --enableSmokeTestContext " + " --enableProtocolVersionOffset "
        #use custom daemon port for all ramses applications to avoid connections to other applications running on the system (e.g. the HMI)
        extendedArgs += " -p {}".format(CUSTOM_DAEMON_PORT)
        env['DISABLE_CONSOLE_COLORS'] = '1'
        env['DISABLE_RAMSH_INTERACTIVE_MODE'] = '1'
        application = self.start_application(applicationName, extendedArgs, binaryDirectoryOnTarget, nameExtension, env, dltAppID)
        if self.systemMonitor is not None:
            self.systemMonitor.addApplication(applicationName)
        return application

    @abstractmethod
    def start_application(self, applicationName, args="", binaryDirectoryOnTarget=None, nameExtension="", env={}, dltAppID=None):
        """ Starts an application
        binaryDirectoryOnTarget: use None for applications in system path
        :param dltAppID dlt app id that can be used to capture the output of the application and to send dlt injections.
        It is up to the target implementation if it uses dlt for the communication
        :return: A Application object that contains the stdin, stdout, stderr streams
        """
        pass

    def configureNetworkMonitoring(self, internalNetwork = True, externalNetwork = True):
        # The target should configure any attached SystemMonitor to monitor the specified kind of network.
        # internalNetwork: should monitor network traffic between applications running on this target
        # externalNetwork: should monitor network traffic between different target devices, used for e.g. multi-target tests

        # override this in subclass if hardware platforms differ from these default values
        if self.systemMonitor is not None:
            if internalNetwork:
                self.systemMonitor.addNetworkParameters(interfaceNames = ["lo"])
            if externalNetwork:
                self.systemMonitor.addNetworkParameters(interfaceNames = ["eth0"])

    def save_application_output(self, application, testClassName, testRunName, nr=None):
        """" saves the output of an application to a file

        This method can only be called if application was stopped before.
        If multiple instances of one application are started on one target the nr parameter can be used to
        generate unique filenames
        """

        if not application.started:
            resultDirForTest = self.resultDir+'/'+testClassName+"/"+testRunName
            if nr is None:
                fileName = "{0}_{1}".format(application.name, self.name)
            else:
                fileName = "{0}{1}_{2}".format(application.name, nr, self.name)

            stdoutOutputList = application.get_stdout_data()
            if stdoutOutputList:
                helper.save_text_file(resultDirForTest+"/"+fileName+".txt", "".join(stdoutOutputList))

            stderrOutputList = application.get_stderr_data()
            if stderrOutputList:
                helper.save_text_file(resultDirForTest+"/"+fileName+"_STDERR.txt", "".join(stderrOutputList))
        else:
            log.error("application output can only be saved if application was stopped before")

    def take_screenshot_and_compare(self, renderer, imageName, testClassName, testRunName,
                                    displayNumber, screenshotNumber, percentageOfWrongPixelsAllowed,
                                    percentageOfRGBDifferenceAllowedPerPixel, numberOfRequiredUnequalPixels,
                                    useSystemCompositorForScreenshot, compareForEquality):
        log.info("taking screenshot...")

        referenceImagePath = self._find_reference_image(imageName)
        refImage = Image.open(referenceImagePath)

        minWidth = refImage.size[0]
        minHeight = refImage.size[1]

        pathToResultScreenshot = self._take_and_transfer_screenshot(renderer, imageName, screenshotNumber, testClassName,
                                                                    testRunName, displayNumber, useSystemCompositorForScreenshot,
                                                                    minWidth, minHeight)
        if not os.path.isfile(pathToResultScreenshot):
            log.errorAndAssert("Screenshot not found at {}".format(pathToResultScreenshot))
        else:
            log.info("comparing images...")
            if (self.percentageOfRGBDifferenceAllowedPerPixelOnTarget > percentageOfRGBDifferenceAllowedPerPixel):
                log.important_info("Allowing higher difference per pixel because of target value: {}%".format(self.percentageOfRGBDifferenceAllowedPerPixelOnTarget*100))
                percentageOfRGBDifferenceAllowedPerPixel = self.percentageOfRGBDifferenceAllowedPerPixelOnTarget

            if (self.percentageOfWrongPixelsAllowedOnTarget > percentageOfWrongPixelsAllowed):
                log.important_info("Allowing higher number of wrong pixels because of target value: {}%".format(self.percentageOfWrongPixelsAllowedOnTarget*100))
                percentageOfWrongPixelsAllowed = self.percentageOfWrongPixelsAllowedOnTarget

            helper.compare_images(pathToResultScreenshot, referenceImagePath, percentageOfWrongPixelsAllowed, percentageOfRGBDifferenceAllowedPerPixel, numberOfRequiredUnequalPixels, self.imageDiffScaleFactor, compareForEquality)

    @abstractmethod
    def delete_file(self, fileName):
        pass

    def _take_and_transfer_screenshot(self, renderer, imageName, screenshotNumber, testClassName, testRunName,
                                      displayNumber, useSystemCompositorForScreenshot, minWidth, minHeight):

        splittedImageName = os.path.splitext(imageName)
        localScreenshotName = "{}_{}_{}{}".format(splittedImageName[0], screenshotNumber, self.name, splittedImageName[1])
        targetScreenshotName = "{}{}_{:04d}_{}".format(self.fixed_screenshot_prefix, self.unique_screenshot_prefix, self.target_screenshot_counter, localScreenshotName)
        self.target_screenshot_counter += 1

        # trigger screenshot(s)
        if useSystemCompositorForScreenshot:
            self._take_system_compositor_screenshot(targetScreenshotName, renderer)
        else:
            self._take_renderer_screenshot(targetScreenshotName, renderer, displayNumber)

        resultDirForTest = helper.get_result_dir_subdirectory(self.resultDir, testClassName+"/"+testRunName)

        self._transfer_screenshots(targetScreenshotName, self.tmpDir, resultDirForTest)

        localScreenshotFile = os.path.join(resultDirForTest, localScreenshotName)
        log.info("Store remote {} as local {}".format(targetScreenshotName, localScreenshotFile))
        os.rename(os.path.join(resultDirForTest, targetScreenshotName), localScreenshotFile)

        #check image size
        pilImage = Image.open(localScreenshotFile)
        if (pilImage.size[0] < minWidth) or (pilImage.size[1] < minHeight):
            log.errorAndAssert("Screenshot too small: expected >= {}x{}, got {}x{}".format(minWidth, minHeight, pilImage.size[0], pilImage.size[1]))

        return localScreenshotFile

    @abstractmethod
    def _delete_files_on_target(self, namePattern, directory):
        pass

    @abstractmethod
    def _transfer_screenshots(self, namePattern, originalDirectory, resultDirectory):
        pass

    def _get_ramsh_option_string_for_variant(self, ramshOptionName, value):
        optionString = ""
        if isinstance(value, int):
            optionString += " -{0} {1}".format(ramshOptionName,value)
        if isinstance(value, list):
            optionString += " -" + ramshOptionName + " " + " ".join(map(str,value))
        return optionString

    def _take_renderer_screenshot(self, screenshotName, renderer, displayNumber):
        log.info("Make screenshot of renderer")
        ramshCommand  = "screenshot -filename \"{0}\"".format(self.tmpDir+"/"+screenshotName)
        if displayNumber:
            ramshCommand += " -displayId {}".format(displayNumber)
        #waitForRendererConfirmation cannot be used for screenshot creation as screenshots are enqeued at the renderer and taken on the next renderer loop
        result = renderer.send_ramsh_command(ramshCommand, response_message="screenshot successfully saved to file")
        if not result:
            log.warning("Screenshot confirmation not received from the renderer, check renderer application output")
            #print some debug output
            renderer.send_ramsh_command("rinfo all -v")

    def _take_system_compositor_screenshot(self, screenshotName, renderer):
        log.info("Make screenshot of screen using system compositor")

        targetScreenshotPath = self.tmpDir+"/"+screenshotName
        renderer.send_ramsh_command("scScreenshot \"{}\" {}".format(targetScreenshotPath, self.main_screen_id), response_message="SystemCompositorController_Wayland_IVI::screenshot: Saved screenshot for screen")

        startTime = time.time()
        while time.time() < startTime + SYSTEM_COMPOSITOR_SCREENSHOT_TIMEOUT:
            (_, _, retCode) = self.execute_on_target("find " + targetScreenshotPath)
            if retCode == 0:
                log.info("system_compositor_screenshot: found {}".format(targetScreenshotPath))
                return True
            log.info("system_compositor_screenshot: find result {} since {}s ".format(retCode, time.time()-startTime))
            time.sleep(0.1)

        log.warning("system_compositor_screenshot: failed to take screenshot {}".format(screenshotName))
        return False

    def _find_reference_image(self, imageName):
        result = None
        for d in self.imagesDesiredDirs:
            path = os.path.normpath(d + '/' + imageName)
            if os.path.isfile(path):
                if result is None:
                    result = path
                else:
                    log.errorAndAssert("Multiple reference images found for image name '{}'".format(imageName))
        if result is None:
            log.errorAndAssert("No reference image found for image name '{}'".format(imageName))
        return result

    def _get_merged_env(self, env):
        result = self.defaultEnvironment.copy()
        result.update(env)
        return result

    def _get_merged_working_directory(self, workingDirectory):
        if workingDirectory:
            return posixpath.join(self.baseWorkingDirectory, workingDirectory)
        else:
            return self.baseWorkingDirectory

    def get_process_list(self, filter=None):
        """ Returns a list of processes running on the target
        :param filter filters the result to only processes that contain the filter as substring
        :return: List of strings with one line per process. The line has to contain the name of the process, all other
        content can be target specific
        """
        fullList = self._get_full_process_list()
        if filter:
            return [line for line in fullList if filter in line]
        else:
            return fullList

    @abstractmethod
    def _get_full_process_list(self):
        pass


class BridgedTarget():
    def __init__(self, bridgeTarget):
        self.bridgeTarget = bridgeTarget
