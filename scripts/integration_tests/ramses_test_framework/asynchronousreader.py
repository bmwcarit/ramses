#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import threading
import time

from ramses_test_framework import log


class AsynchronousPipeReader(threading.Thread):
    def __init__(self, pipe, buffer, name=""):
        threading.Thread.__init__(self)
        self._pipe = pipe
        self._buffer = buffer
        self._name = name
        self.daemon = True
        self.startedLock = threading.Lock()
        self.started = True
        self.abortTime = None
        self.start()

    def run(self):
        while True:
            self.startedLock.acquire()
            doStop = (self.started is False)
            self.startedLock.release()

            if doStop:
                if (self.abortTime is not None) and (time.time() >= self.abortTime):
                    log.error("Timeout while trying to read remaining contents of pipe ({}). Current buffer content\n----------------\n{}\n----------------".
                              format(self._name, self._buffer.get_all_data()))
                    return

            line = self._pipe.readline()
            if not isinstance(line, str):
                # ignore ascii->utf-8 errors without failing
                line = line.decode('utf-8', 'replace')  # TODO(tobias) check if some targets (VDT, others?) send valid utf-8 already

            if line == "":
                if doStop:
                    return  # pipe is empty, we can stop reader immediately
                else:
                    time.sleep(0.1)  # pipe currently empty, wait for new data
            else:
                self._buffer.append(line)

    def stop(self, withTimeout=True):

        self.startedLock.acquire()
        self.abortTime = time.time() + 180.0
        self.started = False
        self.startedLock.release()
        if withTimeout:
            self.join(185.0)  # timeout > 3 minutes, less than job timeout and more than abortTime but enough to prevent false positives
        else:
            self.join()
        if self.isAlive():
            raise RuntimeError("AsynchronousPipeReader: failed to join for {}".format(self._name))
