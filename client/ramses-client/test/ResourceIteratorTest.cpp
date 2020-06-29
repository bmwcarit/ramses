//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include <gtest/gtest.h>
#include "gtest/gtest-typed-test.h"
#include "ramses-client-api/ResourceIterator.h"
#include "ramses-client-api/FloatArray.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/EffectDescription.h"
#include "RamsesObjectTestTypes.h"
#include "CreationHelper.h"
#include "ClientTestUtils.h"
#include "RamsesObjectTypeTraits.h"

namespace ramses
{
    template <typename ResourceType>
    class RamsesClientResourceIteratorTest : public LocalTestClient, public ::testing::Test
    {
    public:
        RamsesClientResourceIteratorTest()
        {
        }

        ERamsesObjectType getTypeIDForResourceType()
        {
            return TYPE_ID_OF_RAMSES_OBJECT<ResourceType>::ID;
        }

        template<typename OtherResourceType>
        OtherResourceType* createResource(const char* name = nullptr)
        {
            return m_creationHelper.createObjectOfType<OtherResourceType>(name);
        }

    protected:
    };

    TYPED_TEST_SUITE(RamsesClientResourceIteratorTest, ResourceTypes);

    TYPED_TEST(RamsesClientResourceIteratorTest, getResourceIteratorEmpty)
    {
        ResourceIterator iter(this->client, this->getTypeIDForResourceType());

        EXPECT_TRUE(nullptr == iter.getNext());
    }

    TYPED_TEST(RamsesClientResourceIteratorTest, SkipsOtherTypes)
    {
        this->template createResource<UInt16Array>();
        this->template createResource<UInt32Array>();
        this->template createResource<FloatArray>();
        this->template createResource<Vector2fArray>();
        this->template createResource<Vector3fArray>();
        this->template createResource<Texture2D>();
        this->template createResource<Texture3D>();
        this->template createResource<TextureCube>();
        this->template createResource<Vector4fArray>();
        this->template createResource<Effect>();
        ResourceIterator iter(this->client, this->getTypeIDForResourceType());

        Resource* res = iter.getNext();
        ASSERT_TRUE(nullptr != res);
        EXPECT_EQ(this->getTypeIDForResourceType(), res->getType());
        EXPECT_TRUE(nullptr == iter.getNext());
    }

    TYPED_TEST(RamsesClientResourceIteratorTest, ReturnsMultipleIfAvailable)
    {
        this->template createResource<UInt16Array>();
        this->template createResource<UInt32Array>();
        this->template createResource<FloatArray>();
        this->template createResource<Vector2fArray>();
        this->template createResource<Vector3fArray>();
        this->template createResource<Texture2D>();
        this->template createResource<Texture3D>();
        this->template createResource<TextureCube>();
        this->template createResource<Vector4fArray>();
        this->template createResource<Effect>();
        this->template createResource<UInt16Array>();
        this->template createResource<UInt32Array>();
        this->template createResource<FloatArray>();
        this->template createResource<Vector2fArray>();
        this->template createResource<Vector3fArray>();
        this->template createResource<Texture2D>();
        this->template createResource<Texture3D>();
        this->template createResource<TextureCube>();
        this->template createResource<Vector4fArray>();
        this->template createResource<Effect>();
        ResourceIterator iter(this->client, this->getTypeIDForResourceType());

        Resource* res = iter.getNext();
        ASSERT_TRUE(nullptr != res);
        EXPECT_EQ(this->getTypeIDForResourceType(), res->getType());
        Resource* res2 = iter.getNext();
        ASSERT_TRUE(nullptr != res2);
        EXPECT_EQ(this->getTypeIDForResourceType(), res2->getType());
        EXPECT_NE(res, res2);
        EXPECT_TRUE(nullptr == iter.getNext());
    }
}
