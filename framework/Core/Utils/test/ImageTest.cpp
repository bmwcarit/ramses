//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Utils/Image.h"
#include "Utils/BinaryFileInputStream.h"
#include "ramses-capu/os/StringUtils.h"
#include <array>
#include <numeric>

using namespace ramses_internal;

TEST(AnImage, canGetResolution)
{
    UInt8 data[2 * 3 * 4] = { 3 };  // width*height*channels

    Image bitmap(2u, 3u, data, data + sizeof(data));
    EXPECT_EQ(2u, bitmap.getWidth());
    EXPECT_EQ(3u, bitmap.getHeight());
    EXPECT_EQ(6u, bitmap.getNumberOfPixels());
}

TEST(AnImage, canSaveAndLoadPNG)
{
    const uint32_t w = 10u;
    const uint32_t h = 5u;
    std::vector<UInt8> data(w * h * 4);
    std::iota(data.begin(), data.end(), UInt8(0u));
    const Image bitmap(w, h, std::move(data));
    bitmap.saveToFilePNG("bitmapTest.png");
    Image loadedBitmap;
    loadedBitmap.loadFromFilePNG("bitmapTest.png");
    EXPECT_EQ(bitmap, loadedBitmap);
    EXPECT_EQ(bitmap.getWidth(), loadedBitmap.getWidth());
    EXPECT_EQ(bitmap.getHeight(), loadedBitmap.getHeight());
    EXPECT_EQ(bitmap.getData(), loadedBitmap.getData());
}

TEST(AnImage, ReportsTheSumOfAllItsPixels)
{
    UInt8 data[400];

    for (UInt8 i = 0; i < 100; ++i)
    {
        data[4 * i + 0] = 1;
        data[4 * i + 1] = 2;
        data[4 * i + 2] = 3;
        data[4 * i + 3] = 4;
    }

    Image bitmap(10, 10, data, data + sizeof(data));

    const UInt64 expectedSumOfPixelData = (1u + 2u + 3u + 4u) * 100u;
    EXPECT_EQ(expectedSumOfPixelData, bitmap.getSumOfPixelValues());
}

TEST(AnImage, CanBeSubtractedFromOtherBitmapWithSameSize)
{
    constexpr size_t Width = 5;
    constexpr size_t Height = 3;
    UInt8 data1[Width * Height * 4];
    UInt8 data2[Width * Height * 4];

    for (UInt8 i = 0; i < Width * Height; ++i)
    {
        // create mosaic difference
        const UInt8 value1 = (i % 2) ? 1 : 3;
        const UInt8 value2 = (i % 2) ? 3 : 1;

        data1[4 * i + 0] = value1;
        data1[4 * i + 1] = value1;
        data1[4 * i + 2] = value1;
        data1[4 * i + 3] = value1;

        data2[4 * i + 0] = value2;
        data2[4 * i + 1] = value2;
        data2[4 * i + 2] = value2;
        data2[4 * i + 3] = value2;
    }

    const Image bitmap1(Width, Height, data1, data1 + sizeof(data1));
    const Image bitmap2(Width, Height, data2, data2 + sizeof(data2));
    const Image bitmapDiff1 = bitmap1.createDiffTo(bitmap2);
    const Image bitmapDiff2 = bitmap2.createDiffTo(bitmap1);

    // W*H pixels, times 4 components (r, g, b, a), times a difference of 2 (because of shifted mosaic pattern)
    const UInt64 expectedDiff = 2u * 4u * Width * Height;

    EXPECT_EQ(expectedDiff, bitmapDiff1.getSumOfPixelValues());
    EXPECT_EQ(expectedDiff, bitmapDiff2.getSumOfPixelValues());
    EXPECT_EQ(2u, bitmapDiff2.getData()[3]);
    EXPECT_EQ(2u, bitmapDiff1.getData()[3]);
}

TEST(AnImage, YieldsBlackImageWhenSubtractedFromItself)
{
    constexpr size_t Width = 3;
    constexpr size_t Height = 5;
    UInt8 data[Width * Height * 4];

    for (UInt8 i = 0; i < Width*Height; ++i)
    {
        data[4 * i + 0] = 12u;
        data[4 * i + 1] = 14u;
        data[4 * i + 2] = 11u;
        data[4 * i + 3] = 13u;
    }

    const Image bitmap(Width, Height, data, data + sizeof(data));
    const Image bitmapDiff = bitmap.createDiffTo(bitmap);

    EXPECT_EQ(0u, bitmapDiff.getSumOfPixelValues());
}

TEST(AnImage, GivesEmptyImageIfEnlargingToSmallerSize)
{
    const Image image(2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
    EXPECT_EQ(Image(), image.createEnlarged(1, 3));
    EXPECT_EQ(Image(), image.createEnlarged(3, 1));
    EXPECT_EQ(Image(), image.createEnlarged(1, 1));
}

TEST(AnImage, GivesImageCopyIfEnlargingToSameSize)
{
    const Image image(2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
    EXPECT_EQ(image, image.createEnlarged(2, 2));
}

TEST(AnImage, CreatesEnlargedImageContainingOriginalAndRestFilledWithGivenValue_extendedWidth)
{
    const Image image(2, 2,
    {
        1,  2,  3,  4,    5,  6,  7,  8,
        9, 10, 11, 12,   13, 14, 15, 16
    });
    const Image expectedImage(3, 2,
    {
        1,  2,  3,  4,    5,  6,  7,  8,   33, 44, 55, 66,
        9, 10, 11, 12,   13, 14, 15, 16,   33, 44, 55, 66
    });
    EXPECT_EQ(expectedImage, image.createEnlarged(3, 2, {33, 44, 55, 66}));
}

TEST(AnImage, CreatesEnlargedImageContainingOriginalAndRestFilledWithGivenValue_extendedHeight)
{
    const Image image(2, 2,
    {
        1,  2,  3,  4,    5,  6,  7,  8,
        9, 10, 11, 12,   13, 14, 15, 16
    });
    const Image expectedImage(2, 3,
    {
        1,  2,  3,  4,    5,  6,  7,  8,
        9, 10, 11, 12,   13, 14, 15, 16,
       33, 44, 55, 66,   33, 44, 55, 66
    });
    EXPECT_EQ(expectedImage, image.createEnlarged(2, 3, { 33, 44, 55, 66 }));
}

TEST(AnImage, CreatesEnlargedImageContainingOriginalAndRestFilledWithGivenValue_extendedWidthAndHeight)
{
    const Image image(2, 2,
    {
        1,  2,  3,  4,    5,  6,  7,  8,
        9, 10, 11, 12,   13, 14, 15, 16
    });
    const Image expectedImage(3, 3,
    {
        1,  2,  3,  4,     5,  6,  7,  8,   33, 44, 55, 66,
        9, 10, 11, 12,    13, 14, 15, 16,   33, 44, 55, 66,
       33, 44, 55, 66,    33, 44, 55, 66,   33, 44, 55, 66
    });
    EXPECT_EQ(expectedImage, image.createEnlarged(3, 3, { 33, 44, 55, 66 }));
}

TEST(AnImage, CopyConstructedBitmapIsSame)
{
    UInt8 data[400] = { 0 };

    for (UInt8 i = 0; i < 100; ++i)
    {
        data[4 * i + 0] = 12u;
        data[4 * i + 1] = 14u;
        data[4 * i + 2] = 11u;
    }

    Image bitmap(10, 10, data, data + sizeof(data));
    Image copied(bitmap);

    EXPECT_EQ(bitmap, copied);
}

TEST(AnImage, AssignedBitmapIsSame)
{
    const Image image(2, 2,
    {
        1,  2,  3,  4,    5,  6,  7,  8,
        9, 10, 11, 12,   13, 14, 15, 16
    });

    Image image2(1, 1, { 1,  2,  3,  4 });

    image2 = image;
    EXPECT_EQ(image, image2);
}

TEST(AnImage, MoveAssignedBitmapIsSame)
{
    const Image image(2, 2,
    {
        1,  2,  3,  4,    5,  6,  7,  8,
        9, 10, 11, 12,   13, 14, 15, 16
    });

    Image image2(1, 1, { 1,  2,  3,  4 });

    const Image origImage = image;

    image2 = std::move(image);
    EXPECT_EQ(origImage, image2);
}

TEST(AnImage, UnequalSizeBitmapsAreDifferent)
{
    static const UInt8 data[5 * 3 * 4] = { 0 };
    const Image bitmap(5, 3, data, data + sizeof(data));

    const Image otherHeight(5, 2, data, data + 5*2*4);
    const Image otherWidth(4, 3, data, data + 4*3*4);
    const Image otherSizeSamePixelCount(3, 5, data, data + sizeof(data));

    EXPECT_NE(bitmap, otherHeight);
    EXPECT_NE(bitmap, otherWidth);
    EXPECT_NE(bitmap, otherSizeSamePixelCount);
}

TEST(AnImage, FlipsImage2x1)
{
    const std::vector<UInt8> data
    {
        1,  2,  3,  4,   5,  6,  7,  8
    };
    const Image expectedImage(2, 1,
    {
        1,  2,  3,  4,   5,  6,  7,  8
    });

    EXPECT_EQ(expectedImage, Image(2, 1, data.cbegin(), data.cend(), true));
}

TEST(AnImage, FlipsImage2x2)
{
    const std::vector<UInt8> data
    {
        1,  2,  3,  4,   5,  6,  7,  8,
        9, 10, 11, 12,  13, 14, 15, 16
    };
    const Image expectedImage(2, 2,
    {
        9, 10, 11, 12,  13, 14, 15, 16,
        1,  2,  3,  4,   5,  6,  7,  8
    });

    EXPECT_EQ(expectedImage, Image(2, 2, data.cbegin(), data.cend(), true));
}

TEST(AnImage, FlipsImage2x3)
{
    const std::vector<UInt8> data
    {
        1,  2,  3,  4,    5,  6,  7,  8,
        9, 10, 11, 12,   13, 14, 15, 16,
        17, 18, 19, 20,  21, 22, 23, 24
    };
    const Image expectedImage(2, 3,
    {
        17, 18, 19, 20,  21, 22, 23, 24,
        9, 10, 11, 12,   13, 14, 15, 16,
        1,  2,  3,  4,    5,  6,  7,  8,
    });

    EXPECT_EQ(expectedImage, Image(2, 3, data.cbegin(), data.cend(), true));
}

TEST(AnImage, FlipsImage4x4)
{
    const std::vector<UInt8> data
    {
        1,  2,  3,  4,    5,  6,  7,  8,   9, 10, 11, 12,  13, 14, 15, 16,
        17, 18, 19, 20,  21, 22, 23, 24,  25, 26, 27, 28,  29, 30, 31, 32,
        33, 34, 35, 36,  37, 38, 39, 40,  41, 42, 43, 44,  45, 46, 47, 48,
        49, 50, 51, 52,  53, 54, 55, 56,  57, 58, 59, 60,  61, 62, 63, 64
    };
    const Image expectedImage(4, 4,
    {
        49, 50, 51, 52,  53, 54, 55, 56,  57, 58, 59, 60,  61, 62, 63, 64,
        33, 34, 35, 36,  37, 38, 39, 40,  41, 42, 43, 44,  45, 46, 47, 48,
        17, 18, 19, 20,  21, 22, 23, 24,  25, 26, 27, 28,  29, 30, 31, 32,
        1,  2,  3,  4,    5,  6,  7,  8,   9, 10, 11, 12,  13, 14, 15, 16
    });

    EXPECT_EQ(expectedImage, Image(4, 4, data.cbegin(), data.cend(), true));
}

TEST(AnImage, getsNumOfNonBlackPixels)
{
    std::array<UInt8, 2 * 2 * 4> data{ { 0, 1, 0, 0, 0, 2, 1, 0, 5, 2, 1, 0, 13, 20, 1, 0 } };
    const Image bitmap(2, 2, data.cbegin(), data.cend());
    EXPECT_EQ(3u, bitmap.getNumberOfNonBlackPixels());
    EXPECT_EQ(2u, bitmap.getNumberOfNonBlackPixels(2));
    EXPECT_EQ(1u, bitmap.getNumberOfNonBlackPixels(5));
    EXPECT_EQ(0u, bitmap.getNumberOfNonBlackPixels(20));
}
