//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Utils/Bitmap.h"
#include "Utils/BinaryFileInputStream.h"
#include "ramses-capu/os/StringUtils.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include <array>

using namespace ramses_internal;

template< UInt32 size >
::testing::AssertionResult AssertArraysEqual( const char* m_expr, const char* n_expr, const char (&m)[size], const char (&n)[size])
{
    char mHexValue[5];
    char nHexValue[5];

    for(UInt32 i = 0; i < size; ++i)
    {
        if(m[i] != n[i])
        {
            ramses_capu::StringUtils::Sprintf(mHexValue, 5, "0x%02x", m[i]);
            ramses_capu::StringUtils::Sprintf(nHexValue, 5, "0x%02x", n[i]);
            return ::testing::AssertionFailure() << m_expr << " and " << n_expr
                << " differ at position " << i << ": "
                << mHexValue << " != " << nHexValue << "\n";
        }
    }

    return ::testing::AssertionSuccess();
}

TEST(ABitmap, canGetResolution)
{
    UInt8 data[2 * 3 * 4] = { 3 };  // width*height*channels

    Bitmap bitmap(2u, 3u, data);
    EXPECT_EQ(2u, bitmap.getWidth());
    EXPECT_EQ(3u, bitmap.getHeight());
    EXPECT_EQ(6u, bitmap.getNumberOfPixels());
}

TEST(ABitmap, CanBeWrittenToProperBMPFile)
{
    const String filename("bitmapSaveTest.BMP");
    const char bitmapFileContent[] = "\x42\x4d\x86\x00\x00\x00\x00\x00\x00\x00\x36\x00\x00\x00\x28\x00\x00\x00\x05\x00\x00\x00\x05\x00\x00\x00\x01\x00\x18\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x01\x00\x03\x02\x01\x04\x03\x02\x05\x04\x03\x06\x05\x04\x00\x07\x06\x05\x08\x07\x06\x09\x08\x07\x0a\x09\x08\x0b\x0a\x09\x00\x0c\x0b\x0a\x0d\x0c\x0b\x0e\x0d\x0c\x0f\x0e\x0d\x10\x0f\x0e\x00\x11\x10\x0f\x12\x11\x10\x13\x12\x11\x14\x13\x12\x15\x14\x13\x00\x16\x15\x14\x17\x16\x15\x18\x17\x16\x19\x18\x17\x1a\x19\x18";

    UInt8 dataOut[100];
    for(UInt8 i = 0; i < 25; ++i)
    {
        dataOut[4*i + 0] = i;
        dataOut[4*i + 1] = i+1;
        dataOut[4*i + 2] = i+2;
        dataOut[4*i + 3] = i+3;
    }

    Bitmap bitmap(5, 5, dataOut);
    bitmap.saveToFileBMP(filename);

    Char dataIn[134];
    File file(filename);
    BinaryFileInputStream fileInput(file);
    fileInput.read(dataIn, 134);
    file.close();

    EXPECT_PRED_FORMAT2( AssertArraysEqual, dataIn, bitmapFileContent );
}

TEST(ABitmap, canBeLoadedFromAFile)
{
    const String filename("res/bitmapLoadTest.BMP");

    UInt8 data[100];
    for(UInt8 i = 0; i < 25; ++i)
    {
        data[4*i + 0] = i;
        data[4*i + 1] = i+1;
        data[4*i + 2] = i+2;
        data[4*i + 3] = i+3;
    }
    Bitmap bitmap(5, 5, data);

    Bitmap bitmapFromFile;
    bitmapFromFile.loadFromFileBMP(filename);

    EXPECT_EQ(bitmap, bitmapFromFile);
}

TEST(ABitmap, IsTheSameAfterWritingAndReadingFromBMPFile)
{
    UInt8 data[400];
    for(UInt8 i = 0; i < 100; ++i)
    {
        data[4*i + 0] = i;
        data[4*i + 1] = i+1;
        data[4*i + 2] = i+2;
        data[4*i + 3] = i+3;
    }

    Bitmap bitmap(10, 10, data);
    EXPECT_EQ(0, PlatformMemory::Compare(data, bitmap.getData().data(), sizeof(data)));

    bitmap.saveToFileBMP("bitmapTest.BMP");
    Bitmap bitmapFromFile;
    bitmapFromFile.loadFromFileBMP("bitmapTest.BMP");

    EXPECT_EQ(bitmap, bitmapFromFile);
}

TEST(ABitmap, IsTheSameReadBMPWritePNG)
{
    const String filenameBMP("res/bitmapLoadTest.BMP");
    const String filenamePNG("bitmapLoadTest.PNG");

    Bitmap bitmapFromBMP;
    bitmapFromBMP.loadFromFileBMP(filenameBMP);
    bitmapFromBMP.saveToFilePNG(filenamePNG);

    Bitmap bitmapFromPNG;
    bitmapFromPNG.loadFromFilePNG(filenamePNG);

    EXPECT_EQ(bitmapFromBMP, bitmapFromPNG);
}

TEST(ABitmap, ReportsTheSumOfAllItsPixels)
{
    UInt8 data[400];

    for (UInt8 i = 0; i < 100; ++i)
    {
        data[4 * i + 0] = 1;
        data[4 * i + 1] = 2;
        data[4 * i + 2] = 3;
        data[4 * i + 3] = 4;
    }

    Bitmap bitmap(10, 10, data);

    // Bitmaps don't have alpha value...
    UInt64 expectedSumOfPixelData = (1u + 2u + 3u /*+ 4u*/) * 100u;
    EXPECT_EQ(expectedSumOfPixelData, bitmap.getSumOfPixelValues());
}

TEST(ABitmap, ReturnsEmptyDiffWhenSizesDontMatchAndNotExplicitlyEnabled)
{
    constexpr size_t dataSize = 5 * 5 * 4;
    UInt8 data[dataSize] = { 0 };

    const Bitmap bitmap1(3, 3, data);
    const Bitmap bitmap2(5, 3, data);
    const Bitmap bitmap3(3, 5, data);
    const Bitmap bitmap4(5, 5, data);

    const Bitmap bitmapDiff1 = bitmap1.createDiffTo(bitmap2);
    const Bitmap bitmapDiff2 = bitmap1.createDiffTo(bitmap3);
    const Bitmap bitmapDiff3 = bitmap1.createDiffTo(bitmap4);

    EXPECT_EQ(0u, bitmapDiff1.getWidth());
    EXPECT_EQ(0u, bitmapDiff1.getHeight());
    EXPECT_EQ(0u, bitmapDiff2.getWidth());
    EXPECT_EQ(0u, bitmapDiff2.getHeight());
    EXPECT_EQ(0u, bitmapDiff3.getWidth());
    EXPECT_EQ(0u, bitmapDiff3.getHeight());
}

TEST(ABitmap, CanBeSubtractedFromOtherBitmapWithSameSize)
{
    constexpr size_t Width = 5;
    constexpr size_t Height = 3;
    UInt8 data1[Width * Height * 4];
    UInt8 data2[Width * Height * 4];

    // Subtraction keeps the larger alpha value
    const UInt8 smallerAlpha = 13u;
    const UInt8 largerAlpha = 14u;

    for (UInt8 i = 0; i < Width * Height; ++i)
    {
        // create mosaic difference
        const UInt8 value1 = (i % 2) ? 1 : 3;
        const UInt8 value2 = (i % 2) ? 3 : 1;

        data1[4 * i + 0] = value1;
        data1[4 * i + 1] = value1;
        data1[4 * i + 2] = value1;
        data1[4 * i + 3] = smallerAlpha;

        data2[4 * i + 0] = value2;
        data2[4 * i + 1] = value2;
        data2[4 * i + 2] = value2;
        data2[4 * i + 3] = largerAlpha;
    }

    const Bitmap bitmap1(Width, Height, data1);
    const Bitmap bitmap2(Width, Height, data2);
    const Bitmap bitmapDiff1 = bitmap1.createDiffTo(bitmap2, false);
    const Bitmap bitmapDiff2 = bitmap2.createDiffTo(bitmap1, false);

    // W*H pixels, times three components (r, g, b), times a difference of 2 (because of shifted mosaic pattern)
    const UInt64 expectedDiff = 2u * 3u * Width * Height;

    EXPECT_EQ(expectedDiff, bitmapDiff1.getSumOfPixelValues());
    EXPECT_EQ(expectedDiff, bitmapDiff2.getSumOfPixelValues());
    EXPECT_EQ(largerAlpha, bitmapDiff1.getData()[3]);
    EXPECT_EQ(largerAlpha, bitmapDiff2.getData()[3]);
}

TEST(ABitmap, CanBeSubtractedFromOtherBitmapWithDifferentSize)
{
    static const UInt32 Width1 = 3;
    static const UInt32 Height1 = 2;
    const UInt8 data1[Width1 * Height1 * 4] =
    {
        1, 1, 1, 10,        11, 11, 11, 10,     2, 2, 2, 20,
        1, 1, 1, 10,        11, 11, 11, 10,     2, 2, 2, 20,
    };

    static const UInt32 Width2 = 2;
    static const UInt32 Height2 = 4;
    const UInt8 data2[Width2 * Height2 * 4] =
    {
        11, 11, 11, 11,     1, 1, 1, 5,
        11, 11, 11, 11,     1, 1, 1, 5,
        33, 33, 33, 33,     2, 2, 2, 5,
        33, 33, 33, 33,     2, 2, 2, 5,
    };

    static const UInt32 ExpectWidth = 3;
    static const UInt32 ExpectHeight = 4;
    const UInt8 expectData[ExpectWidth * ExpectHeight * 4] =
    {
        10, 10, 10, 10,     10, 10, 10, 10,     2, 2, 2, 20,
        10, 10, 10, 10,     10, 10, 10, 10,     2, 2, 2, 20,
        33, 33, 33, 255,    2, 2, 2, 255,       0, 0, 0, 255,
        33, 33, 33, 255,    2, 2, 2, 255,       0, 0, 0, 255,
    };

    const Bitmap bitmap1(Width1, Height1, data1);
    const Bitmap bitmap2(Width2, Height2, data2);
    const Bitmap bitmapDiff1 = bitmap1.createDiffTo(bitmap2, true);
    const Bitmap bitmapDiff2 = bitmap2.createDiffTo(bitmap1, true);

    EXPECT_EQ(Bitmap(ExpectWidth, ExpectHeight, expectData), bitmapDiff1);
    EXPECT_EQ(Bitmap(ExpectWidth, ExpectHeight, expectData), bitmapDiff2);

}

TEST(ABitmap, YieldsBlackImageWhenSubtractedFromItself)
{
    constexpr size_t Width = 3;
    constexpr size_t Height = 5;
    UInt8 data[Width * Height * 4];

    // Alpha does not influence the pixel values, but subtraction keeps the alpha value of the first operand
    const UInt8 alpha = 13u;

    for (UInt8 i = 0; i < Width*Height; ++i)
    {
        data[4 * i + 0] = 12u;
        data[4 * i + 1] = 14u;
        data[4 * i + 2] = 11u;
        data[4 * i + 3] = alpha;
    }

    const Bitmap bitmap(Width, Height, data);
    const Bitmap bitmapDiff = bitmap.createDiffTo(bitmap, false);

    EXPECT_EQ(0u, bitmapDiff.getSumOfPixelValues());
    EXPECT_EQ(alpha, bitmapDiff.getData()[3]);
}

TEST(ABitmap, CopyConstructedBitmapIsSame)
{
    UInt8 data[400] = { 0 };

    for (UInt8 i = 0; i < 100; ++i)
    {
        data[4 * i + 0] = 12u;
        data[4 * i + 1] = 14u;
        data[4 * i + 2] = 11u;
    }

    Bitmap bitmap(10, 10, data);
    Bitmap copied(bitmap);

    EXPECT_EQ(bitmap, copied);
    EXPECT_EQ(bitmap.getData(), copied.getData());
    EXPECT_EQ(0, PlatformMemory::Compare(data, copied.getData().data(), sizeof(data)));
}

TEST(ABitmap, UnequalSizeBitmapsAreDifferent)
{
    UInt8 data[16 * 4] = { 0 };

    Bitmap bitmap(4, 4, data);
    Bitmap otherHeight(4, 3, data);
    Bitmap otherWidth(2, 4, data);

    EXPECT_NE(bitmap, otherHeight);
    EXPECT_NE(bitmap, otherWidth);
}

TEST(ABitmap, LoadPaddedBitmap)
{
    // padding_test.bmp has padded image data due its width of 3 pixels
    Bitmap original;
    original.loadFromFileBMP("res/padding_test.bmp");

    // check for correct image data loading, bitmap only contains 15 full color channels, so its sum must be 15 * 255 = 3825
    EXPECT_EQ(3825u, original.getSumOfPixelValues());
}

TEST(ABitmap, IsEqualToFlippedVersionWhenEmpty)
{
    const Bitmap emptyBitmap(0, 0, nullptr, false);
    const Bitmap flippedEmptyBitmap(0, 0, nullptr, true);
    EXPECT_EQ(emptyBitmap, flippedEmptyBitmap);
    EXPECT_EQ(emptyBitmap, flippedEmptyBitmap);
    EXPECT_EQ(emptyBitmap.getSumOfPixelValues(), flippedEmptyBitmap.getSumOfPixelValues());
}

TEST(ABitmap, IsEqualToFlippedVersionWhenSingleRow)
{
    const UInt8 data[2u * 4u] = { 0, 1, 2, 3, 10, 11, 12, 13 };
    const Bitmap oneRowBitmap(2, 1, data, false);
    const Bitmap flippedOneRowBitmap(2, 1, data, true);
    EXPECT_EQ(oneRowBitmap, flippedOneRowBitmap);
    EXPECT_EQ(oneRowBitmap.getSumOfPixelValues(), flippedOneRowBitmap.getSumOfPixelValues());
}

TEST(ABitmap, IsNotEqualToFlippedVersionWhenMultipleRows)
{
    const UInt8 data[4u * 4u] = { 0, 1, 2, 3, 10, 11, 12, 13, 20, 21, 22, 23, 30, 31, 32, 33 };
    const Bitmap multiRowsBitmap(2, 2, data, false);
    const Bitmap flippedMultiRowsBitmap(2, 2, data, true);
    EXPECT_NE(multiRowsBitmap, flippedMultiRowsBitmap);
    EXPECT_EQ(multiRowsBitmap.getSumOfPixelValues(), flippedMultiRowsBitmap.getSumOfPixelValues());
}

TEST(ABitmap, getsNumOfNonBlackPixels)
{
    const Bitmap bitmap(2, 2, std::array<UInt8, 2*2*4>{ { 0, 1, 0, 0,  0, 2, 1, 0,  5, 2, 1, 0,  13, 20, 1, 0 }}.data());
    EXPECT_EQ(3u, bitmap.getNumberOfNonBlackPixels());
    EXPECT_EQ(2u, bitmap.getNumberOfNonBlackPixels(2));
    EXPECT_EQ(1u, bitmap.getNumberOfNonBlackPixels(5));
    EXPECT_EQ(0u, bitmap.getNumberOfNonBlackPixels(20));
}
