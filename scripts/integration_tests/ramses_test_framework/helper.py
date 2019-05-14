#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import shutil
import os
from PIL import Image
from urllib2 import urlopen, HTTPError
import re
import codecs

from ramses_test_framework import image_utils
from ramses_test_framework import log


def compare_images(screenshotPath, desiredImagePath, percentageOfWrongPixelsAllowed, percentageOfRGBDifferenceAllowedPerPixel, numberOfRequiredUnequalPixels, imageDiffScaleFactor, compareForEquality):
    try:
        image = Image.open(screenshotPath)
        desiredImage = Image.open(desiredImagePath)

    except IOError as e:
        log.errorAndAssert("Image file for comparison could not be opened ({} {})".format(e.strerror, e.filename))

    #crop image if bigger than desiredImage
    if (image.size[0] > desiredImage.size[0] and image.size[1] >= desiredImage.size[1]) or \
        (image.size[1] > desiredImage.size[1] and image.size[0] >= desiredImage.size[0]):
        log.info("Image is bigger than desired image, using cropped image")
        splittedImagePath = os.path.splitext(screenshotPath)
        croppedImagePath = splittedImagePath[0]+"_cropped"+splittedImagePath[1]
        image = image.crop((0, 0, desiredImage.size[0], desiredImage.size[1]))
        image.save(croppedImagePath)
    if compareForEquality:
        log.info("compareEqual: Bitmap compare of {0} with {1}".format(screenshotPath, desiredImagePath))
        result = image_utils.compareEqual(image, desiredImage, percentageOfWrongPixelsAllowed, percentageOfRGBDifferenceAllowedPerPixel)
    else:
        log.info("compareUnequal: Bitmap compare of {0} with {1}".format(screenshotPath, desiredImagePath))
        result = image_utils.compareUnequal(image, desiredImage, numberOfRequiredUnequalPixels, percentageOfRGBDifferenceAllowedPerPixel)
    if not result:
        image_utils.create_diff_images(image, desiredImage, screenshotPath, imageDiffScaleFactor)
        log.errorAndAssert("Result of screenshot image comparison was false, created diff image for screenshot: {})".format(screenshotPath))


def create_result_dir(basePath, resultsDir):
    resultDir = os.path.normcase(basePath+"/"+resultsDir)
    if os.path.exists(resultDir):
        #remove old directory
        shutil.rmtree(os.path.normcase(resultDir))
    #create new directory
    os.makedirs(resultDir)
    return resultDir


def get_result_dir_subdirectory(baseResultsDir, subDirName):
    resultDir = os.path.normcase(baseResultsDir+"/"+subDirName)
    if not os.path.exists(resultDir):
        os.makedirs(resultDir)
    return resultDir


def save_text_file(filePath, content):
        fileDirectory = os.path.dirname(filePath)
        if not os.path.exists(fileDirectory):
            os.makedirs(fileDirectory)
        file = codecs.open(filePath, 'w', encoding="utf-8", errors='ignore')
        file.write(content)
        file.close()


def get_env_var_setting_string(env):
    result=""
    for vname, vvalue in env.iteritems():
        result += vname + "=" + vvalue +" "
    return result
