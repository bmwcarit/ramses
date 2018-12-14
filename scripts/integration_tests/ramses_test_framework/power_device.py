#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import telnetlib
import time
from ramses_test_framework import log

class PowerDevice:
    def __init__(self, url, username, password):
        self.url = url
        self.username = username
        self.password = password

    def createTelnetConnection(self):
        log.info("Connecting to power device {0} user: {1}".format(self.url, self.username))
        #create telnet connection to power outlet
        try:
            telnet_conn = telnetlib.Telnet(self.url, 1234)
            return telnet_conn
        except:
            log.error("Connection to power device {0} could not be established".format(self.url))
        return None

    def switch(self, outletNr, on):
        #self.telnet_connection.read_until("100 HELLO 00000000 - KSHELL V1.5")
        telnet_conn = self.createTelnetConnection()
        if not telnet_conn:
            return False

        assert(telnet_conn)
        telnet_conn.write("login {0} {1}\n".format(self.username, self.password))

        #turn power outlet on or off
        status = 0
        if on:
            status = 1
        telnet_conn.write("port {0} {1}\n".format(outletNr, status))

        telnet_conn.write("quit\n")
        time.sleep(1)
        log.info(telnet_conn.read_eager())

        telnet_conn.close()
        return True
