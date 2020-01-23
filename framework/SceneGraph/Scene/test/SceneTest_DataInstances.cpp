//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "SceneTest.h"
#include "framework_common_gmock_header.h"
#include "TestEqualHelper.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Matrix44f.h"

using namespace testing;

namespace ramses_internal
{
    TYPED_TEST_CASE(AScene, SceneTypes);

    TYPED_TEST(AScene, ContainsZeroTotalDataInstancesUponCreation)
    {
        EXPECT_EQ(0u, this->m_scene.getDataInstanceCount());
    }

    TYPED_TEST(AScene, InitializesDataInstanceFieldsWithZero)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({ DataFieldInfo(EDataType_Float), DataFieldInfo(EDataType_Float) }, ResourceContentHash(123u, 0u));
        const DataInstanceHandle containerHandle = this->m_scene.allocateDataInstance(dataLayout);

        EXPECT_EQ(0.0f, this->m_scene.getDataSingleFloat(containerHandle, DataFieldHandle(0u)));
        EXPECT_EQ(0.0f, this->m_scene.getDataSingleFloat(containerHandle, DataFieldHandle(1u)));
    }

    TYPED_TEST(AScene, InitializesDataInstanceBufferFieldsWithInvalidHashAndDataBufferAndZeroDivisor)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({ DataFieldInfo(EDataType_UInt16Buffer), DataFieldInfo(EDataType_FloatBuffer) }, ResourceContentHash(123u, 0u));
        const DataInstanceHandle containerHandle = this->m_scene.allocateDataInstance(dataLayout);

        {
            const ResourceField& dataResource = this->m_scene.getDataResource(containerHandle, DataFieldHandle(0u));
            EXPECT_FALSE(dataResource.hash.isValid());
            EXPECT_FALSE(dataResource.dataBuffer.isValid());
            EXPECT_EQ(0u, dataResource.instancingDivisor);
        }

        {
            const ResourceField& dataResource = this->m_scene.getDataResource(containerHandle, DataFieldHandle(1u));
            EXPECT_FALSE(dataResource.hash.isValid());
            EXPECT_FALSE(dataResource.dataBuffer.isValid());
            EXPECT_EQ(0u, dataResource.instancingDivisor);
        }
    }

    TYPED_TEST(AScene, SetsTheValueOfADataInstanceProperty)
    {
        const DataFieldInfoVector dataFields =
        {
            DataFieldInfo(EDataType_Float),
            DataFieldInfo(EDataType_Vector2F),
            DataFieldInfo(EDataType_Vector3F),
            DataFieldInfo(EDataType_Vector4F),
            DataFieldInfo(EDataType_Int32),
            DataFieldInfo(EDataType_Vector2I),
            DataFieldInfo(EDataType_Vector3I),
            DataFieldInfo(EDataType_Vector4I),
            DataFieldInfo(EDataType_Vector3Buffer),
            DataFieldInfo(EDataType_TextureSampler),
            DataFieldInfo(EDataType_Matrix22F),
            DataFieldInfo(EDataType_Matrix33F),
            DataFieldInfo(EDataType_Matrix44F),
            DataFieldInfo(EDataType_DataReference)
        };
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout(dataFields, ResourceContentHash(123u, 0u));
        const DataInstanceHandle containerHandle = this->m_scene.allocateDataInstance(dataLayout);

        const Matrix22f zeroMatrix22 = Matrix22f::Empty;
        const Matrix33f zeroMatrix33 = Matrix33f::Empty;
        const Matrix44f zeroMatrix44 = Matrix44f::Empty;

        // test default values
        EXPECT_EQ(0                                 , this->m_scene.getDataSingleFloat                    (containerHandle, DataFieldHandle(0u)));
        EXPECT_EQ(Vector2(0.0f)                     , this->m_scene.getDataSingleVector2f                 (containerHandle, DataFieldHandle(1u)));
        EXPECT_EQ(Vector3(0.0f)                     , this->m_scene.getDataSingleVector3f                 (containerHandle, DataFieldHandle(2u)));
        EXPECT_EQ(Vector4(0.0f)                     , this->m_scene.getDataSingleVector4f                 (containerHandle, DataFieldHandle(3u)));
        EXPECT_EQ(0                                 , this->m_scene.getDataSingleInteger                  (containerHandle, DataFieldHandle(4u)));
        EXPECT_EQ(Vector2i(0)                       , this->m_scene.getDataSingleVector2i                 (containerHandle, DataFieldHandle(5u)));
        EXPECT_EQ(Vector3i(0)                       , this->m_scene.getDataSingleVector3i                 (containerHandle, DataFieldHandle(6u)));
        EXPECT_EQ(Vector4i(0)                       , this->m_scene.getDataSingleVector4i                 (containerHandle, DataFieldHandle(7u)));
        {
            const ResourceField& dataResource = this->m_scene.getDataResource(containerHandle, DataFieldHandle(8u));
            EXPECT_EQ(ResourceContentHash::Invalid(), dataResource.hash);
            EXPECT_FALSE(dataResource.dataBuffer.isValid());
            EXPECT_EQ(0u, dataResource.instancingDivisor);
        }
        EXPECT_EQ(TextureSamplerHandle::Invalid()   , this->m_scene.getDataTextureSamplerHandle           (containerHandle, DataFieldHandle(9u)));
        EXPECT_TRUE(matrixFloatEquals(zeroMatrix22  , this->m_scene.getDataSingleMatrix22f                (containerHandle, DataFieldHandle(10u))));
        EXPECT_TRUE(matrixFloatEquals(zeroMatrix33  , this->m_scene.getDataSingleMatrix33f                (containerHandle, DataFieldHandle(11u))));
        EXPECT_TRUE(matrixFloatEquals(zeroMatrix44  , this->m_scene.getDataSingleMatrix44f                (containerHandle, DataFieldHandle(12u))));
        EXPECT_EQ(DataInstanceHandle::Invalid()     , this->m_scene.getDataReference                      (containerHandle, DataFieldHandle(13u)));

        //change values
        const ResourceContentHash hash(1234u, 0);
        const TextureSamplerHandle samplerHandle(4321u);
        const DataInstanceHandle dataRef = DataInstanceHandle(124u);

        this->m_scene.setDataSingleFloat          (containerHandle, DataFieldHandle(0u) , 12.3f);
        this->m_scene.setDataSingleVector2f       (containerHandle, DataFieldHandle(1u) , Vector2(0.12f, 0.34f));
        this->m_scene.setDataSingleVector3f       (containerHandle, DataFieldHandle(2u) , Vector3(0.13f, 0.35f, 0.46f));
        this->m_scene.setDataSingleVector4f       (containerHandle, DataFieldHandle(3u) , Vector4(0.14f, 0.36f, 0.47f, 0.58f));
        this->m_scene.setDataSingleInteger        (containerHandle, DataFieldHandle(4u) , 123);
        this->m_scene.setDataSingleVector2i       (containerHandle, DataFieldHandle(5u) , Vector2i(12, 34));
        this->m_scene.setDataSingleVector3i       (containerHandle, DataFieldHandle(6u) , Vector3i(13, 35, 46));
        this->m_scene.setDataSingleVector4i       (containerHandle, DataFieldHandle(7u) , Vector4i(14, 36, 47, 58));
        this->m_scene.setDataResource             (containerHandle, DataFieldHandle(8u) , hash, DataBufferHandle::Invalid(), 123u);
        this->m_scene.setDataTextureSamplerHandle (containerHandle, DataFieldHandle(9u), samplerHandle);
        this->m_scene.setDataSingleMatrix22f      (containerHandle, DataFieldHandle(10u), Matrix22f(1.f, 2.f, 3.f, 4.f));
        this->m_scene.setDataSingleMatrix33f      (containerHandle, DataFieldHandle(11u), Matrix33f::RotationEulerZYX(1.0f, 2.0f, 3.0f));
        this->m_scene.setDataSingleMatrix44f      (containerHandle, DataFieldHandle(12u), Matrix44f::Translation(1.0f, 2.0f, 3.0f));
        this->m_scene.setDataReference            (containerHandle, DataFieldHandle(13u), dataRef);

        EXPECT_EQ(12.3f                                 , this->m_scene.getDataSingleFloat                    (containerHandle, DataFieldHandle(0u)));
        EXPECT_EQ(Vector2(0.12f, 0.34f)                 , this->m_scene.getDataSingleVector2f                 (containerHandle, DataFieldHandle(1u)));
        EXPECT_EQ(Vector3(0.13f, 0.35f, 0.46f)          , this->m_scene.getDataSingleVector3f                 (containerHandle, DataFieldHandle(2u)));
        EXPECT_EQ(Vector4(0.14f, 0.36f, 0.47f, 0.58f)   , this->m_scene.getDataSingleVector4f                 (containerHandle, DataFieldHandle(3u)));
        EXPECT_EQ(123                                   , this->m_scene.getDataSingleInteger                  (containerHandle, DataFieldHandle(4u)));
        EXPECT_EQ(Vector2i(12, 34)                      , this->m_scene.getDataSingleVector2i                 (containerHandle, DataFieldHandle(5u)));
        EXPECT_EQ(Vector3i(13, 35, 46)                  , this->m_scene.getDataSingleVector3i                 (containerHandle, DataFieldHandle(6u)));
        EXPECT_EQ(Vector4i(14, 36, 47, 58)              , this->m_scene.getDataSingleVector4i                 (containerHandle, DataFieldHandle(7u)));
        {
            const ResourceField& dataResourceOut = this->m_scene.getDataResource(containerHandle, DataFieldHandle(8u));
            EXPECT_EQ(hash, dataResourceOut.hash);
            EXPECT_FALSE(dataResourceOut.dataBuffer.isValid());
            EXPECT_EQ(123u, dataResourceOut.instancingDivisor);
        }
        EXPECT_EQ(samplerHandle                         , this->m_scene.getDataTextureSamplerHandle           (containerHandle, DataFieldHandle(9u)));
        EXPECT_TRUE(matrixFloatEquals(Matrix22f(1.f, 2.f, 3.f, 4.f), this->m_scene.getDataSingleMatrix22f     (containerHandle, DataFieldHandle(10u))));
        EXPECT_TRUE(matrixFloatEquals(Matrix33f::RotationEulerZYX(1.0f, 2.0f, 3.0f), this->m_scene.getDataSingleMatrix33f(containerHandle, DataFieldHandle(11u))));
        EXPECT_TRUE(matrixFloatEquals(Matrix44f::Translation(1.0f, 2.0f, 3.0f), this->m_scene.getDataSingleMatrix44f(containerHandle, DataFieldHandle(12u))));
        EXPECT_EQ(dataRef                               , this->m_scene.getDataReference                      (containerHandle, DataFieldHandle(13u)));

        this->m_scene.setDataResource(containerHandle, DataFieldHandle(8u), ResourceContentHash::Invalid(), DataBufferHandle(1u), 123u);
        {
            const ResourceField& dataResourceOut = this->m_scene.getDataResource(containerHandle, DataFieldHandle(8u));
            EXPECT_EQ(ResourceContentHash::Invalid(), dataResourceOut.hash);
            EXPECT_EQ(DataBufferHandle(1u), dataResourceOut.dataBuffer);
        }
    }

    TYPED_TEST(AScene, CreatingDataInstanceIncreasesDataInstanceCount)
    {
        DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({}, ResourceContentHash(123u, 0u));
        this->m_scene.allocateDataInstance(dataLayout);

        EXPECT_EQ(1u, this->m_scene.getDataInstanceCount());
    }

    TYPED_TEST(AScene, DataInstanceWithFieldWithElementCountGreaterOneReturnsSameValue)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({ DataFieldInfo(EDataType_Int32, 4u) }, ResourceContentHash(123u, 0u));
        const DataFieldHandle field(0u);

        const DataInstanceHandle instance = this->m_scene.allocateDataInstance(dataLayout);
        const Int32 inValues[4] = { 1, 2, 3, 4 };
        this->m_scene.setDataIntegerArray(instance, field, 4, inValues);

        const Int32* outValues = this->m_scene.getDataIntegerArray(instance, field);
        EXPECT_EQ(inValues[0], outValues[0]);
        EXPECT_EQ(inValues[1], outValues[1]);
        EXPECT_EQ(inValues[2], outValues[2]);
        EXPECT_EQ(inValues[3], outValues[3]);
    }

    TYPED_TEST(AScene, DataInstanceFieldsWithElementCountGreaterOneDoesNotInfluenceOtherField)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({ DataFieldInfo(EDataType_Float, 3u), DataFieldInfo(EDataType_Vector4Buffer) }, ResourceContentHash::Invalid());

        const DataInstanceHandle instance = this->m_scene.allocateDataInstance(dataLayout);
        const Float inValues0[3] = { 0.f, 0.f, 0.f };
        const ResourceContentHash inValue1(std::numeric_limits<UInt64>::max(), std::numeric_limits<UInt64>::max());
        const UInt32 inDivisor = std::numeric_limits<UInt32>::max();
        this->m_scene.setDataFloatArray(instance, DataFieldHandle(0u), 3, inValues0);
        this->m_scene.setDataResource(instance, DataFieldHandle(1u), inValue1, DataBufferHandle::Invalid(), inDivisor);

        const Float* outValues0 = this->m_scene.getDataFloatArray(instance, DataFieldHandle(0u));
        EXPECT_EQ(inValues0[0], outValues0[0]);
        EXPECT_EQ(inValues0[1], outValues0[1]);
        EXPECT_EQ(inValues0[2], outValues0[2]);
        const ResourceField& dataResourceOut = this->m_scene.getDataResource(instance, DataFieldHandle(1u));
        EXPECT_EQ(inValue1, dataResourceOut.hash);
        EXPECT_FALSE(dataResourceOut.dataBuffer.isValid());
        EXPECT_EQ(inDivisor, dataResourceOut.instancingDivisor);
    }
}
