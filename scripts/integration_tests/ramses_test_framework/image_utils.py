#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from PIL import ImageChops
from PIL import ImageEnhance
from PIL import ImageMath
import os.path
from ramses_test_framework import log

def _checkImageSizeEqual(image1, image2):
    if (image1.size[0] != image2.size[0]) or (image1.size[1] != image2.size[1]):
        log.error("images sizes do not match, cannot compare ({}*{} vs{}*{}".format(image1.size[0], image1.size[1],image2.size[0], image2.size[1]))
        return False
    return True

def _forAllPixels(image1, image2, fun):
    image1data = image1.getdata()  #getPixel for each pixel is 3 times slower
    image2 = image2.convert("RGB")  # on the fly convert to RGB image, needed to guarantee 3-value tuple access
    image2data = image2.getdata()

    for y in range(0, image1.size[1]):  #height
        for x in range(0, image1.size[0]): #width
            index = y*image1.size[0] + x
            image1Channels = image1data[index]
            image2Channels = image2data[index]
            diff = tuple(abs(e[0]-e[1])/255. for e in zip(image1Channels, image2Channels))
            fun(x, y, image1Channels, image2Channels, diff)


def compareEqual(image1, image2, percentageOfWrongPixelsAllowed, percentageOfRGBDifferenceAllowedPerPixel):
    log.info("compareEqual: Bitmap compare of {0} with {1}".format(image1.filename, image2.filename))
    log.info("Allowing {}% tolerance on RGB value per pixel, and {}% wrong pixels".format(percentageOfRGBDifferenceAllowedPerPixel*100, percentageOfWrongPixelsAllowed*100))

    if not _checkImageSizeEqual(image1, image2):
        return False

    # work around nested function variable access rules
    nrWrongPixels = [0]
    nrDifferentPixels = [0]

    def compare(x, y, c1, c2, diff):
        if any(e > 0 for e in diff):
            nrDifferentPixels[0] += 1
            if any(e > percentageOfRGBDifferenceAllowedPerPixel for e in diff):
                if nrWrongPixels[0] == 0:
                    log.info("First wrong Pixel {}/{} (source pixel {} / otherPixel {})".format(x, y, c1, c2))
                nrWrongPixels[0] += 1

    _forAllPixels(image1, image2, compare)

    totalNumberOfPixels = image1.size[0] * image1.size[1]
    log.important_info("Comparison stats: Percentage of wrong pixels: {0}%".format(float(nrWrongPixels[0]) / totalNumberOfPixels*100))
    log.important_info("Comparison stats: Percentage of different, but accepted pixels: {0}%".format(float(nrDifferentPixels[0]-nrWrongPixels[0]) / totalNumberOfPixels*100))

    if ( (float(nrWrongPixels[0])/ totalNumberOfPixels) > percentageOfWrongPixelsAllowed):
        log.error("compareEqual: Too many wrong pixels, aborting...")
        return False
    return True


def compareUnequal(image1, image2, numberOfRequiredUnequalPixels, percentageOfRGBDifferenceRequiredPerPixel):
    log.info("compareUnequal: Bitmap compare of {0} with {1}".format(image1.filename, image2.filename))
    log.info("Requiring {}% difference on RGB value per pixel, and require {} really distinct pixels".format(percentageOfRGBDifferenceRequiredPerPixel*100, numberOfRequiredUnequalPixels))

    if not _checkImageSizeEqual(image1, image2):
        return False

    # work around crappy nested function variable access rules
    nrEqualPixels = [0]
    nrTooSimilarPixels = [0]

    def compare(x, y, c1, c2, diff):
        if all(e < percentageOfRGBDifferenceRequiredPerPixel for e in diff):
            if nrTooSimilarPixels[0] == 0:
                log.info("First wrong Pixel {}/{} (source pixel {} / otherPixel {})".format(x, y, c1, c2))
            nrTooSimilarPixels[0] += 1
            if all(e == 0 for e in diff):
                nrEqualPixels[0] += 1

    _forAllPixels(image1, image2, compare)

    totalNumberOfPixels = image1.size[0] * image1.size[1]
    log.important_info("Comparison stats: Percentage of too similar pixels: {}% ({})".format(float(nrTooSimilarPixels[0]) / totalNumberOfPixels*100, nrTooSimilarPixels[0]))
    log.important_info("Comparison stats: Percentage of exactly equal pixels: {}% ({})".format(float(nrEqualPixels[0]) / totalNumberOfPixels*100, nrEqualPixels[0]))

    if totalNumberOfPixels - nrTooSimilarPixels[0] < numberOfRequiredUnequalPixels:
        log.error("compareUnequal: Not enough unequal pixels, aborting...")
        return False
    return True


def create_diff_images(image1, image2, originalFilePath, scaleFactor):
    diff = ImageChops.difference(image1, image2)
    diffScaled = ImageEnhance.Contrast(diff).enhance(scaleFactor)
    (root, ext) = os.path.splitext(originalFilePath)
    diff.save(root+"_DIFF"+ext)
    diffScaled.save(root+"_DIFF_SCALED"+ext)
    log.info("diff files saved")
