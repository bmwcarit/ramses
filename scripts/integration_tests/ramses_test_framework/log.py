#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


from __future__ import print_function
import platform
import os

#some color enumeration for text printing
black, red, green, orange, blue, purple, cyan, light_gray, gray, light_red, light_green, yellow, light_blue, light_cyan = range(14)

enableLogColors = platform.system() != "Windows"

#internal class for text unit codes
class text:
    endc = '\033[0m'
    bold = '\033[1m'

    #foreground
    class fg:
        black = '\033[30m'
        red = '\033[31m'
        green = '\033[32m'
        orange = '\033[33m'
        blue = '\033[34m'
        purple = '\033[35m'
        cyan = '\033[36m'
        light_grey = '\033[37m'
        grey = '\033[90m'
        light_red = '\033[91m'
        light_green = '\033[92m'
        yellow = '\033[93m'
        light_blue = '\033[94m'
        pink = '\033[95m'
        light_cyan = '\033[96m'

    #background
    class bg:
        black = '\033[40m'
        red = '\033[41m'
        green = '\033[42m'
        orange = '\033[43m'
        blue = '\033[44m'
        purple = '\033[45m'
        cyan = '\033[46m'
        light_grey = '\033[47m'
        grey = '\033[100m'
        light_red = '\033[101m'
        light_green = '\033[102m'
        yellow = '\033[103m'
        light_blue = '\033[104m'
        pink = '\033[105m'
        light_cyan = '\033[106m'

    foreground = [fg.black, fg.red, fg.green, fg.orange, fg.blue, fg.purple, fg.cyan, fg.light_grey, fg.grey, fg.light_red, fg.light_green, fg.yellow, fg.light_blue, fg.light_cyan]
    background = [bg.black, bg.red, bg.green, bg.orange, bg.blue, bg.purple, fg.cyan, bg.light_grey, bg.grey, bg.light_red, bg.light_green, bg.yellow, bg.light_blue, bg.light_cyan]


class FileLogger:
    def __init__(self, pathToFile):
        dirName = os.path.dirname(pathToFile)
        if not os.path.exists(dirName):
            os.makedirs(dirName)
        self.logFile = open(pathToFile, 'w')

    def __del__(self):
        if self.logFile:
            self.logFile.close()

    def log(self, msg):
        self.logFile.write(msg+"\n")

    def close(self):
        self.logFile.close()
        self.logFile = None

default_file_logger = None

#internal(private) method; should not be used outside this class
def _internal_print(msg, bold=False, foreground='', background=''):
    if enableLogColors and (bold or foreground or background):
        b = ""
        if bold:
            b = text.bold
        print(text.endc + b + foreground + background + msg + text.endc)
    else:
        print(msg)

    if default_file_logger:
        default_file_logger.log(msg)


#public logging methods
def info(msg):
    _internal_print(msg)


def important_info(msg):
    _internal_print(msg, True)


def warning(msg):
    _internal_print("Warning: "+msg, True, foreground=text.fg.orange)


def error(msg):
    _internal_print("Error: "+msg, True, foreground=text.fg.red)

def errorAndAssert(msg):
    """
    Logs an error and then asserts
    This method can be used to make the test fail
    """
    error(msg)
    assert False, msg

def color_separator(color, txt=''):
    smart_print(txt, backcolor=color)


def separator(ch):
    important_info(ch*100)


def smart_print(msg, forecolor=None, backcolor=None, bold=False):
    fc = ''
    bc = ''
    if forecolor is not None:
        fc = text.foreground[forecolor]
    if backcolor is not None:
        bc = text.background[backcolor]
    _internal_print(msg, bold, fc, bc)


def strip_color_codes_from_file(path):
    f = open(path, 'r')
    content = f.read()
    f.close()

    toReplace =  [text.endc, text.bold]
    toReplace.extend(text.foreground)
    toReplace.extend(text.background)
    for item in toReplace:
        content = content.replace(item, "")

    f = open(path, 'w')
    f.write(content)
    f.close()
