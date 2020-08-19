#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import os
import stat
import common_modules.common as cc

def check_file_attributes(filename):
    """ Check for unwanted file attributes """

    mode = os.stat(filename).st_mode
    if bool(mode & stat.S_IXUSR) or bool(mode & stat.S_IXGRP) or bool(mode & stat.S_IXOTH):
        cc.log_warning("check_file_attributes", filename, 0, "may not have file executable bits set", "")
