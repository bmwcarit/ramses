//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "ramses-client-api/DataObject.h"
#include "ramses-utils.h"
#include "glm/gtx/range.hpp"

using namespace testing;
using namespace ramses_internal;

namespace ramses
{
    template <typename T>
    class ADataObject : public LocalTestClientWithScene, public testing::Test
    {
    };

    template <typename T>
    T SomeValue();
    template <> int32_t SomeValue() { return 1; }
    template <> float SomeValue() { return 2.f; }
    template <> vec2f SomeValue() { return vec2f{ 1.f, 2.f }; }
    template <> vec3f SomeValue() { return vec3f{ 1.f, 2.f, 3.f }; }
    template <> vec4f SomeValue() { return vec4f{ 1.f, 2.f, 3.f, 4.f }; }
    template <> vec2i SomeValue() { return vec2i{ 1, 2 }; }
    template <> vec3i SomeValue() { return vec3i{ 1, 2, 3 }; }
    template <> vec4i SomeValue() { return vec4i{ 1, 2, 3, 4 }; }
    template <> matrix22f SomeValue() { matrix22f ret; std::iota(begin(ret), end(ret), 1.f); return ret; }
    template <> matrix33f SomeValue() { matrix33f ret; std::iota(begin(ret), end(ret), 1.f); return ret; }
    template <> matrix44f SomeValue() { matrix44f ret; std::iota(begin(ret), end(ret), 1.f); return ret; }

    using DataTypes = ::testing::Types<
        int32_t,
        float,
        vec2f,
        vec3f,
        vec4f,
        vec2i,
        vec3i,
        vec4i,
        matrix22f,
        matrix33f,
        matrix44f
    >;
    TYPED_TEST_SUITE(ADataObject, DataTypes);

    TYPED_TEST(ADataObject, canGetDataType)
    {
        const auto dataObject = this->m_scene.createDataObject(GetEDataType<TypeParam>(), "data");
        ASSERT_TRUE(dataObject);
        EXPECT_EQ(GetEDataType<TypeParam>(), dataObject->getDataType());
    }

    TYPED_TEST(ADataObject, hasDefaultValueAfterCreation)
    {
        const auto dataObject = this->m_scene.createDataObject(GetEDataType<TypeParam>(), "data");
        ASSERT_TRUE(dataObject);

        TypeParam getVal;
        EXPECT_EQ(StatusOK, dataObject->getValue(getVal));
        EXPECT_EQ(TypeParam{}, getVal);
    }

    TYPED_TEST(ADataObject, canSetAndGetAValue)
    {
        const auto dataObject = this->m_scene.createDataObject(GetEDataType<TypeParam>(), "data");
        ASSERT_TRUE(dataObject);

        EXPECT_EQ(StatusOK, dataObject->setValue(SomeValue<TypeParam>()));

        TypeParam getVal;
        EXPECT_EQ(StatusOK, dataObject->getValue(getVal));
        EXPECT_EQ(SomeValue<TypeParam>(), getVal);
    }

    TYPED_TEST(ADataObject, failsToSetAndGetAValueIfUsingWrongDataType)
    {
        const auto dataObject = this->m_scene.createDataObject(GetEDataType<TypeParam>(), "data");
        ASSERT_TRUE(dataObject);

        if constexpr (std::is_same_v<TypeParam, int32_t>)
        {
            EXPECT_NE(StatusOK, dataObject->setValue(SomeValue<float>()));
            float getVal;
            EXPECT_NE(StatusOK, dataObject->getValue(getVal));
        }
        else
        {
            EXPECT_NE(StatusOK, dataObject->setValue(SomeValue<int32_t>()));
            int32_t getVal;
            EXPECT_NE(StatusOK, dataObject->getValue(getVal));
        }
    }

    TEST(ADataObject, failsToCreateIfUsingWrongDataType)
    {
        RamsesFramework framework{ RamsesFrameworkConfig{EFeatureLevel_Latest} };
        auto client = framework.createClient("test");
        auto scene = client->createScene(sceneId_t{ 123 });
        EXPECT_EQ(nullptr, scene->createDataObject(EDataType::UInt16));
        EXPECT_EQ(nullptr, scene->createDataObject(EDataType::UInt32));
        EXPECT_EQ(nullptr, scene->createDataObject(EDataType::ByteBlob));
        EXPECT_EQ(nullptr, scene->createDataObject(EDataType::TextureSampler2D));
        EXPECT_EQ(nullptr, scene->createDataObject(EDataType::TextureSampler2DMS));
        EXPECT_EQ(nullptr, scene->createDataObject(EDataType::TextureSampler3D));
        EXPECT_EQ(nullptr, scene->createDataObject(EDataType::TextureSamplerCube));
        EXPECT_EQ(nullptr, scene->createDataObject(EDataType::TextureSamplerExternal));
    }
}
