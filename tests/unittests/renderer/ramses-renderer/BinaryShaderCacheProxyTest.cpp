//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/renderer/IBinaryShaderCache.h"
#include "impl/BinaryShaderCacheProxy.h"
#include "internal/Core/Utils/ThreadBarrier.h"

#include <gmock/gmock.h>
#include <random>
#include <thread>

using namespace testing;
namespace ramses::internal
{

    class BinaryShaderCacheMock : public ramses::IBinaryShaderCache
    {
    public:
        BinaryShaderCacheMock() = default;
        ~BinaryShaderCacheMock() override = default;

        MOCK_METHOD(void, deviceSupportsBinaryShaderFormats, (const binaryShaderFormatId_t* supportedFormats, uint32_t numSupportedFormats), (override));
        MOCK_METHOD(bool, hasBinaryShader, (effectId_t effectId), (const, override));
        MOCK_METHOD(uint32_t, getBinaryShaderSize, (effectId_t effectId), (const, override));
        MOCK_METHOD(binaryShaderFormatId_t, getBinaryShaderFormat, (effectId_t effectId), (const, override));
        MOCK_METHOD(bool, shouldBinaryShaderBeCached, (effectId_t effectId, sceneId_t sceneId), (const, override));
        MOCK_METHOD(void, getBinaryShaderData, (effectId_t effectId, std::byte* buffer, uint32_t bufferSize), (const, override));
        MOCK_METHOD(void, storeBinaryShader, (effectId_t effectId, sceneId_t sceneId, const std::byte* binaryShaderData, uint32_t binaryShaderDataSize, binaryShaderFormatId_t binaryShaderFormat), (override));
        MOCK_METHOD(void, binaryShaderUploaded, (effectId_t effectId, bool success), (const, override));
    };

    class ABinaryShaderCacheProxy : public testing::Test
    {
    public:
        ABinaryShaderCacheProxy()
            : proxy(cache)
        {

        }

        ~ABinaryShaderCacheProxy() override = default;

    protected:
        testing::StrictMock<BinaryShaderCacheMock> cache;
        BinaryShaderCacheProxy proxy;

        ResourceContentHash effectHash = ResourceContentHash{ 0x1234567890abcdef, 0xfedcba0987654321 };
        effectId_t effectExternal = { 0x1234567890abcdef, 0xfedcba0987654321 };
    };

    TEST_F(ABinaryShaderCacheProxy, forwardsDeviceSupportsBinaryShaderFormatsCallsToCacheAndTransformsVectorCorrectly)
    {
        std::vector<BinaryShaderFormatID> format{
            BinaryShaderFormatID{ 1 },
            BinaryShaderFormatID{ 2 },
            BinaryShaderFormatID{ 3 },
            BinaryShaderFormatID{ 4 }
        };

        EXPECT_CALL(cache, deviceSupportsBinaryShaderFormats(_, 4));
        proxy.deviceSupportsBinaryShaderFormats(format);

        format.clear();
        EXPECT_CALL(cache, deviceSupportsBinaryShaderFormats(_, 0));
        proxy.deviceSupportsBinaryShaderFormats(format);
    }

    TEST_F(ABinaryShaderCacheProxy, forwardsCallsToCache)
    {
        EXPECT_CALL(cache, hasBinaryShader(effectExternal)).WillOnce(Return(true));
        EXPECT_TRUE(proxy.hasBinaryShader(effectHash));

        EXPECT_CALL(cache, getBinaryShaderSize(effectExternal)).WillOnce(Return(3u));
        EXPECT_EQ(3u, proxy.getBinaryShaderSize(effectHash));

        EXPECT_CALL(cache, getBinaryShaderFormat(effectExternal)).WillOnce(Return(binaryShaderFormatId_t{ 3u }));
        EXPECT_EQ(BinaryShaderFormatID{ 3u }, proxy.getBinaryShaderFormat(effectHash));

        EXPECT_CALL(cache, shouldBinaryShaderBeCached(effectExternal, sceneId_t{ 111 })).WillOnce(Return(true));
        EXPECT_TRUE(proxy.shouldBinaryShaderBeCached(effectHash, SceneId{ 111 }));

        EXPECT_CALL(cache, getBinaryShaderData(effectExternal, nullptr, 123));
        proxy.getBinaryShaderData(effectHash, nullptr, 123);

        EXPECT_CALL(cache, storeBinaryShader(effectExternal, sceneId_t{ 111 }, nullptr, 123, binaryShaderFormatId_t{ 3u }));
        proxy.storeBinaryShader(effectHash, SceneId{ 111 }, nullptr, 123, BinaryShaderFormatID{ 3u });

        EXPECT_CALL(cache, binaryShaderUploaded(effectExternal, true));
        proxy.binaryShaderUploaded(effectHash, true);
    }

    TEST_F(ABinaryShaderCacheProxy, makesCallsToCacheInAThreadsafeWay)
    {
        ThreadBarrier startBarrier;
        auto randomlyExecuteFunctions = [this, &startBarrier]() {
            std::random_device randomSource;
            std::mt19937 gen(randomSource());
            std::uniform_int_distribution<uint32_t> dis(0, 6);

            startBarrier.wait();
            for (int i = 0; i < 10000; ++i)
            {
                switch (dis(gen))
                {
                case 0:
                    proxy.hasBinaryShader(effectHash);
                    break;
                case 1:
                    proxy.getBinaryShaderSize(effectHash);
                    break;
                case 2:
                    proxy.getBinaryShaderFormat(effectHash);
                    break;
                case 3:
                    proxy.shouldBinaryShaderBeCached(effectHash, SceneId{ 111 });
                    break;
                case 4:
                    proxy.getBinaryShaderData(effectHash, nullptr, 123);
                    break;
                case 5:
                    proxy.storeBinaryShader(effectHash, SceneId{ 111 }, nullptr, 123, BinaryShaderFormatID{ 3u });
                    break;
                case 6:
                    proxy.binaryShaderUploaded(effectHash, true);
                    break;
                default:
                    assert(0);
                }
            }
        };

        int w = 0; // all threads write this variable to trigger tsan
        EXPECT_CALL(cache, hasBinaryShader(_)).Times(AnyNumber()).WillRepeatedly([&](auto /*unused*/) { w = 8; return true; });
        EXPECT_CALL(cache, getBinaryShaderSize(_)).Times(AnyNumber()).WillRepeatedly([&](auto /*unused*/) { w = 22; return 3u; });
        EXPECT_CALL(cache, getBinaryShaderFormat(_)).Times(AnyNumber()).WillRepeatedly([&](auto /*unused*/) { w = 800; return binaryShaderFormatId_t{ 3 }; });
        EXPECT_CALL(cache, shouldBinaryShaderBeCached(_, _)).Times(AnyNumber()).WillRepeatedly([&](auto /*unused*/, auto /*unused*/) { w = 12; return true; });
        EXPECT_CALL(cache, getBinaryShaderData(_, _, _)).Times(AnyNumber()).WillRepeatedly([&](auto /*unused*/, auto /*unused*/, auto /*unused*/) { w = 120; });
        EXPECT_CALL(cache, storeBinaryShader(_, _, _, _, _)).Times(AnyNumber()).WillRepeatedly([&](auto /*unused*/, auto /*unused*/, auto /*unused*/, auto /*unused*/, auto /*unused*/) { w = 60; });
        EXPECT_CALL(cache, binaryShaderUploaded(_, _)).Times(AnyNumber()).WillRepeatedly([&](auto /*unused*/, auto /*unused*/) { w = 99; });

        startBarrier.init_wait_for_num(3);
        std::thread t1(randomlyExecuteFunctions);
        std::thread t2(randomlyExecuteFunctions);
        std::thread t3(randomlyExecuteFunctions);

        t1.join();
        t2.join();
        t3.join();
    }
}
