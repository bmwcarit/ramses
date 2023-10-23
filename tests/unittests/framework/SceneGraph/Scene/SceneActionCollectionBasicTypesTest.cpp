//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/Scene/SceneActionCollection.h"
#include "gtest/gtest.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/SceneVersionTag.h"

namespace ramses::internal
{
    template <typename T>
    class ASceneActionCollectionBasicTypes : public ::testing::Test
    {
    public:
        SceneActionCollection m_collection;
        static const T m_value;
    };

    // provide sensible default values for all tested types
    template<> const int8_t                    ASceneActionCollectionBasicTypes<int8_t>::m_value = -1;
    template<> const uint8_t                   ASceneActionCollectionBasicTypes<uint8_t>::m_value = 1u;
    template<> const int16_t                   ASceneActionCollectionBasicTypes<int16_t>::m_value = -2;
    template<> const uint16_t                  ASceneActionCollectionBasicTypes<uint16_t>::m_value = 2u;
    template<> const int32_t                   ASceneActionCollectionBasicTypes<int32_t>::m_value = -3;
    template<> const uint32_t                  ASceneActionCollectionBasicTypes<uint32_t>::m_value = 3u;
    template<> const int64_t                   ASceneActionCollectionBasicTypes<int64_t>::m_value = -4;
    template<> const uint64_t                  ASceneActionCollectionBasicTypes<uint64_t>::m_value = 4u;
    template<> const float                     ASceneActionCollectionBasicTypes<float>::m_value = 5.0f;
    template<> const double                    ASceneActionCollectionBasicTypes<double>::m_value = 6.0f;
    template<> const NodeHandle                ASceneActionCollectionBasicTypes<NodeHandle>::m_value = NodeHandle();
    template<> const RenderableHandle          ASceneActionCollectionBasicTypes<RenderableHandle>::m_value = RenderableHandle();
    template<> const TransformHandle           ASceneActionCollectionBasicTypes<TransformHandle>::m_value = TransformHandle();
    template<> const DataLayoutHandle          ASceneActionCollectionBasicTypes<DataLayoutHandle>::m_value = DataLayoutHandle();
    template<> const DataInstanceHandle        ASceneActionCollectionBasicTypes<DataInstanceHandle>::m_value = DataInstanceHandle();
    template<> const CameraHandle              ASceneActionCollectionBasicTypes<CameraHandle>::m_value = CameraHandle();
    template<> const RenderStateHandle         ASceneActionCollectionBasicTypes<RenderStateHandle>::m_value = RenderStateHandle();
    template<> const RenderPassHandle          ASceneActionCollectionBasicTypes<RenderPassHandle>::m_value = RenderPassHandle();
    template<> const TextureSamplerHandle      ASceneActionCollectionBasicTypes<TextureSamplerHandle>::m_value = TextureSamplerHandle();
    template<> const RenderTargetHandle        ASceneActionCollectionBasicTypes<RenderTargetHandle>::m_value = RenderTargetHandle();
    template<> const DataFieldHandle           ASceneActionCollectionBasicTypes<DataFieldHandle>::m_value = DataFieldHandle();
    template<> const SceneVersionTag           ASceneActionCollectionBasicTypes<SceneVersionTag>::m_value = SceneVersionTag();
    template<> const SceneReferenceHandle      ASceneActionCollectionBasicTypes<SceneReferenceHandle>::m_value = SceneReferenceHandle();

    // types to test
    using SceneActionCollectionBasicTypes = ::testing::Types <
        int8_t,
        uint8_t,
        int16_t,
        uint16_t,
        int32_t,
        uint32_t,
        int64_t,
        uint64_t,
        float,
        double,
        NodeHandle,
        RenderableHandle,
        TransformHandle,
        DataLayoutHandle,
        DataInstanceHandle,
        CameraHandle,
        RenderStateHandle,
        RenderPassHandle,
        TextureSamplerHandle,
        RenderTargetHandle,
        DataFieldHandle,
        SceneVersionTag,
        SceneReferenceHandle
    >;


    TYPED_TEST_SUITE(ASceneActionCollectionBasicTypes, SceneActionCollectionBasicTypes);

    TYPED_TEST(ASceneActionCollectionBasicTypes, WriteSingleElementAndCheckBufferSize)
    {
        this->m_collection.write(TestFixture::m_value);
        EXPECT_EQ(sizeof(TypeParam), this->m_collection.collectionData().size());
    }

    TYPED_TEST(ASceneActionCollectionBasicTypes, WriteAndReadSingleElement)
    {
        this->m_collection.beginWriteSceneAction(ESceneActionId::TestAction);
        this->m_collection.write(TestFixture::m_value);
        EXPECT_EQ(sizeof(TypeParam), this->m_collection.collectionData().size());

        SceneActionCollection::SceneActionReader reader(this->m_collection[0]);
        EXPECT_EQ(0u, reader.offsetInCollection());
        EXPECT_EQ(sizeof(TypeParam), reader.size());
        TypeParam value;
        reader.read(value);

        EXPECT_EQ(TestFixture::m_value, value);
        EXPECT_TRUE(reader.isFullyRead());
    }

    TYPED_TEST(ASceneActionCollectionBasicTypes, WriteAndReadMultipleElements)
    {
        const uint32_t numElements = 10;

        this->m_collection.beginWriteSceneAction(ESceneActionId::TestAction);
        for (uint32_t i = 0; i < numElements; ++i)
        {
            this->m_collection.write(TestFixture::m_value);
        }
        EXPECT_EQ(sizeof(TypeParam)*numElements, this->m_collection.collectionData().size());

        SceneActionCollection::SceneActionReader reader(this->m_collection[0]);
        EXPECT_EQ(0u, reader.offsetInCollection());
        EXPECT_EQ(sizeof(TypeParam)*numElements, reader.size());

        for (uint32_t i = 0; i < numElements; ++i)
        {
            ASSERT_FALSE(reader.isFullyRead());

            TypeParam value;
            reader.read(value);
            EXPECT_EQ(TestFixture::m_value, value);
        }
        EXPECT_TRUE(reader.isFullyRead());
    }

    TYPED_TEST(ASceneActionCollectionBasicTypes, PutMultipleElementsGetThemMultipleTimes)
    {
        const uint32_t numReadIterations = 2;
        const uint32_t numElements = 10;

        this->m_collection.beginWriteSceneAction(ESceneActionId::TestAction);
        for (size_t i = 0; i < numElements; ++i)
        {
            this->m_collection.write(TestFixture::m_value);
        }
        EXPECT_EQ(sizeof(TypeParam)*numElements, this->m_collection.collectionData().size());

        for (uint32_t i = 0; i < numReadIterations; ++i)
        {
            SceneActionCollection::SceneActionReader reader(this->m_collection[0]);
            EXPECT_EQ(0u, reader.offsetInCollection());
            EXPECT_EQ(sizeof(TypeParam)*numElements, reader.size());

            for (uint32_t k = 0; k < numElements; ++k)
            {
                ASSERT_FALSE(reader.isFullyRead());
                TypeParam value;
                reader.read(value);
                EXPECT_EQ(TestFixture::m_value, value);
            }
            EXPECT_TRUE(reader.isFullyRead());
        }
    }

    TYPED_TEST(ASceneActionCollectionBasicTypes, PutMultipleActionsWithMultipleElementsGetThemOnce)
    {
        const uint32_t numIterations = 4;
        const uint32_t numElements = 10;

        for (uint32_t i = 0; i < numIterations; ++i)
        {
            this->m_collection.beginWriteSceneAction(ESceneActionId::TestAction);
            for (uint32_t j = 0; j < numElements; ++j)
            {
                this->m_collection.write(TestFixture::m_value);
            }
        }
        EXPECT_EQ(numIterations, this->m_collection.numberOfActions());
        EXPECT_EQ(sizeof(TypeParam)*numElements*numIterations, this->m_collection.collectionData().size());

        for (uint32_t i = 0; i < numIterations; ++i)
        {
            SceneActionCollection::SceneActionReader reader(this->m_collection[i]);
            EXPECT_EQ(i* sizeof(TypeParam)*numElements, reader.offsetInCollection());
            EXPECT_EQ(sizeof(TypeParam)*numElements, reader.size());

            for (uint32_t k = 0; k < numElements; ++k)
            {
                ASSERT_FALSE(reader.isFullyRead());
                TypeParam value;
                reader.read(value);
                EXPECT_EQ(TestFixture::m_value, value);
            }
            EXPECT_TRUE(reader.isFullyRead());
        }
    }

    TYPED_TEST(ASceneActionCollectionBasicTypes, SameObjectsYieldEquality)
    {
        const uint32_t numElements = 5;
        SceneActionCollection other;

        this->m_collection.beginWriteSceneAction(ESceneActionId::TestAction);
        other.beginWriteSceneAction(ESceneActionId::TestAction);
        for (uint32_t i = 0; i < numElements; ++i)
        {
            this->m_collection.write(TestFixture::m_value);
            other.write(TestFixture::m_value);
        }

        EXPECT_TRUE(this->m_collection == other);
        EXPECT_FALSE(this->m_collection != other);
    }

    TYPED_TEST(ASceneActionCollectionBasicTypes, DifferentObjectsYieldNotEquality)
    {
        const uint32_t numElements = 5;
        SceneActionCollection other;

        this->m_collection.beginWriteSceneAction(ESceneActionId::TestAction);
        other.beginWriteSceneAction(ESceneActionId::TestAction);
        for (uint32_t i = 0; i < numElements; ++i)
        {
            this->m_collection.write(TestFixture::m_value);
            other.write(TestFixture::m_value);
        }
        other.write(TestFixture::m_value);

        EXPECT_FALSE(this->m_collection == other);
        EXPECT_TRUE(this->m_collection != other);
    }
}
