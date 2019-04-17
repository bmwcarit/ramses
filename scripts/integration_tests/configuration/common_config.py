#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import os

class Config:
    def __init__(self):
        self.scriptDir = os.path.realpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), ".."))

        # Local paths (i.e. on build server), relative to base path
        self.testResultsDir = 'test-results'  # directory where test results should be put

        # Local paths, relative to scriptDir (integration_tests directory)
        self.testDirs = [os.path.join(self.scriptDir, 'tests')]  # list of directories containing tests
        self.testPattern = 'test*.py'
        self.imagesDesiredDirs = [os.path.join(self.scriptDir, 'images_desired')]  # list of directories containing desired images

        self.imageDiffScaleFactor = 100 #scale factor for the scaled diff images

        # common switches
        self.ramsesApplicationLogLevel = 5
