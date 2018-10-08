//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/TextureData.h"

using namespace ramses_internal;

TEST(TextureDataTest, Constructor)
{
    UInt8* d = new UInt8[20 * 30 * 3 * 2];
    TextureData data(ETextureFormat_RG8, 20, 30, 3, d, true);

    EXPECT_EQ(ETextureFormat_RG8, data.getFormat());
    EXPECT_EQ(20u, data.getWidth());
    EXPECT_EQ(30u, data.getHeight());
    EXPECT_EQ(3u, data.getDepth());
    EXPECT_EQ(d, data.getData());
    EXPECT_TRUE(data.ownsData());
}

TEST(TextureDataTest, ConstructorNoOwner)
{
    UInt8* d = new UInt8[20 * 30 * 3 * 2];
    TextureData data(ETextureFormat_RG8, 20, 30, 3, d, false);

    EXPECT_FALSE(data.ownsData());

    delete[] d;
}

TEST(TextureDataTest, set)
{
    UInt8* d = new UInt8[20 * 30 * 3 * 2];
    TextureData data(ETextureFormat_RG8, 20, 30, 3, d, true);

    d = new UInt8[40 * 50 * 3];
    data.set(ETextureFormat_RGB8, 40, 50, 1, d, true);

    EXPECT_EQ(ETextureFormat_RGB8, data.getFormat());
    EXPECT_EQ(40u, data.getWidth());
    EXPECT_EQ(50u, data.getHeight());
    EXPECT_EQ(1u, data.getDepth());
    EXPECT_EQ(d, data.getData());
}

TEST(TextureDataTest, setNoOwner)
{
    UInt8* d = new UInt8[20 * 30 * 3 * 2];
    TextureData data(ETextureFormat_RG8, 20, 30, 3, d, false);

    UInt8* d2 = new UInt8[40 * 50 * 3];
    data.set(ETextureFormat_RGB8, 40, 50, 1, d2, false);

    EXPECT_EQ(ETextureFormat_RGB8, data.getFormat());
    EXPECT_EQ(40u, data.getWidth());
    EXPECT_EQ(50u, data.getHeight());
    EXPECT_EQ(1u, data.getDepth());
    EXPECT_EQ(d2, data.getData());

    delete[] d;
    delete[] d2;
}

TEST(TextureDataTest, convertR8ToRGB)
{
    const UInt32 n = 20 * 30 * 3;
    UInt8* d = new UInt8[n];
    for (UInt32 i = 0; i < n; i++)
    {
        d[i] = static_cast<UInt8>(i);
    }

    TextureData data(ETextureFormat_R8, 20, 30, 3, d, false);

    EXPECT_TRUE(data.convert(ETextureFormat_RGB8));

    const UInt8* d2= data.getData();

    for (UInt32 i = 0; i < n; i++)
    {
        EXPECT_EQ(d[i], d2[i * 3 + 0]);
        EXPECT_EQ(0, d2[i * 3 + 1]);
        EXPECT_EQ(0, d2[i * 3 + 2]);
    }

    delete[] d;
}

TEST(TextureDataTest, convertRG8ToRGB)
{
    const UInt32 n = 20 * 30 * 3;
    UInt8* d = new UInt8[n * 2];
    for (UInt32 i = 0; i < n; i++)
    {
        d[i * 2 + 0] = static_cast<UInt8>(i);
        d[i * 2 + 1] = static_cast<UInt8>(i + 1);
    }

    TextureData data(ETextureFormat_RG8, 20, 30, 3, d, false);

    EXPECT_TRUE(data.convert(ETextureFormat_RGB8));

    const UInt8* d2= data.getData();

    for (UInt32 i = 0; i < n; i++)
    {
        EXPECT_EQ(d[i * 2 + 0], d2[i * 3 + 0]);
        EXPECT_EQ(d[i * 2 + 1], d2[i * 3 + 1]);
        EXPECT_EQ(0, d2[i * 3 + 2]);
    }

    delete[] d;
}

TEST(TextureDataTest, canSetDataToItself_noOwnership)
{
    UInt8 d[2] = { 1u, 3u };
    TextureData data(ETextureFormat_R8, 1, 2, 1, d, false);

    data.set(data.getFormat(), data.getWidth(), data.getHeight(), data.getDepth(), data.getData(), data.ownsData());

    EXPECT_EQ(ETextureFormat_R8, data.getFormat());
    EXPECT_EQ(1u, data.getWidth());
    EXPECT_EQ(2u, data.getHeight());
    EXPECT_EQ(1u, data.getDepth());
    EXPECT_EQ(d, data.getData());
    EXPECT_FALSE(data.ownsData());
}

TEST(TextureDataTest, canSetDataToItself_hasOwnership)
{
    UInt8* d = new UInt8[2];
    d[0] = 1u;
    d[1] = 3u;
    TextureData data(ETextureFormat_R8, 1, 2, 1, d, true);

    data.set(data.getFormat(), data.getWidth(), data.getHeight(), data.getDepth(), data.getData(), data.ownsData());

    EXPECT_EQ(ETextureFormat_R8, data.getFormat());
    EXPECT_EQ(1u, data.getWidth());
    EXPECT_EQ(2u, data.getHeight());
    EXPECT_EQ(1u, data.getDepth());
    EXPECT_EQ(d, data.getData());
    EXPECT_TRUE(data.ownsData());
}
