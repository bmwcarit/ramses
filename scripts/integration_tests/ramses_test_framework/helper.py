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


def download_tar_from_ci_cache(hash, buildJobName, ciCacheUrl, destFolder):
    chunksize = 32 * 1024

    filter_re = re.compile("{}-(.*)-(.*)-{}\.tar\.gz".format(buildJobName, hash), re.IGNORECASE)
    for f in os.listdir(destFolder):
        match = re.match(filter_re, f)
        if match:
            log.info("File already exist in destFolder, using existing file {}".format(f))
            return True, match.group(1), match.group(2)

    try:
        page_url = "{}/{}-{}/".format(ciCacheUrl, buildJobName, hash)
        page_response = urlopen(page_url)
        content = page_response.read()
    except HTTPError:
        log.error("Could not open url {}".format(page_url))
        return False, "", ""

    match = re.search(filter_re, content)
    if match is None:
        log.error("Could not find tar on page")
        print(content)
        return False, "", ""

    tarfilename = match.group(0)
    version = match.group(1)
    commitNr = match.group(2)
    log.info("downloading: {}".format(tarfilename))

    file_response = urlopen("{}/{}-{}/{}".format(ciCacheUrl, buildJobName, hash, tarfilename))
    meta = file_response.info()
    content_length = int(meta.getheaders("Content-Length")[0])
    progress_messages_step_size = content_length / 10
    bytes = 0
    bytes_since_last_message = 0
    with open(os.path.join(destFolder, tarfilename), "wb") as f:
        while True:
            chunk = file_response.read(chunksize)
            if not chunk:
                break
            f.write(chunk)
            bytes += len(chunk)
            bytes_since_last_message += len(chunk)
            if bytes_since_last_message >= progress_messages_step_size or bytes == content_length:
                log.info("downloaded {} of {} bytes".format(bytes, content_length))
                bytes_since_last_message = 0
    if bytes == content_length:
        return True, version, commitNr
    else:
        log.error("Download incomplete")
        return False, "", ""
