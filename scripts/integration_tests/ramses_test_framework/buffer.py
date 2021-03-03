
#  -------------------------------------------------------------------------
#  Copyright (C) 2015 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from builtins import object
from builtins import range
from builtins import str
import threading
import time
import copy
import re

from ramses_test_framework import log


class Buffer(object):
    def __init__(self):
        self.internalBuffer = []
        self.lock = threading.Lock()

    def append(self, line):
        self.lock.acquire()
        self.internalBuffer.append(line)
        self.lock.release()

    def get_length(self):
        self.lock.acquire()
        length = len(self.internalBuffer)
        self.lock.release()
        return length

    def get_line(self, index):
        self.lock.acquire()
        line = self.internalBuffer[index]
        self.lock.release()
        return line

    def get_all_data(self):
        self.lock.acquire()
        content = copy.copy(self.internalBuffer)
        self.lock.release()
        return content


class BufferWatcher(object):
    def __init__(self, buffer):
        self.buffer = buffer
        self.watchStartPositions = dict()
        self.watchCount = 0

    def start_watch(self):
        # unique id for each start_watch
        self.watchCount += 1
        self.watchStartPositions[self.watchCount] = self.buffer.get_length()
        return self.watchCount

    def get_watch_from_beginning(self):
        self.watchCount += 1
        self.watchStartPositions[self.watchCount] = 0
        return self.watchCount

    def is_active(self, watch_id):
        return watch_id in self.watchStartPositions

    def remove_watch(self, watch_id):
        del self.watchStartPositions[watch_id]

    def wait_for_msg(self, watch_id, msg, timeout=None):
        """
        Blocks until msg found or timeout
        :return: True if msg found during watch, False otherwise
        """

        # don't wait if no watch_id
        if watch_id is None:
            log.error("Cannot wait! No watch_id given!")
            return False

        # don't wait if watch not active
        if not self.is_active(watch_id):
            log.error("Cannot wait! Reader has no active watch with given id: {}!".format(watch_id))
            return False

        timer = None
        if timeout:
            timer = threading.Timer(timeout, self.remove_watch, [watch_id])
            timer.start()
            if msg is None:
                log.warning("Msg not given! Will block until timeout")
                timer.join()
                return False
        elif msg:
            log.warning("Timeout not given! Will block until message received.\n"
                        "(Note: if msg not found, will block forever.)")
        else:
            # don't wait if not timer and no msg given
            log.error("Cannot wait! Nor timeout or msg given.")
            return False

        # check data in buffer that is available until now
        nextLinePos = self.buffer.get_length()
        for i in range(self.watchStartPositions[watch_id], nextLinePos):
            if not self.is_active(watch_id):
                log.warning("Timeout ({} seconds) waiting for msg:\'{}\' in reader: {}.".format(str(timeout), msg, self.name))
                return False
            line = self.buffer.get_line(i)
            if re.search(msg, line):
                self.remove_watch(watch_id)
                if timer:
                    timer.cancel()
                return True

        # wait for new data
        while self.is_active(watch_id):
            bufferSize = self.buffer.get_length()
            if nextLinePos >= bufferSize:
                time.sleep(0.1)
            else:
                line = self.buffer.get_line(nextLinePos)
                if re.search(msg, line):
                    self.remove_watch(watch_id)
                    if timer:
                        timer.cancel()
                    return True
                else:
                    nextLinePos += 1
        log.warning("Timeout ({} seconds) waiting for msg:\'{}\'".format(str(timeout), msg))
        return False
