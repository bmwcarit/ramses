#  -------------------------------------------------------------------------
#  Copyright (C) 2014 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from PIL import ImageChops
from PIL import ImageEnhance
import os.path
from ramses_test_framework import log

def _checkImageSizeEqual(image1, image2):
    if (image1.size[0] != image2.size[0]) or (image1.size[1] != image2.size[1]):
        log.error("images sizes do not match, cannot compare {}*{} vs {}*{}".format(image1.size[0], image1.size[1], image2.size[0], image2.size[1]))
        return False
    return True


def compareEqual(image1, image2, percentageOfWrongPixelsAllowed, percentageOfRGBDifferenceAllowedPerPixel):
    log.info("Allowing {}% tolerance on RGBA value per pixel, and {}% wrong pixels".format(percentageOfRGBDifferenceAllowedPerPixel*100, percentageOfWrongPixelsAllowed*100))

    if not _checkImageSizeEqual(image1, image2):
        return False

    nrWrongPixels = 0
    nrDifferentPixels = 0

    # PIL image comparison is well optimized -> early out if images are identical
    if image1 != image2:
        imageDiff = ImageChops.difference(image1.convert("RGBA"), image2.convert("RGBA"))
        imageData = imageDiff.getdata()
        percentageOfRGBDifferenceAllowedPerPixelScaled = int(percentageOfRGBDifferenceAllowedPerPixel*255)

        for i in range(0, image1.width * image1.height):
            chMax = max(imageData[i])
            if chMax > 0:
                nrDifferentPixels += 1
                if chMax > percentageOfRGBDifferenceAllowedPerPixelScaled:
                    nrWrongPixels += 1

    totalNumberOfPixels = image1.width * image1.height
    log.important_info("Comparison stats: Percentage of wrong pixels: {0}%".format(float(nrWrongPixels) / totalNumberOfPixels*100))
    log.important_info("Comparison stats: Percentage of different, but accepted pixels: {0}%".format(float(nrDifferentPixels-nrWrongPixels) / totalNumberOfPixels*100))

    if ((float(nrWrongPixels) / totalNumberOfPixels) > percentageOfWrongPixelsAllowed):
        log.error("compareEqual: Too many wrong pixels, aborting...")
        return False
    return True


def compareUnequal(image1, image2, numberOfRequiredUnequalPixels, percentageOfRGBDifferenceRequiredPerPixel):
    log.info("Requiring {}% difference on RGBA value per pixel, and require {} really distinct pixels".format(percentageOfRGBDifferenceRequiredPerPixel*100, numberOfRequiredUnequalPixels))

    if not _checkImageSizeEqual(image1, image2):
        return False

    nrEqualPixels = 0
    nrTooSimilarPixels = 0
    totalNumberOfPixels = image1.width * image1.height

    # PIL image comparison is well optimized -> early out if images are identical
    if image1 == image2:
        nrEqualPixels = totalNumberOfPixels
        nrTooSimilarPixels = totalNumberOfPixels
    else:
        imageDiff = ImageChops.difference(image1.convert("RGBA"), image2.convert("RGBA"))
        imageData = imageDiff.getdata()
        percentageOfRGBDifferenceRequiredPerPixelScaled = int(percentageOfRGBDifferenceRequiredPerPixel*255)

        for i in range(0, image1.width * image1.height):
            chMax = max(imageData[i])
            if chMax < percentageOfRGBDifferenceRequiredPerPixelScaled:
                nrTooSimilarPixels += 1
                if chMax == 0:
                    nrEqualPixels += 1

    log.important_info("Comparison stats: Percentage of too similar pixels: {}% ({})".format(float(nrTooSimilarPixels) / totalNumberOfPixels*100, nrTooSimilarPixels))
    log.important_info("Comparison stats: Percentage of exactly equal pixels: {}% ({})".format(float(nrEqualPixels) / totalNumberOfPixels*100, nrEqualPixels))

    if totalNumberOfPixels - nrTooSimilarPixels < numberOfRequiredUnequalPixels:
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
