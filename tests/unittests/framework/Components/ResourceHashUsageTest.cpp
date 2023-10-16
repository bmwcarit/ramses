//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "internal/Components/ResourceHashUsageCallback.h"
#include "internal/Components/ResourceHashUsage.h"

namespace ramses::internal
{
    using testing::NiceMock;

    class MockIResourceHashUsageCallback : public IResourceHashUsageCallback
    {
    public:
        MOCK_METHOD(void, resourceHashUsageZero, (const ResourceContentHash& hash), (override));
    };

    TEST(AResourceHashUsage, callsOnZeroCallbackWhenGoingOutOfScope)
    {
        const ResourceContentHash hash(4765u, 0);
        MockIResourceHashUsageCallback mockCallback;
        ResourceHashUsageCallback callback(mockCallback);

        ResourceHashUsage usage(hash, callback);
        EXPECT_CALL(mockCallback, resourceHashUsageZero(hash));
        usage = ResourceHashUsage();

        ::testing::Mock::VerifyAndClearExpectations(&mockCallback);
    }

    TEST(AResourceHashUsage, callsOnZeroCallbackWhenLastUsageGoesOutOfScope)
    {
        const ResourceContentHash hash(4765u, 0);
        MockIResourceHashUsageCallback mockCallback;
        ResourceHashUsageCallback callback(mockCallback);

        ResourceHashUsage usage(hash, callback);
        ResourceHashUsage usage2 = usage;
        EXPECT_CALL(mockCallback, resourceHashUsageZero(hash)).Times(0);
        usage = ResourceHashUsage();

        ::testing::Mock::VerifyAndClearExpectations(&mockCallback);

        EXPECT_CALL(mockCallback, resourceHashUsageZero(hash));
        usage2 = ResourceHashUsage();
        ::testing::Mock::VerifyAndClearExpectations(&mockCallback);
    }

    TEST(AResourceHashUsage, isNotEqualWhenPointingDifferentHashes)
    {
        const ResourceContentHash hash(4765u, 0);
        const ResourceContentHash hash2(4765u, 0);
        NiceMock<MockIResourceHashUsageCallback> mockCallback;
        ResourceHashUsageCallback callback(mockCallback);

        ResourceHashUsage usage(hash, callback);
        ResourceHashUsage usage2(hash2, callback);
        EXPECT_NE(usage, usage2);
    }

    TEST(AResourceHashUsage, isEqualWhenPointingToSameHash)
    {
        const ResourceContentHash hash(4765u, 0);
        NiceMock<MockIResourceHashUsageCallback> mockCallback;
        ResourceHashUsageCallback callback(mockCallback);

        ResourceHashUsage usage(hash, callback);
        ResourceHashUsage usage2 = usage;
        (void)usage2;
        EXPECT_EQ(usage, usage2);
    }

    TEST(AResourceHashUsage, ReturnsCorrectHash)
    {
        const ResourceContentHash hash(4765u, 0);
        NiceMock<MockIResourceHashUsageCallback> mockCallback;
        ResourceHashUsageCallback callback(mockCallback);

        ResourceHashUsage usage(hash, callback);
        EXPECT_EQ(hash, usage.getHash());
    }
}
