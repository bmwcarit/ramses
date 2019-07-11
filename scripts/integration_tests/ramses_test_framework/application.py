#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from ramses_test_framework.asynchronousreader import AsynchronousPipeReader
from ramses_test_framework.buffer import Buffer, BufferWatcher
from ramses_test_framework import log

class Application:

    DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT = 60

    def __init__(self, stdin, stdout, stderr, name, workingDirectoryAbsolute, nameExtension=""):
        self.stdin = stdin
        self.stdout = stdout
        self.stderr = stderr
        self.name = name # executable name
        # description name so one can make a difference between two applications started from the same executable
        self.extendedName = name + nameExtension;
        self.started = False
        self.initialised = True
        self.initialisationMessage = None
        self.initialisationWatchID = None
        self.stdoutReader = None
        self.stdoutBuffer = None
        self.stdoutBufferWatcher = None
        self.stderrReader = None
        self.stderrBuffer = None
        self.stderrBufferWatcher = None
        self.workingDirectoryAbsolute = workingDirectoryAbsolute
        self.rendererConfirmationNr = 0
        if self.stdout:
            self.stdoutBuffer = Buffer()
            self.stdoutBufferWatcher = BufferWatcher(self.stdoutBuffer)
            self.initialisationWatchID = self.start_watch_stdout()
            self.stdoutReader = AsynchronousPipeReader(self.stdout, self.stdoutBuffer, "stdoutReader-"+self.name)

        if self.stderr:
            self.stderrBuffer = Buffer()
            self.stderrBufferWatcher = BufferWatcher(self.stderrBuffer)
            self.stderrReader = AsynchronousPipeReader(self.stderr, self.stderrBuffer, "stderrReader-"+self.name)

    def send_ramsh_command(self, command, response_message=None, timeout=DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT, waitForRendererConfirmation=False):
        """optional arguments:
        response_message -- waits after sending command until response_message(string) appears in application stdout
            Cannot be used in combination with waitForRendererConfirmation
            response_message is interpreted as regular expression
        timeout -- maximum time (in seconds) to wait for response message. If set to None method will block
            waitForRendererConfirmation: adds a second command in command queue of renderer to confirm that given command was
            executed. Can only be used with renderer executables and cannot be used in combination with response_message
            until response message is received without timeout
        @:return True if requested response_message or renderer confirmation have been received (or none of them was set)
        """
        if response_message and waitForRendererConfirmation:
            log.error("response_message and waitForRendererConfirmation cannot be used in combination")
            return False

        if self.started and self.is_initialised(Application.DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT):
            if response_message or waitForRendererConfirmation:
                watchId = self.start_watch_stdout()
            else:
                watchId = None

            self._internal_send_ramsh_command(command)

            if waitForRendererConfirmation:
                self.rendererConfirmationNr += 1
                self._internal_send_ramsh_command("confirm {}".format(self.rendererConfirmationNr))

            if response_message:
                return self.wait_for_msg_in_stdout(watchId, response_message, timeout)
            elif waitForRendererConfirmation:
                log.info("waiting for renderer confirmation number {}".format(self.rendererConfirmationNr))
                return self.wait_for_msg_in_stdout(watchId, "confirmation: {}".format(self.rendererConfirmationNr), timeout)
            else:
                return True
        else:
            log.error("command cannot be executed, as application was not started correctly")
            return False

    def showScene(self, sceneId, displayId=0, blockUntilShown=True, timeout=DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT):
        ramshCommand = "showSceneOnDisplay -sceneId {0} -displayId {1}".format(sceneId, displayId)
        if  blockUntilShown:
            confirm_message = "scene_{}_shown".format(sceneId)
            ramshCommand += " -confirm " + confirm_message
            log.info("sending showSceneOnDisplay command for scene {}, waiting for confirmation".format(sceneId))
            success = self.send_ramsh_command(ramshCommand, response_message="confirmation: " + confirm_message, timeout=timeout)
            if success:
                log.info("received confirmation of showSceneOnDisplay command for scene {}".format(sceneId))
            else:
                self.send_ramsh_command("rinfo")
            assert success, "Timeout waiting for show event of scene, check rinfo output in renderer log"
        else:
            self.send_ramsh_command(ramshCommand, waitForRendererConfirmation=True, timeout=timeout)

    def _internal_send_ramsh_command(self, command):
        self.stdin.write(command+"\n")
        #just to be sure
        self.stdin.flush()

    def _get_buffer_data(self, buffer):
        if buffer:
            return buffer.get_all_data()
        else:
            return None

    def start_watch_stdout(self):
        return self.stdoutBufferWatcher.start_watch()

    def wait_for_msg_in_stdout(self, watch_id=None, msg=None, timeout=None):
        return self.stdoutBufferWatcher.wait_for_msg(watch_id, msg, timeout)

    def wait_for_msg_in_stdout_from_beginning(self, msg=None, timeout=None):
        return self.stdoutBufferWatcher.wait_for_msg(self.stdoutBufferWatcher.get_watch_from_beginning(), msg, timeout)

    def get_stdout_data(self):
        return self._get_buffer_data(self.stdoutBuffer)

    def get_stderr_data(self):
        return self._get_buffer_data(self.stderrBuffer)

    def stop_readers(self, withTimeout=True):
        if self.stdout:
            self.stdoutReader.stop(withTimeout)
        if self.stderr:
            self.stderrReader.stop(withTimeout)

    def initialisation_message_to_look_for(self, msg):
        self.initialisationMessage = msg
        self.initialised = False

    def is_initialised(self, timeout=None):
        if self.initialisationMessage and not self.initialised and self.initialisationWatchID:
            log.info("waiting for initialisation message")
            self.initialised = self.wait_for_msg_in_stdout(self.initialisationWatchID, self.initialisationMessage, timeout)
            if self.initialised:
                log.info("initialisation message received")
        return self.initialised

    #def shutdown(self):
        #todo send Ramsh command for controlled shutdown

    def get_return_code_blocking(self):
        returnCode = self.stdout.channel.recv_exit_status()
        self.started = False
        self.stop_readers(withTimeout=False)

        return returnCode

    def block_until_ended(self):
        self.get_return_code_blocking()
