//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "CreationHelper.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/StreamTexture.h"

using namespace testing;

namespace ramses
{
    class StreamTextureTest : public LocalTestClientWithScene, public ::testing::Test
    {
    public:
        StreamTextureTest()
        {
        }
    };

    TEST_F(StreamTextureTest, createStreamTexture)
    {
        const Texture2D& fallbackTexture = createObject<Texture2D>("fallbackTexture");
        StreamTexture* streamTexture = this->m_scene.createStreamTexture(fallbackTexture, streamSource_t(0), "testStreamTexture");

        ASSERT_NE(static_cast<StreamTexture*>(0), streamTexture);
    }

    TEST_F(StreamTextureTest, reportsErrorWhenValidatedWithInvalidFallbackTexture)
    {
        const Texture2D& fallbackTexture = createObject<Texture2D>("fallbackTexture");
        StreamTexture* streamTexture = this->m_scene.createStreamTexture(fallbackTexture, streamSource_t(0), "testStreamTexture");
        ASSERT_TRUE(NULL != streamTexture);
        EXPECT_EQ(StatusOK, streamTexture->validate());

        EXPECT_EQ(StatusOK, client.destroy(fallbackTexture));
        EXPECT_NE(StatusOK, streamTexture->validate());
    }

    TEST_F(StreamTextureTest, canToggleForceFallbackImage)
    {
        const Texture2D& fallbackTexture = createObject<Texture2D>("fallbackTexture");
        StreamTexture* streamTexture = this->m_scene.createStreamTexture(fallbackTexture, streamSource_t(0), "testStreamTexture");

        ASSERT_NE(static_cast<StreamTexture*>(0), streamTexture);

        EXPECT_FALSE(streamTexture->getForceFallbackImage());

        EXPECT_EQ(StatusOK, streamTexture->forceFallbackImage(true));
        EXPECT_TRUE(streamTexture->getForceFallbackImage());

        EXPECT_EQ(StatusOK, streamTexture->forceFallbackImage(false));
        EXPECT_FALSE(streamTexture->getForceFallbackImage());
    }

    TEST_F(StreamTextureTest, canGetStreamSourceId)
    {
        const Texture2D& fallbackTexture = createObject<Texture2D>("fallbackTexture");
        StreamTexture* streamTexture = this->m_scene.createStreamTexture(fallbackTexture, streamSource_t(123u), "testStreamTexture");

        EXPECT_EQ(streamSource_t(123u), streamTexture->getStreamSourceId());
    }
}
