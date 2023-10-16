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
#include "impl/SceneImpl.h"
#include "ramses/client/ramses-utils.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/AttributeInput.h"
#include "impl/ArrayBufferImpl.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/DataTypeUtils.h"
#include "glm/gtx/range.hpp"

using namespace testing;

namespace ramses::internal
{
    template <typename T>
    class AnArrayBuffer : public LocalTestClientWithScene, public ::testing::Test
    {
    protected:
        static constexpr ramses::EDataType GetDataType() { return GetEDataType<T>(); }

        static constexpr uint32_t ElementSizeInBytes = EnumToSize(DataTypeUtils::ConvertDataTypeToInternal(GetDataType()));

        std::vector<T> m_data{ SomeDataVector<T>() };
        ArrayBuffer& m_dataBuffer{ *m_scene.createArrayBuffer(GetDataType(), static_cast<uint32_t>(m_data.size()), "m_data buffer") };
    };

    using DataTypes = ::testing::Types<
        uint16_t,
        uint32_t,
        float,
        vec2f,
        vec3f,
        vec4f,
        std::byte
    >;

    TYPED_TEST_SUITE(AnArrayBuffer, DataTypes);

    TYPED_TEST(AnArrayBuffer, IsAllocatedOnInternalSceneAfterCreation)
    {
        EXPECT_TRUE(this->m_dataBuffer.impl().getDataBufferHandle().isValid());
        EXPECT_TRUE(this->m_scene.impl().getIScene().isDataBufferAllocated(this->m_dataBuffer.impl().getDataBufferHandle()));
    }

    TYPED_TEST(AnArrayBuffer, ContainsZeroedDataAfterCreation)
    {
        EXPECT_EQ(this->m_data.size(), this->m_dataBuffer.getMaximumNumberOfElements());
        EXPECT_EQ(0u, this->m_dataBuffer.getUsedNumberOfElements());

        std::vector<TypeParam> dataInBuffer(this->m_data.size());
        EXPECT_TRUE(this->m_dataBuffer.getData(dataInBuffer.data(), this->m_dataBuffer.getMaximumNumberOfElements()));
        for (auto v : dataInBuffer)
        {
            if constexpr (std::is_same_v<TypeParam, vec2f> || std::is_same_v<TypeParam, vec3f> || std::is_same_v<TypeParam, vec4f>)
            {
                for (auto e : v)
                    EXPECT_FLOAT_EQ(0.f, e);
            }
            else
            {
                EXPECT_EQ(static_cast<TypeParam>(0), v);
            }
        }
    }

    TYPED_TEST(AnArrayBuffer, CanUpdateWholeBuffer)
    {
        EXPECT_TRUE(this->m_dataBuffer.updateData(0u, this->m_dataBuffer.getMaximumNumberOfElements(), this->m_data.data()));

        std::vector<TypeParam> dataInBuffer(this->m_data.size());
        EXPECT_TRUE(this->m_dataBuffer.getData(dataInBuffer.data(), this->m_dataBuffer.getMaximumNumberOfElements()));
        EXPECT_EQ(this->m_data, dataInBuffer);
    }

    TYPED_TEST(AnArrayBuffer, CanUpdateSingleElement)
    {
        // set whole buffer first
        EXPECT_TRUE(this->m_dataBuffer.updateData(0u, this->m_dataBuffer.getMaximumNumberOfElements(), this->m_data.data()));

        // update only second element with first element from test data
        EXPECT_TRUE(this->m_dataBuffer.updateData(1u, 1u, &this->m_data[0]));
        std::vector<TypeParam> dataInBuffer(this->m_data.size());
        EXPECT_TRUE(this->m_dataBuffer.getData(dataInBuffer.data(), this->m_dataBuffer.getMaximumNumberOfElements()));
        EXPECT_EQ(this->m_data[0], dataInBuffer[1]);
        for (size_t i = 0u; i < this->m_data.size(); ++i)
        {
            if (i != 1u)
            {
                EXPECT_EQ(this->m_data[i], dataInBuffer[i]);
            }
        }

        // now update only first element with second element from test data
        EXPECT_TRUE(this->m_dataBuffer.updateData(0u, 1u, &this->m_data[1]));
        EXPECT_TRUE(this->m_dataBuffer.getData(dataInBuffer.data(), this->m_dataBuffer.getMaximumNumberOfElements()));
        EXPECT_EQ(this->m_data[0], dataInBuffer[1]);
        EXPECT_EQ(this->m_data[1], dataInBuffer[0]);
        for (size_t i = 0u; i < this->m_data.size(); ++i)
        {
            if (i != 0u && i != 1u)
            {
                EXPECT_EQ(this->m_data[i], dataInBuffer[i]);
            }
        }
    }

    TYPED_TEST(AnArrayBuffer, CanNotBeUpdatedWhenDataSizeBiggerThanMaximumSize)
    {
        this->m_data.push_back(this->m_data.front());
        EXPECT_FALSE(this->m_dataBuffer.updateData(0u, uint32_t(this->m_data.size()), this->m_data.data()));
    }

    TYPED_TEST(AnArrayBuffer, CanNotBeUpdatedWhenDataSizeAndOffsetBiggerThanMaximumSize)
    {
        EXPECT_FALSE(this->m_dataBuffer.updateData(1u, uint32_t(this->m_data.size()), this->m_data.data()));
    }

    TYPED_TEST(AnArrayBuffer, CanBeValidated)
    {
        const auto effect = TestEffects::CreateTestEffectWithAttribute(this->m_scene);
        Geometry& geom = this->createValidGeometry(effect);
        if (DataTypeUtils::IsValidIndicesType(this->m_dataBuffer.getDataType()))
        {
            EXPECT_TRUE(geom.setIndices(this->m_dataBuffer));
        }
        else if (ramses::EDataType::ByteBlob == this->GetDataType())
        {
            constexpr uint16_t nonZeroStride = 56u;
            EXPECT_TRUE(geom.setInputBuffer(*effect->findAttributeInput("a_position"), this->m_dataBuffer, 0u, nonZeroStride));
            EXPECT_TRUE(geom.setInputBuffer(*effect->findAttributeInput("a_vec2"), this->m_dataBuffer, sizeof(float), nonZeroStride) );
        }
        else
        {
            const char* validInputName = nullptr;
            switch (this->GetDataType())
            {
            case ramses::EDataType::Float:
                validInputName = "a_position";
                break;
            case ramses::EDataType::Vector2F:
                validInputName = "a_vec2";
                break;
            case ramses::EDataType::Vector3F:
                validInputName = "a_vec3";
                break;
            case ramses::EDataType::Vector4F:
                validInputName = "a_vec4";
                break;
            default: assert(false);
            }
            EXPECT_TRUE(geom.setInputBuffer(*effect->findAttributeInput(validInputName), this->m_dataBuffer));
        }
        EXPECT_TRUE(this->m_dataBuffer.updateData(0u, 1u, this->m_data.data()));
        ValidationReport report;
        this->m_dataBuffer.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TYPED_TEST(AnArrayBuffer, IsValidIfNotUsedByAnyMeshButUsedByPickableObject)
    {
        ArrayBuffer* geometryBuffer = this->m_scene.createArrayBuffer(ramses::EDataType::Vector3F, 3, "geometryBuffer");
        EXPECT_TRUE(geometryBuffer->updateData(0u, 1u, SomeDataVector<vec3f>().data()));
        const pickableObjectId_t id(2);
        ASSERT_TRUE(this->m_scene.createPickableObject(*geometryBuffer, id, "PickableObject"));

        ValidationReport report;
        geometryBuffer->validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TYPED_TEST(AnArrayBuffer, ReportsWarningIfNotUsedInGeometry)
    {
        EXPECT_TRUE(this->m_dataBuffer.updateData(0u, 1u, this->m_data.data()));
        ValidationReport report;
        this->m_dataBuffer.validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TYPED_TEST(AnArrayBuffer, ReportsWarningIfUsedInGeometryButNotInitialized)
    {
        const auto effect = TestEffects::CreateTestEffectWithAttribute(this->m_scene);
        Geometry& geom = this->createValidGeometry(effect);
        if (DataTypeUtils::IsValidIndicesType(this->m_dataBuffer.getDataType()))
        {
            geom.setIndices(this->m_dataBuffer);
        }
        else
        {
            geom.setInputBuffer(*effect->findAttributeInput("a_position"), this->m_dataBuffer);
        }
        ValidationReport report;
        this->m_dataBuffer.validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TYPED_TEST(AnArrayBuffer, CanGetDataType)
    {
        EXPECT_EQ(this->GetDataType(), this->m_dataBuffer.getDataType());
    }

    TYPED_TEST(AnArrayBuffer, CanGetMaximumSize)
    {
        EXPECT_EQ(this->m_data.size(), this->m_dataBuffer.getMaximumNumberOfElements());
        EXPECT_EQ(this->m_data.size(), this->m_dataBuffer.impl().getElementCount());
    }

    TYPED_TEST(AnArrayBuffer, CanGetUsedSize)
    {
        EXPECT_EQ(0u, this->m_dataBuffer.getUsedNumberOfElements());

        // set 1 element
        EXPECT_TRUE(this->m_dataBuffer.updateData(0u, 1u, this->m_data.data()));
        EXPECT_EQ(1u, this->m_dataBuffer.getUsedNumberOfElements());
        // set same element again - no change in used count
        EXPECT_TRUE(this->m_dataBuffer.updateData(0u, 1u, this->m_data.data()));
        EXPECT_EQ(1u, this->m_dataBuffer.getUsedNumberOfElements());

        // set 2 elements
        EXPECT_TRUE(this->m_dataBuffer.updateData(0u, 2u, this->m_data.data()));
        EXPECT_EQ(2u, this->m_dataBuffer.getUsedNumberOfElements());
        // set first element again - no change in used count
        EXPECT_TRUE(this->m_dataBuffer.updateData(0u, 1u, this->m_data.data()));
        EXPECT_EQ(2u, this->m_dataBuffer.getUsedNumberOfElements());

        // set all elements
        EXPECT_TRUE(this->m_dataBuffer.updateData(0u, uint32_t(this->m_data.size()), this->m_data.data()));
        EXPECT_EQ(uint32_t(this->m_data.size()), this->m_dataBuffer.getUsedNumberOfElements());
        EXPECT_EQ(this->m_dataBuffer.getMaximumNumberOfElements(), this->m_dataBuffer.getUsedNumberOfElements());
    }

    TYPED_TEST(AnArrayBuffer, FailsToUpdateIfUsingWrongType)
    {
        if (this->GetDataType() != ramses::EDataType::ByteBlob)
        {
            EXPECT_FALSE(this->m_dataBuffer.updateData(0u, 1u, SomeDataVector<std::byte>().data()));
        }
        else
        {
            EXPECT_FALSE(this->m_dataBuffer.updateData(0u, 1u, SomeDataVector<float>().data()));
        }
    }

    TYPED_TEST(AnArrayBuffer, FailsToGetDataIfUsingWrongType)
    {
        if (this->GetDataType() != ramses::EDataType::ByteBlob)
        {
            auto data = SomeDataVector<std::byte>();
            EXPECT_FALSE(this->m_dataBuffer.getData(data.data(), 1u));
        }
        else
        {
            auto data = SomeDataVector<float>();
            EXPECT_FALSE(this->m_dataBuffer.getData(data.data(), 1u));
        }
    }

    TEST(AnArrayBuffer, FailsToCreateForUnsupportedDataType)
    {
        LocalTestClientWithScene testScene;
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::Bool, 1u));
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::Int32, 1u));
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::Vector2I, 1u));
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::Vector3I, 1u));
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::Vector4I, 1u));
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::Matrix22F, 1u));
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::Matrix33F, 1u));
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::Matrix44F, 1u));
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::TextureSampler2D, 1u));
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::TextureSampler2DMS, 1u));
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::TextureSampler3D, 1u));
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::TextureSamplerCube, 1u));
        EXPECT_EQ(nullptr, testScene.getScene().createArrayBuffer(ramses::EDataType::TextureSamplerExternal, 1u));
    }
}
