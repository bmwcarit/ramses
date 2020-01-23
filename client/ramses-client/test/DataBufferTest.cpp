//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <array>

#include "ClientTestUtils.h"
#include "SceneImpl.h"
#include "ramses-utils.h"
#include "Scene/ClientScene.h"
#include "ramses-client-api/IndexDataBuffer.h"
#include "ramses-client-api/VertexDataBuffer.h"
#include "ramses-client-api/AttributeInput.h"
#include "IndexDataBufferImpl.h"
#include "VertexDataBufferImpl.h"
#include "RamsesObjectTypeUtils.h"

using namespace testing;
using namespace ramses_internal;

namespace ramses
{
    template<typename DataBufferT>
    class ADataBuffer : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        ADataBuffer()
            : LocalTestClientWithScene()
            , dataBuffer(*m_creationHelper.createObjectOfType<DataBufferT>("data buffer"))
            , dataBufferHandle(dataBuffer.impl.getDataBufferHandle())
        {
        }

        EDataType getCreationDataType();

        DataBufferT& dataBuffer;
        const ramses_internal::DataBufferHandle dataBufferHandle;
    };

    template<>
    EDataType ADataBuffer<IndexDataBuffer>::getCreationDataType()
    {
        return EDataType_UInt32;
    }

    template<>
    EDataType ADataBuffer<VertexDataBuffer>::getCreationDataType()
    {
        return EDataType_Float;
    }

    typedef ::testing::Types<IndexDataBuffer, VertexDataBuffer> DataBufferTestTypes;
    TYPED_TEST_CASE(ADataBuffer, DataBufferTestTypes);

    TYPED_TEST(ADataBuffer, IsAllocatedOnInternalSceneAfterCreation)
    {
        EXPECT_TRUE(this->dataBufferHandle.isValid());
        EXPECT_TRUE(this->m_scene.impl.getIScene().isDataBufferAllocated(this->dataBufferHandle));
    }

    TYPED_TEST(ADataBuffer, PropagatesDataChangesToInternalScene)
    {
        EXPECT_EQ(StatusOK, this->dataBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 3>{ {12, 23 ,34} }.data()), 3 * sizeof(uint32_t)));

        const uint32_t* dataBufferData = reinterpret_cast<const uint32_t*>(this->m_scene.impl.getIScene().getDataBuffer(this->dataBufferHandle).data.data());

        EXPECT_EQ(12u, dataBufferData[0]);
        EXPECT_EQ(23u, dataBufferData[1]);
        EXPECT_EQ(34u, dataBufferData[2]);

        EXPECT_EQ(StatusOK, this->dataBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 2>{ {6, 7} }.data()), 2 * sizeof(uint32_t), 1 * sizeof(uint32_t)));

        EXPECT_EQ(12u, dataBufferData[0]);
        EXPECT_EQ(6u, dataBufferData[1]);
        EXPECT_EQ(7u, dataBufferData[2]);
    }

    TYPED_TEST(ADataBuffer, CanNotBeUpdatedWhenDataSizeBiggerThanMaximumSize)
    {
        EXPECT_NE(StatusOK, this->dataBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 14>().data()), 14 * sizeof(uint32_t)));
    }

    TYPED_TEST(ADataBuffer, CanNotBeUpdatdWhenDataSizeAndOffsetBiggerThanMaximumSize)
    {
        EXPECT_NE(StatusOK, this->dataBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 4>().data()), 4 * sizeof(uint32_t), 10 * sizeof(uint32_t)));
    }

    TYPED_TEST(ADataBuffer, CanBeValidated)
    {
        const auto effect = TestEffects::CreateTestEffectWithAttribute(this->client);
        GeometryBinding& geom = this->createValidGeometry(effect);
        if (this->dataBuffer.getType() == ERamsesObjectType_IndexDataBuffer)
            geom.setIndices(RamsesObjectTypeUtils::ConvertTo<IndexDataBuffer>(this->dataBuffer));
        else
        {
            AttributeInput attrInput;
            effect->findAttributeInput("a_position", attrInput);
            geom.setInputBuffer(attrInput, RamsesObjectTypeUtils::ConvertTo<VertexDataBuffer>(this->dataBuffer));
        }
        const char data[] = { 0 };
        EXPECT_EQ(StatusOK, this->dataBuffer.setData(data, 1u));
        EXPECT_EQ(StatusOK, this->dataBuffer.validate());
    }

    TYPED_TEST(ADataBuffer, IsValidIfNotUsedByAnyMeshButUsedByPickableObject)
    {
        VertexDataBuffer* geometryBuffer = this->m_scene.createVertexDataBuffer(36, EDataType_Vector3F, "geometryBuffer");
        const char data[] = { 0 };
        geometryBuffer->setData(data, 1u);
        const pickableObjectId_t id(2);
        this->m_scene.createPickableObject(*geometryBuffer, id, "PickableObject");

        EXPECT_EQ(StatusOK, geometryBuffer->validate());
    }

    TYPED_TEST(ADataBuffer, ReportsWarningIfNotUsedInGeometry)
    {
        const char data[] = { 0 };
        EXPECT_EQ(StatusOK, this->dataBuffer.setData(data, 1u));
        EXPECT_NE(StatusOK, this->dataBuffer.validate());
    }

    TYPED_TEST(ADataBuffer, ReportsWarningIfUsedInGeometryButNotInitialized)
    {
        const auto effect = TestEffects::CreateTestEffectWithAttribute(this->client);
        GeometryBinding& geom = this->createValidGeometry(effect);
        if (this->dataBuffer.getType() == ERamsesObjectType_IndexDataBuffer)
            geom.setIndices(RamsesObjectTypeUtils::ConvertTo<IndexDataBuffer>(this->dataBuffer));
        else
        {
            AttributeInput attrInput;
            effect->findAttributeInput("a_position", attrInput);
            geom.setInputBuffer(attrInput, RamsesObjectTypeUtils::ConvertTo<VertexDataBuffer>(this->dataBuffer));
        }
        EXPECT_NE(StatusOK, this->dataBuffer.validate());
    }

    TYPED_TEST(ADataBuffer, CanGetDataType)
    {
        EXPECT_EQ(this->getCreationDataType(), this->dataBuffer.getDataType());
    }

    TYPED_TEST(ADataBuffer, CanGetMaximumSize)
    {
        EXPECT_EQ(13 * sizeof(uint32_t), this->dataBuffer.getMaximumSizeInBytes());
        EXPECT_EQ(13u, this->dataBuffer.impl.getElementCount());
    }

    TYPED_TEST(ADataBuffer, CanGetUsedSize)
    {
        EXPECT_EQ(StatusOK, this->dataBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 3>{ {12, 23, 34} }.data()), 3 * sizeof(uint32_t)));
        EXPECT_EQ(13 * sizeof(uint32_t), this->dataBuffer.getMaximumSizeInBytes());
        EXPECT_EQ(13u, this->dataBuffer.impl.getElementCount());
        EXPECT_EQ(3 * sizeof(uint32_t), this->dataBuffer.getUsedSizeInBytes());
        EXPECT_EQ(3u, this->dataBuffer.impl.getUsedElementCount());
    }

    TYPED_TEST(ADataBuffer, CanGetData)
    {
        EXPECT_EQ(StatusOK, this->dataBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 3>{ {12, 23, 34} }.data()), 3 * sizeof(uint32_t)));

        std::array<uint32_t, 13> dataBufferOut;
        EXPECT_EQ(StatusOK, this->dataBuffer.getData(reinterpret_cast<char*>(dataBufferOut.data()), static_cast<uint32_t>(dataBufferOut.size() * sizeof(uint32_t))));

        EXPECT_EQ(12u, dataBufferOut[0]);
        EXPECT_EQ(23u, dataBufferOut[1]);
        EXPECT_EQ(34u, dataBufferOut[2]);

        EXPECT_EQ(StatusOK, this->dataBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 2>{ {6, 7} }.data()), 2 * sizeof(uint32_t), 1 * sizeof(uint32_t)));

        EXPECT_EQ(StatusOK, this->dataBuffer.getData(reinterpret_cast<char*>(dataBufferOut.data()), static_cast<uint32_t>(dataBufferOut.size() * sizeof(uint32_t))));
        EXPECT_EQ(12u, dataBufferOut[0]);
        EXPECT_EQ(6u,  dataBufferOut[1]);
        EXPECT_EQ(7u,  dataBufferOut[2]);
    }

    TYPED_TEST(ADataBuffer, CanGetPartialData)
    {
        EXPECT_EQ(StatusOK, this->dataBuffer.setData(reinterpret_cast<const char*>(std::array<uint32_t, 3>{ {12, 23, 34} }.data()), 3 * sizeof(uint32_t)));

        uint32_t dataBufferOut;
        EXPECT_EQ(StatusOK, this->dataBuffer.getData(reinterpret_cast<char*>(&dataBufferOut), sizeof(uint32_t)));
        EXPECT_EQ(12u, dataBufferOut);
    }
}
