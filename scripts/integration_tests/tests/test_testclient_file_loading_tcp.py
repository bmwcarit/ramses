#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from tests.file_loading_base import file_loading_base

class TestClientFileLoadingTcp(file_loading_base.TestClientFileLoadingBase):
    def __init__(self, methodName='runTest'):
        file_loading_base.TestClientFileLoadingBase.__init__(self, methodName)
