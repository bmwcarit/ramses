#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import time
import paramiko
import socket
import os
import subprocess
import posixpath
import glob

from paramiko.ssh_exception import SSHException, AuthenticationException, BadHostKeyException, PasswordRequiredException

from ramses_test_framework import log
from ramses_test_framework.application import Application
from ramses_test_framework.targets.target import Target
from ramses_test_framework.asynchronousreader import AsynchronousPipeReader
from ramses_test_framework.buffer import Buffer
from ramses_test_framework import helper


class RemoteTarget(Target):

    def __init__(self, targetInfo, ramsesInstallDir, resultDir, imagesDesiredDirs, imageDiffScaleFactor,
                 basePath,
                 sshConnectionNrAttempts, sshConnectionTimeoutPerAttempt,
                 sshConnectionSleepPerAttempt, powerNrAttempts, logLevel):
        Target.__init__(self, targetInfo, ramsesInstallDir, resultDir, imagesDesiredDirs, imageDiffScaleFactor, logLevel)
        self.basePath = basePath
        self.privateKey = targetInfo.privateKey
        self.sshConnectionNrAttempts = sshConnectionNrAttempts
        self.sshConnectionTimeoutPerAttempt = sshConnectionTimeoutPerAttempt
        self.sshConnectionSleepPerAttempt = sshConnectionSleepPerAttempt
        self.powerNrAttempts = powerNrAttempts
        self.executableExistsOnTarget = {}

    def _start_ramses_application(self, applicationName, args, workingDirectory, nameExtension, env, dltAppID):
        extendedArgs = args+" -myip "+self.tcpTestsInterfaceIp
        if self.tcpAliveIntervalMs:
            extendedArgs += " -tcpAlive {}".format(self.tcpAliveIntervalMs)
        if self.tcpAliveTimeoutMs:
            extendedArgs += " -tcpAliveTimeout {}".format(self.tcpAliveTimeoutMs)
        return Target._start_ramses_application(self, applicationName, extendedArgs, workingDirectory, nameExtension, env, dltAppID)

    def _executable_exists_on_target(self, binaryPath):
        if binaryPath in self.executableExistsOnTarget:
            return self.executableExistsOnTarget[binaryPath]
        (_, _, exitCode) = self.execute_on_target("type " + binaryPath)  # test -e cannot be used as it does not work for applications in system path
        result = (exitCode == 0)
        self.executableExistsOnTarget[binaryPath] = result
        return result

    def start_application(self, applicationName, args="", binaryDirectoryOnTarget=None, nameExtension="", env={}, dltAppID=None):
        #ensure binary is there
        if binaryDirectoryOnTarget:
            binaryPathOnTarget = binaryDirectoryOnTarget + '/' + applicationName
        else:
            binaryPathOnTarget = applicationName
        if not self._executable_exists_on_target(binaryPathOnTarget):
            log.error("Error: executable '{0}' could not be found (path: '{1}')".format(applicationName, binaryPathOnTarget))
            return Application(None, None, None, applicationName, binaryDirectoryOnTarget, nameExtension)

        prefix = helper.get_env_var_setting_string(self._get_merged_env(env))

        #execute application
        if binaryDirectoryOnTarget:
            command = "cd {}; {} ./{} {}".format(binaryDirectoryOnTarget, prefix, applicationName, args)
        else:
            command = "{} {} {}".format(prefix, applicationName, args)

        log.info("start_application command: '{}'".format(command))
        try:
            stdin, stdout, stderr = self.sshClient.exec_command(command)
        except Exception as e:
            log.error("Error: {0} could not be started (error message: {1})".format(applicationName, e.message))
            return Application(None, None, None, applicationName, binaryDirectoryOnTarget, nameExtension)

        application = Application(stdin, stdout, stderr, applicationName, binaryDirectoryOnTarget, nameExtension)
        application.started = True

        return application

    def _get_daemon_args(self, ramsesDaemonTarget):
        if ramsesDaemonTarget != None:
            return " --daemon-ip "+ramsesDaemonTarget.hostname
        else:
            return ""

    def start_renderer(self, applicationName, args="", workingDirectory=None, ramsesDaemonTarget=None, nameExtension="", env={}, dltAppID='REND', waitForDisplayManagerRamsh=True):
        extendedArgs = args + self._get_daemon_args(ramsesDaemonTarget)
        return Target.start_renderer(self, applicationName, extendedArgs, workingDirectory, ramsesDaemonTarget, nameExtension, env, dltAppID, waitForDisplayManagerRamsh)

    def start_client(self, applicationName, args="", workingDirectory=None, ramsesDaemonTarget=None, nameExtension="", env={}, dltAppID=None):
        extendedArgs = args + self._get_daemon_args(ramsesDaemonTarget)
        return Target.start_client(self, applicationName, extendedArgs, workingDirectory, ramsesDaemonTarget, nameExtension, env, dltAppID)

    def kill_application(self, application):
        if application.started:
            self.execute_on_target("killall -9 " + application.name)
            application.started = False
            application.stop_readers()

    def get_install_dir(self):
        return "~/{}".format(self.ramsesInstallDir)

    def _delete_files_on_target(self, namePattern, directory):
        self.execute_on_target("rm -f {}/{}".format(directory, namePattern))

    def _transfer_screenshots(self, namePattern, originalDirectory, resultDirectory):
        sourcePath = originalDirectory + '/' + namePattern
        log.info("download screenshot(s): from {}:{} to {}".format(self.hostname, sourcePath, resultDirectory))
        self._scp(sourcePath, True, resultDirectory, False)

    def setup(self, transfer_binaries=True):
        nrAttempts = 0
        while (not self.isConnected) and (nrAttempts < self.powerNrAttempts):
            nrAttempts += 1
            self.connect()
            #have you tried turning it off and on again?
            if not self.isConnected and (nrAttempts < self.powerNrAttempts): #no need to try it after last connection attempt
                self._power_reset()

        if not self.isConnected:
            return False

        if transfer_binaries:
            self._prepare_install_directory()
            binariesTransferSuccessful = self._transfer_binaries()
            if not binariesTransferSuccessful:
                return False

        return True

    def target_specific_tear_down(self, shutdown=True):
        try:
            Target.target_specific_tear_down(self, shutdown=shutdown)
            if self.isConnected:
                if shutdown:
                    self._shutdown()
                self.sshClient.close()
        except SSHException as e:
            log.warning('Paramiko exception during target_specific_tear_down: ' + str(e))

    def connect(self, error_on_fail=True):
        self.sshClient = paramiko.SSHClient()
        self.sshClient.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        log.info("connecting to "+self.name+" hostname: "+self.hostname+" user: "+self.username)

        rsa_key = None
        if self.privateKey is not None:
            rsa_key = paramiko.RSAKey.from_private_key_file(self.privateKey)

        # wait till SSH connection can be established
        nrAttempts = 0
        while (not self.isConnected) and (nrAttempts < self.sshConnectionNrAttempts):
            try:
                nrAttempts += 1
                log.info("Trying to connect to {} port {} for the {} time.".format(self.hostname, self.sshPort, nrAttempts))
                self.sshClient.connect(self.hostname, port=self.sshPort, username=self.username, password=self.password,
                                       pkey=rsa_key, timeout=self.sshConnectionTimeoutPerAttempt, allow_agent=False,
                                       look_for_keys=False, banner_timeout=45)
                transport = self.sshClient.get_transport()
                transport.set_keepalive(5)

            except (socket.error, SSHException) as e:
                log.warning("Connection could not be established, maybe not ready yet: {0}".format(e))
                time.sleep(self.sshConnectionSleepPerAttempt)  # server not yet ready
            except (BadHostKeyException, AuthenticationException) as e:
                log.error("Connection error: {0}".format(e))
                break
            else:
                self.isConnected = True

        if not self.isConnected:
            if error_on_fail:
                log.error("Connection to {0} could not be established".format(self.name))
            return

        self.ftpClient = self.sshClient.open_sftp()
        log.info("Connection to {0} successfully established".format(self.name))

    def _power_reset(self):
        if self.powerDevice:
            self.powerDevice.switch(self.powerOutletNr, False)
            time.sleep(60)
            self.powerDevice.switch(self.powerOutletNr, True)

    def _prepare_install_directory(self):
        #create result dir if it does not exist
        (_, _, resultTest) = self.execute_on_target("test -e " + self.ramsesInstallDir)
        if resultTest != 0:
            self.execute_on_target("mkdir -p " + self.ramsesInstallDir)
        #make sure it is empty (delete old binaries from previous tests)
        self.execute_on_target("rm -r {0}/*".format(self.ramsesInstallDir))

    def _scp(self, source, sourceIsRemote, dest, destIsRemote, dest_has_filename=False):
        assert not (sourceIsRemote and destIsRemote)
        assert source.find('*') == -1
        assert dest.find('*') == -1

        if not dest_has_filename:
            dest = dest + '/' + os.path.basename(source)

        try:
            if sourceIsRemote:
                log.info("copy remote source '{}' to local dest '{}'".format(source, dest))
                self.ftpClient.get(source, dest)
            if destIsRemote:
                log.info("copy local source '{}' to remote dest '{}'".format(source, dest))
                self.ftpClient.put(source, dest)
        except Exception as e:
            print('SCP Error:', e)
            raise

    def _transfer_binaries(self):
        packageBaseName = self.buildJobName+'-*'

        # glob to get single expected archive for target, otherwise fail
        resultList = glob.glob("{0}/{1}".format(self.basePath, packageBaseName))
        if not resultList:
            log.error("no package found for filter \"{}\"".format(packageBaseName))
            return False
        elif len(resultList) > 1:
            log.error("too many packages found for filter \"{}\": {}".format(packageBaseName, resultList))
            return False

        packagePathOnBuildServer = "{0}/{1}".format(self.basePath, os.path.basename(resultList[0]))
        packagePathOnTarget = "{0}/{1}".format(self.ramsesInstallDir, os.path.basename(resultList[0]))

        # transfer package
        self._scp(packagePathOnBuildServer, False, self.ramsesInstallDir, True)

        #extract package
        stdout, stderr, returnCode = self.execute_on_target(
            "tar mxf {0} -C {1}".format(packagePathOnTarget, self.ramsesInstallDir), block=True)
        if returnCode != 0:
            log.error("tar extraction not successful, return code: {}, stdout: {}, stderr: {}".
                      format(returnCode, "".join(stdout), "".join(stderr)))
            return False

        #remove tar
        self.execute_on_target("rm {0}".format(packagePathOnTarget))

        #move contents from subfolder directly to install dir
        self.execute_on_target("mv {0}/{1}/* {0}".format(self.ramsesInstallDir, packageBaseName), block=True)

        return True

    def _shutdown(self):
        self.execute_on_target("shutdown -h 0", False)

    def execute_on_target(self, commandToExecute, block=True, env={}, cwd=None, timeout=None):
        prefix = helper.get_env_var_setting_string(env)
        command = "{} ".format(prefix) + commandToExecute
        if cwd:
            command = "cd {}; ".format(cwd) + command

        log.info("[{}]{}".format(self.name, command))
        stdin, stdout, stderr = self.sshClient.exec_command(command)
        stdoutBuffer = Buffer()
        stdoutReader = AsynchronousPipeReader(stdout, stdoutBuffer)
        stderrBuffer = Buffer()
        stderrReader = AsynchronousPipeReader(stderr, stderrBuffer)

        if block:
            if timeout:
                # poll on application exit with timeout to prevent infinite blocking
                endTime = time.time() + timeout
                while time.time() < endTime and not stdout.channel.exit_status_ready():
                    time.sleep(0.1)
                if not stdout.channel.exit_status_ready():
                    return [], ["<timeout>"], 1

            # get application exit status blocking. will immediately succeed when timeout was given
            returnCode = stdout.channel.recv_exit_status()
            stdoutReader.stop(withTimeout=True)
            stderrReader.stop(withTimeout=True)
            return stdoutBuffer.get_all_data(), stderrBuffer.get_all_data(), returnCode
        return None

    def copy_file_from_target(self, targetFileName, destFile, targetWorkingDirectory=None):
        fullTargetFileName = os.path.join(self._get_merged_working_directory(targetWorkingDirectory), targetFileName)
        # transfer file
        self._scp(fullTargetFileName, True, destFile, False)

    def copy_file_to_target(self, sourceFile, targetFileName, targetWorkingDirectory=None):
        fullTargetFileName = posixpath.join(self._get_merged_working_directory(targetWorkingDirectory), targetFileName)
        # transfer file
        self._scp(sourceFile, False, fullTargetFileName, True, dest_has_filename=True)

    def delete_file(self, fileName):
        self.execute_on_target("rm {0}".format(fileName))

    def _get_full_process_list(self):
        (out, _, retCode) = self.execute_on_target("ps aux")
        assert retCode == 0, "_get_full_process_list failed"
        return out
