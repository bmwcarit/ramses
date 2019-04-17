//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/SceneActionCollection.h"
#include "gtest/gtest.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/SceneVersionTag.h"
#include "Animation/AnimationCommon.h"

namespace ramses_internal
{
    template <typename T>
    class ASceneActionCollectionBasicTypes : public ::testing::Test
    {
    public:
        SceneActionCollection m_collection;
        static const T m_value;
    };

    // provide sensible default values for all tested types
    template<> const Int8                      ASceneActionCollectionBasicTypes<Int8>::m_value = -1;
    template<> const UInt8                     ASceneActionCollectionBasicTypes<UInt8>::m_value = 1u;
    template<> const Int16                     ASceneActionCollectionBasicTypes<Int16>::m_value = -2;
    template<> const UInt16                    ASceneActionCollectionBasicTypes<UInt16>::m_value = 2u;
    template<> const Int32                     ASceneActionCollectionBasicTypes<Int32>::m_value = -3;
    template<> const UInt32                    ASceneActionCollectionBasicTypes<UInt32>::m_value = 3u;
    template<> const Int64                     ASceneActionCollectionBasicTypes<Int64>::m_value = -4;
    template<> const UInt64                    ASceneActionCollectionBasicTypes<UInt64>::m_value = 4u;
    template<> const Float                     ASceneActionCollectionBasicTypes<Float>::m_value = 5.0f;
    template<> const Double                    ASceneActionCollectionBasicTypes<Double>::m_value = 6.0f;
    template<> const NodeHandle                ASceneActionCollectionBasicTypes<NodeHandle>::m_value = NodeHandle();
    template<> const RenderableHandle          ASceneActionCollectionBasicTypes<RenderableHandle>::m_value = RenderableHandle();
    template<> const TransformHandle           ASceneActionCollectionBasicTypes<TransformHandle>::m_value = TransformHandle();
    template<> const DataLayoutHandle          ASceneActionCollectionBasicTypes<DataLayoutHandle>::m_value = DataLayoutHandle();
    template<> const DataInstanceHandle        ASceneActionCollectionBasicTypes<DataInstanceHandle>::m_value = DataInstanceHandle();
    template<> const CameraHandle              ASceneActionCollectionBasicTypes<CameraHandle>::m_value = CameraHandle();
    template<> const RenderStateHandle         ASceneActionCollectionBasicTypes<RenderStateHandle>::m_value = RenderStateHandle();
    template<> const SplineHandle              ASceneActionCollectionBasicTypes<SplineHandle>::m_value = SplineHandle();
    template<> const AnimationHandle           ASceneActionCollectionBasicTypes<AnimationHandle>::m_value = AnimationHandle();
    template<> const AnimationInstanceHandle   ASceneActionCollectionBasicTypes<AnimationInstanceHandle>::m_value = AnimationInstanceHandle();
    template<> const DataBindHandle            ASceneActionCollectionBasicTypes<DataBindHandle>::m_value = DataBindHandle();
    template<> const RenderPassHandle          ASceneActionCollectionBasicTypes<RenderPassHandle>::m_value = RenderPassHandle();
    template<> const TextureSamplerHandle      ASceneActionCollectionBasicTypes<TextureSamplerHandle>::m_value = TextureSamplerHandle();
    template<> const RenderTargetHandle        ASceneActionCollectionBasicTypes<RenderTargetHandle>::m_value = RenderTargetHandle();
    template<> const QueueHandle               ASceneActionCollectionBasicTypes<QueueHandle>::m_value = QueueHandle();
    template<> const DataFieldHandle           ASceneActionCollectionBasicTypes<DataFieldHandle>::m_value = DataFieldHandle();
    template<> const SceneVersionTag           ASceneActionCollectionBasicTypes<SceneVersionTag>::m_value = SceneVersionTag();

    // types to test
    typedef ::testing::Types <
        Int8,
        UInt8,
        Int16,
        UInt16,
        Int32,
        UInt32,
        Int64,
        UInt64,
        Float,
        Double,
        NodeHandle,
        RenderableHandle,
        TransformHandle,
        DataLayoutHandle,
        DataInstanceHandle,
        CameraHandle,
        RenderStateHandle,
        SplineHandle,
        AnimationHandle,
        AnimationInstanceHandle,
        DataBindHandle,
        RenderPassHandle,
        TextureSamplerHandle,
        RenderTargetHandle,
        QueueHandle,
        DataFieldHandle,
        SceneVersionTag
    > SceneActionCollectionBasicTypes;


    TYPED_TEST_CASE(ASceneActionCollectionBasicTypes, SceneActionCollectionBasicTypes);

    TYPED_TEST(ASceneActionCollectionBasicTypes, WriteSingleElementAndCheckBufferSize)
    {
        this->m_collection.write(TestFixture::m_value);
        EXPECT_EQ(sizeof(TypeParam), this->m_collection.collectionData().size());
    }

    TYPED_TEST(ASceneActionCollectionBasicTypes, WriteAndReadSingleElement)
    {
        this->m_collection.beginWriteSceneAction(ESceneActionId_TestAction);
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
        const UInt32 numElements = 10;

        this->m_collection.beginWriteSceneAction(ESceneActionId_TestAction);
        for (UInt32 i = 0; i < numElements; ++i)
        {
            this->m_collection.write(TestFixture::m_value);
        }
        EXPECT_EQ(sizeof(TypeParam)*numElements, this->m_collection.collectionData().size());

        SceneActionCollection::SceneActionReader reader(this->m_collection[0]);
        EXPECT_EQ(0u, reader.offsetInCollection());
        EXPECT_EQ(sizeof(TypeParam)*numElements, reader.size());

        for (UInt32 i = 0; i < numElements; ++i)
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
        const UInt32 numReadIterations = 2;
        const UInt32 numElements = 10;

        this->m_collection.beginWriteSceneAction(ESceneActionId_TestAction);
        for (UInt i = 0; i < numElements; ++i)
        {
            this->m_collection.write(TestFixture::m_value);
        }
        EXPECT_EQ(sizeof(TypeParam)*numElements, this->m_collection.collectionData().size());

        for (UInt32 i = 0; i < numReadIterations; ++i)
        {
            SceneActionCollection::SceneActionReader reader(this->m_collection[0]);
            EXPECT_EQ(0u, reader.offsetInCollection());
            EXPECT_EQ(sizeof(TypeParam)*numElements, reader.size());

            for (UInt32 k = 0; k < numElements; ++k)
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
        const UInt32 numIterations = 4;
        const UInt32 numElements = 10;

        for (UInt32 i = 0; i < numIterations; ++i)
        {
            this->m_collection.beginWriteSceneAction(ESceneActionId_TestAction);
            for (UInt32 j = 0; j < numElements; ++j)
            {
                this->m_collection.write(TestFixture::m_value);
            }
        }
        EXPECT_EQ(numIterations, this->m_collection.numberOfActions());
        EXPECT_EQ(sizeof(TypeParam)*numElements*numIterations, this->m_collection.collectionData().size());

        for (UInt32 i = 0; i < numIterations; ++i)
        {
            SceneActionCollection::SceneActionReader reader(this->m_collection[i]);
            EXPECT_EQ(i* sizeof(TypeParam)*numElements, reader.offsetInCollection());
            EXPECT_EQ(sizeof(TypeParam)*numElements, reader.size());

            for (UInt32 k = 0; k < numElements; ++k)
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
        const UInt32 numElements = 5;
        SceneActionCollection other;

        this->m_collection.beginWriteSceneAction(ESceneActionId_TestAction);
        other.beginWriteSceneAction(ESceneActionId_TestAction);
        for (UInt32 i = 0; i < numElements; ++i)
        {
            this->m_collection.write(TestFixture::m_value);
            other.write(TestFixture::m_value);
        }

        EXPECT_TRUE(this->m_collection == other);
        EXPECT_FALSE(this->m_collection != other);
    }

    TYPED_TEST(ASceneActionCollectionBasicTypes, DifferentObjectsYieldNotEquality)
    {
        const UInt32 numElements = 5;
        SceneActionCollection other;

        this->m_collection.beginWriteSceneAction(ESceneActionId_TestAction);
        other.beginWriteSceneAction(ESceneActionId_TestAction);
        for (UInt32 i = 0; i < numElements; ++i)
        {
            this->m_collection.write(TestFixture::m_value);
            other.write(TestFixture::m_value);
        }
        other.write(TestFixture::m_value);

        EXPECT_FALSE(this->m_collection == other);
        EXPECT_TRUE(this->m_collection != other);
    }
}
