#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import os
import subprocess
import shutil
import glob
import platform
import tempfile
from distutils import spawn

from ramses_test_framework import log
from ramses_test_framework.targets.target import Target
from ramses_test_framework.local_application import LocalApplication

class LocalTarget(Target):

    def __init__(self, targetInfo, ramsesInstallDir, resultDir, imagesDesiredDirs, imageDiffScaleFactor, logLevel):
        Target.__init__(self, targetInfo, ramsesInstallDir, resultDir, imagesDesiredDirs, imageDiffScaleFactor, logLevel)
        self.tmpDir = tempfile.gettempdir()
        self.hostname = "127.0.0.1"

    def setup(self, transfer_binaries=True):
        # add platform specific dll/so search path
        if platform.system() == "Windows":
            self.defaultEnvironment = {'PATH' : os.environ.get('PATH', '') + ";" + os.path.join(self.ramsesInstallDir, 'lib')}
        elif platform.system() == "Linux":
            self.defaultEnvironment = {'LD_LIBRARY_PATH' : os.environ.get('LD_LIBRARY_PATH', '') + ":" + os.path.join(self.ramsesInstallDir, 'lib')}
        else:
            log.warning("Unknown system {}: There might be shared library load errors".format(platform.system()))

        return True

    def start_application(self, applicationName, args="", binaryDirectoryOnTarget= None, nameExtension="", env={}, dltAppID=None):
        if binaryDirectoryOnTarget:
            applicationDirectory = os.path.normcase(binaryDirectoryOnTarget)
            applicationPath = os.path.normcase(applicationDirectory + "/" + applicationName)
        else:
            applicationDirectory = None
            applicationPath = applicationName

        if not spawn.find_executable(applicationPath):
            log.error("executable '{0}' could not be found (path: '{1}')".format(applicationName, applicationPath))
            return LocalApplication(None, applicationName, binaryDirectoryOnTarget)

        command = "{} {}".format(applicationPath, args)

        my_env = dict(os.environ)
        my_env.update(self.defaultEnvironment)
        my_env.update(env)

        popenApp = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                    cwd=applicationDirectory, shell=True, env=my_env)
        application = LocalApplication(popenApp, applicationName, applicationDirectory, nameExtension=nameExtension)
        application.started = True
        return application

    def kill_application(self, application):
        if application.started:
            if platform.system() == "Windows":
                self.execute_on_target("Taskkill /F /T /IM {}.exe".format(application.name))
            elif platform.system() == "Linux":
                self.execute_on_target("killall -9 {}".format(application.name))
            else:
                log.error("Unknown system {}: Application cannot be killed".format(platform.system()))
            application.started = False
            application.stop_readers()

    def _delete_files_on_target(self, namePattern, directory):
        files = glob.glob(os.path.normcase(directory + '/' + namePattern))
        for file in files:
            os.remove(file)

    def _transfer_screenshots(self, filename, originalDirectory, resultDirectory):
        shutil.move(os.path.normcase(originalDirectory + '/' + filename),
                    os.path.join(resultDirectory, os.path.basename(filename)))

    def execute_on_target(self, commandToExecute, block=True, env=None, cwd=None):
        command = commandToExecute.split(' ')
        popenApp = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env, cwd=cwd)
        if block:
            (stdoutdata, stderrdata) = popenApp.communicate() #block till call is finished
            return stdoutdata.splitlines(), stderrdata.splitlines(), popenApp.returncode
        return None

    def copy_file_from_target(self, targetFileName, destFile, targetWorkingDirectory=None):
        targetDirectory = self._get_merged_working_directory(targetWorkingDirectory)
        shutil.copyfile(os.path.join(targetDirectory, targetFileName), destFile)

    def copy_file_to_target(self, sourceFile, targetFileName, targetWorkingDirectory=None):
        targetDirectory = self._get_merged_working_directory(targetWorkingDirectory)
        shutil.copyfile(sourceFile, os.path.join(targetDirectory, targetFileName))

    def delete_file(self, fileName):
        if os.path.exists(fileName):
            os.remove(fileName)

    def _get_full_process_list(self):
        retCode = 0
        if platform.system() == "Windows":
            (out, _, retCode) = self.execute_on_target("tasklist")
        elif platform.system() == "Linux":
            (out, _, retCode) = self.execute_on_target("ps aux")
        else:
            log.error("Unknown system {}: Process list cannot be retrieved".format(platform.system()))
            out = []
        assert retCode == 0, "_get_full_process_list failed"

        return out
