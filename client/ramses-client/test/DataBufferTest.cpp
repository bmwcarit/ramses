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
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/AttributeInput.h"
#include "ArrayBufferImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "DataTypeUtils.h"

using namespace testing;
using namespace ramses_internal;

namespace ramses
{
    class ADataBuffer : public LocalTestClientWithScene, public ::testing::TestWithParam<EDataType>
    {
    protected:
        ADataBuffer()
            : LocalTestClientWithScene()
            , elementSizeInBytes(EnumToSize(DataTypeUtils::ConvertDataTypeToInternal(GetParam())))
            , dataBuffer(*m_scene.createArrayBuffer(GetParam(), 13, "data buffer"))
            , dataBufferHandle(dataBuffer.impl.getDataBufferHandle())
            , singleElementData(elementSizeInBytes)
        {
            std::iota(singleElementData.begin(), singleElementData.end(), static_cast<Byte>(1));
        }

        EDataType getCreationDataType() { return GetParam(); }

        const uint32_t elementSizeInBytes;
        ArrayBuffer& dataBuffer;
        const ramses_internal::DataBufferHandle dataBufferHandle;
        std::vector<Byte> singleElementData;
    };

    TEST_P(ADataBuffer, IsAllocatedOnInternalSceneAfterCreation)
    {
        EXPECT_TRUE(this->dataBufferHandle.isValid());
        EXPECT_TRUE(this->m_scene.impl.getIScene().isDataBufferAllocated(this->dataBufferHandle));
    }

    TEST_P(ADataBuffer, PropagatesDataChangesToInternalScene)
    {
        EXPECT_EQ(StatusOK, this->dataBuffer.updateData(0u, 1u, singleElementData.data()));

        const Byte* dataBufferData = this->m_scene.impl.getIScene().getDataBuffer(this->dataBufferHandle).data.data();
        EXPECT_EQ(0, std::memcmp(singleElementData.data(), dataBufferData, singleElementData.size()));

        std::vector<Byte> twoElementsDataBuffer(elementSizeInBytes*2);
        std::iota(twoElementsDataBuffer.begin(), twoElementsDataBuffer.end(), static_cast<Byte>(20));
        EXPECT_EQ(StatusOK, this->dataBuffer.updateData(1u, 2u, twoElementsDataBuffer.data()));

        std::vector<Byte> expectedThreeeElements = singleElementData;
        expectedThreeeElements.insert(expectedThreeeElements.end(), twoElementsDataBuffer.begin(), twoElementsDataBuffer.end());
        ASSERT_EQ(dataBufferData, this->m_scene.impl.getIScene().getDataBuffer(this->dataBufferHandle).data.data());
        EXPECT_EQ(0, std::memcmp(expectedThreeeElements.data(), dataBufferData, expectedThreeeElements.size()));
    }

    TEST_P(ADataBuffer, CanNotBeUpdatedWhenDataSizeBiggerThanMaximumSize)
    {
        std::vector<Byte> data(elementSizeInBytes*14);
        EXPECT_NE(StatusOK, this->dataBuffer.updateData(0u, 14u, data.data()));
    }

    TEST_P(ADataBuffer, CanNotBeUpdatdWhenDataSizeAndOffsetBiggerThanMaximumSize)
    {
        std::vector<Byte> data(elementSizeInBytes*4);
        EXPECT_NE(StatusOK, this->dataBuffer.updateData(10u, 4u, data.data()));
    }

    TEST_P(ADataBuffer, CanBeValidated)
    {
        const auto effect = TestEffects::CreateTestEffectWithAttribute(this->m_scene);
        GeometryBinding& geom = this->createValidGeometry(effect);
        if (DataTypeUtils::IsValidIndicesType(this->dataBuffer.getDataType()))
            EXPECT_EQ(StatusOK, geom.setIndices(RamsesObjectTypeUtils::ConvertTo<ArrayBuffer>(this->dataBuffer)));
        else
        {
            const char* validInputName = nullptr;
            switch (GetParam())
            {
            case EDataType::Float:
                validInputName = "a_position";
                break;
            case EDataType::Vector2F:
                validInputName = "a_vec2";
                break;
            case EDataType::Vector3F:
                validInputName = "a_vec3";
                break;
            case EDataType::Vector4F:
                validInputName = "a_vec4";
                break;
            default: assert(false);
            }
            AttributeInput attrInput;
            effect->findAttributeInput(validInputName, attrInput);
            EXPECT_EQ(StatusOK, geom.setInputBuffer(attrInput, RamsesObjectTypeUtils::ConvertTo<ArrayBuffer>(this->dataBuffer)));
        }
        EXPECT_EQ(StatusOK, this->dataBuffer.updateData(0u, 1u, singleElementData.data()));
        EXPECT_EQ(StatusOK, this->dataBuffer.validate());
    }

    TEST_P(ADataBuffer, IsValidIfNotUsedByAnyMeshButUsedByPickableObject)
    {
        ArrayBuffer* geometryBuffer = this->m_scene.createArrayBuffer(EDataType::Vector3F, 3, "geometryBuffer");
        const float data[] = { 0.f, 0.f, 0.f };
        geometryBuffer->updateData(0u, 1u, data);
        const pickableObjectId_t id(2);
        this->m_scene.createPickableObject(*geometryBuffer, id, "PickableObject");

        EXPECT_EQ(StatusOK, geometryBuffer->validate());
    }

    TEST_P(ADataBuffer, ReportsWarningIfNotUsedInGeometry)
    {
        EXPECT_EQ(StatusOK, this->dataBuffer.updateData(0u, 1u, singleElementData.data()));
        EXPECT_NE(StatusOK, this->dataBuffer.validate());
    }

    TEST_P(ADataBuffer, ReportsWarningIfUsedInGeometryButNotInitialized)
    {
        const auto effect = TestEffects::CreateTestEffectWithAttribute(this->m_scene);
        GeometryBinding& geom = this->createValidGeometry(effect);
        if (DataTypeUtils::IsValidIndicesType(this->dataBuffer.getDataType()))
            geom.setIndices(RamsesObjectTypeUtils::ConvertTo<ArrayBuffer>(this->dataBuffer));
        else
        {
            AttributeInput attrInput;
            effect->findAttributeInput("a_position", attrInput);
            geom.setInputBuffer(attrInput, RamsesObjectTypeUtils::ConvertTo<ArrayBuffer>(this->dataBuffer));
        }
        EXPECT_NE(StatusOK, this->dataBuffer.validate());
    }

    TEST_P(ADataBuffer, CanGetDataType)
    {
        EXPECT_EQ(this->getCreationDataType(), this->dataBuffer.getDataType());
    }

    TEST_P(ADataBuffer, CanGetMaximumSize)
    {
        EXPECT_EQ(13u, this->dataBuffer.getMaximumNumberOfElements());
        EXPECT_EQ(13u, this->dataBuffer.impl.getElementCount());
    }

    TEST_P(ADataBuffer, CanGetUsedSize)
    {
        EXPECT_EQ(StatusOK, this->dataBuffer.updateData(0u, 1u, singleElementData.data()));
        EXPECT_EQ(13u, this->dataBuffer.getMaximumNumberOfElements());
        EXPECT_EQ(13u, this->dataBuffer.impl.getElementCount());
        EXPECT_EQ(1u, this->dataBuffer.getUsedNumberOfElements());
        EXPECT_EQ(1u, this->dataBuffer.impl.getUsedElementCount());
    }

    TEST_P(ADataBuffer, CanGetData)
    {
        EXPECT_EQ(StatusOK, this->dataBuffer.updateData(0u, 1u, singleElementData.data()));

        std::vector<Byte> dataBufferOut(singleElementData);
        EXPECT_EQ(StatusOK, this->dataBuffer.getData(dataBufferOut.data(), 1));
        EXPECT_EQ(singleElementData, dataBufferOut);

        std::vector<Byte> twoElementsDataBuffer(elementSizeInBytes*2);
        std::iota(twoElementsDataBuffer.begin(), twoElementsDataBuffer.end(), static_cast<Byte>(20));
        EXPECT_EQ(StatusOK, this->dataBuffer.updateData(1u, 2u, twoElementsDataBuffer.data()));

        std::vector<Byte> threeElementsDataBufferOut(elementSizeInBytes*3);
        EXPECT_EQ(StatusOK, this->dataBuffer.getData(threeElementsDataBufferOut.data(), 3u));

        std::vector<Byte> expectedThreeeElements = singleElementData;
        expectedThreeeElements.insert(expectedThreeeElements.end(), twoElementsDataBuffer.begin(), twoElementsDataBuffer.end());
        EXPECT_EQ(expectedThreeeElements,  threeElementsDataBufferOut);
    }

    INSTANTIATE_TEST_SUITE_P(ADataBufferTest, ADataBuffer,
        ::testing::Values(EDataType::UInt16, EDataType::UInt32, EDataType::Float, EDataType::Vector2F, EDataType::Vector3F, EDataType::Vector4F));
}
