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

from ramses_test_framework import log


class PowerDevice:
    __metaclass__ = ABCMeta

    def __init__(self, url, username, password):
        self.url = url
        self.username = username
        self.password = password

    @abstractmethod
    def switch(self, outletNr, on):
        pass


class NETIOPowerDevice(PowerDevice):
    def switch(self, outletNr, on):
        log.info("Connecting to power device {0} user: {1}".format(self.url, self.username))
        #create telnet connection to power outlet
        try:
            tn = telnetlib.Telnet(self.url, 1234)
        except:
            log.error("Connection to power device {0} could not be established".format(self.url))
        else:
            #tn.read_until("100 HELLO 00000000 - KSHELL V1.5")

            tn.write("login {0} {1}\n".format(self.username, self.password))

            #turn power outlet on or off
            status = 0
            if on:
                status = 1
            tn.write("port {0} {1}\n".format(outletNr, status))

            tn.write("quit\n")
            time.sleep(1)
            log.info(tn.read_eager())
