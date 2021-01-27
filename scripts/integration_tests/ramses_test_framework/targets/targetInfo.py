#  -------------------------------------------------------------------------
#  Copyright (C) 2015 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from builtins import object
import re

class TargetInfo(object):

    def __init__(self, classname, name, hostname, username, buildJobName, powerDevice=None, powerOutletNr=None, privateKey=None, password=None,
                 systemMonitorClassname=None, sshPort=22, someIPTestsInterfaceIp=None, tcpTestsInterfaceIp=None):
        """ To use no password for the authentication (key-based) leave password to None!
            If a empty string is passed as password it will be used as password """

        if re.match(r'^[\w-]+$', name) is None:
            raise RuntimeError("Target name '{}' not allowed".format(name))
        self.classname = classname
        self.name = name # used to generate the names of the tests cases
        self.hostname = hostname
        self.username = username
        self.buildJobName = buildJobName
        self.powerDevice = powerDevice
        self.powerOutletNr = powerOutletNr
        self.password = password
        self.systemMonitorClassname = systemMonitorClassname
        self.sshPort = sshPort
        self.privateKey = privateKey

        if someIPTestsInterfaceIp:
            self.someIPTestsInterfaceIp = someIPTestsInterfaceIp
        else:
            self.someIPTestsInterfaceIp = hostname
        if tcpTestsInterfaceIp:
            self.tcpTestsInterfaceIp = tcpTestsInterfaceIp
        else:
            self.tcpTestsInterfaceIp = hostname


class BridgedTargetInfo(TargetInfo):
    def __init__(self, targetInfoBridgeTarget, classname, name, hostname, username, buildJobName, powerDevice=None, powerOutletNr=None, privateKey=None, password=None,
                 systemMonitorClassname=None, sshPort=22, someIPTestsInterfaceIp=None, tcpTestsInterfaceIp=None):
        TargetInfo.__init__(self, classname, name, hostname, username, buildJobName, powerDevice, powerOutletNr, privateKey,
                            password, systemMonitorClassname, sshPort, someIPTestsInterfaceIp, tcpTestsInterfaceIp)
        self.targetInfoBridgeTarget = targetInfoBridgeTarget

