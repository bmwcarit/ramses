#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import telnetlib
import time
from abc import ABCMeta, abstractmethod
from future.utils import with_metaclass
import sys
import socket

from ramses_test_framework import log


class PowerDevice(with_metaclass(ABCMeta)):
    def __init__(self, url, username, password):
        self.url = url
        self.username = username
        self.password = password

    @abstractmethod
    def switch(self, outletNr, on):
        pass


class NETIOPowerDevice(PowerDevice):
    def __init__(self, url, username, password):
        self.url = url
        self.username = username
        self.password = password

    def createTelnetConnection(self):
        log.info("Connecting to power device {0} user: {1}".format(self.url, self.username))
        # create telnet connection to power outlet
        try:
            telnet_conn = telnetlib.Telnet(self.url, 1234)
            return telnet_conn
        except (ConnectionRefusedError, socket.timeout):
            log.error("Connection to power device {0} could not be established".format(self.url))
        return None

    def switch(self, outletNr, on):
        # self.telnet_connection.read_until("100 HELLO 00000000 - KSHELL V1.5")
        telnet_conn = self.createTelnetConnection()
        if not telnet_conn:
            return False

        self._write(telnet_conn, "login {0} {1}\n".format(self.username, self.password))

        # turn power outlet on or off
        status = 0
        if on:
            status = 1
        self._write(telnet_conn, "port {0} {1}\n".format(outletNr, status))

        self._write(telnet_conn, "quit\n")
        time.sleep(1)
        log.info(telnet_conn.read_eager())

        return True

    def _write(self, conn, command):
        assert(conn)
        if sys.version_info < (3, 0):
            command = bytes(command)
        else:
            command = bytes(command, 'utf8')
        conn.write(command)
