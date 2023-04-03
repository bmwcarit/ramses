//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "WithTempDirectory.h"
#include "FeatureLevelTestValues.h"

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/DataArray.h"
#include "impl/DataArrayImpl.h"
#include "internals/ErrorReporting.h"
#include "generated/DataArrayGen.h"

#include <fmt/format.h>

namespace rlogic::internal
{
    template <typename T>
    class ADataArray : public ::testing::Test
    {
    protected:
        LogicEngine m_logicEngine{ T::FeatureLevel };
    };

    template <typename T, EFeatureLevel F>
    struct DataTypeFeatureLevelPair
    {
        using DataType = T;
        static const EFeatureLevel FeatureLevel = F;
    };

    using DataTypes = ::testing::Types <
        DataTypeFeatureLevelPair<float, EFeatureLevel_01>,
        DataTypeFeatureLevelPair<float, EFeatureLevel_Latest>,
        DataTypeFeatureLevelPair<vec2f, EFeatureLevel_01>,
        DataTypeFeatureLevelPair<vec2f, EFeatureLevel_Latest>,
        DataTypeFeatureLevelPair<vec3f, EFeatureLevel_01>,
        DataTypeFeatureLevelPair<vec3f, EFeatureLevel_Latest>,
        DataTypeFeatureLevelPair<vec4f, EFeatureLevel_01>,
        DataTypeFeatureLevelPair<vec4f, EFeatureLevel_Latest>,
        DataTypeFeatureLevelPair<int32_t, EFeatureLevel_01>,
        DataTypeFeatureLevelPair<int32_t, EFeatureLevel_Latest>,
        DataTypeFeatureLevelPair<vec2i, EFeatureLevel_01>,
        DataTypeFeatureLevelPair<vec2i, EFeatureLevel_Latest>,
        DataTypeFeatureLevelPair<vec3i, EFeatureLevel_01>,
        DataTypeFeatureLevelPair<vec3i, EFeatureLevel_Latest>,
        DataTypeFeatureLevelPair<vec4i, EFeatureLevel_01>,
        DataTypeFeatureLevelPair<vec4i, EFeatureLevel_Latest>,
        DataTypeFeatureLevelPair<std::vector<float>, EFeatureLevel_Latest>
    >;
    TYPED_TEST_SUITE(ADataArray, DataTypes);

    template <typename T>
    std::vector<T> SomeDataVector();
    template <> std::vector<float> SomeDataVector() { return { 1.f, 2.f, 3.f }; }
    template <> std::vector<vec2f> SomeDataVector() { return { {1.f, 2.f}, {3.f, 4.f} }; }
    template <> std::vector<vec3f> SomeDataVector() { return { {1.f, 2.f, 3.f}, {3.f, 4.f, 5.f} }; }
    template <> std::vector<vec4f> SomeDataVector() { return { {1.f, 2.f, 3.f, 4.f}, {3.f, 4.f, 5.f, 6.f} }; }
    template <> std::vector<int32_t> SomeDataVector() { return { 1, 2, 3 }; }
    template <> std::vector<vec2i> SomeDataVector() { return { {1, 2}, {3, 4} }; }
    template <> std::vector<vec3i> SomeDataVector() { return { {1, 2, 3}, {3, 4, 5} }; }
    template <> std::vector<vec4i> SomeDataVector() { return { {1, 2, 3, 4}, {3, 4, 5, 6} }; }
    template <> std::vector<std::vector<float>> SomeDataVector() { return { {1.f, 2.f, 3.f, 4.f, 5.f}, {3.f, 4.f, 5.f, 6.f, 7.f} }; }

    TYPED_TEST(ADataArray, IsCreated)
    {
        const auto data = SomeDataVector<typename TypeParam::DataType>();
        const auto dataArray = this->m_logicEngine.createDataArray(data, "dataarray");
        EXPECT_TRUE(this->m_logicEngine.getErrors().empty());
        ASSERT_NE(nullptr, dataArray);
        EXPECT_EQ(dataArray, this->m_logicEngine.template findByName<DataArray>("dataarray"));

        EXPECT_EQ("dataarray", dataArray->getName());
        EXPECT_EQ(EPropertyType(PropertyTypeToEnum<typename TypeParam::DataType>::TYPE), dataArray->getDataType());
        ASSERT_NE(nullptr, dataArray->template getData<typename TypeParam::DataType>());
        EXPECT_EQ(data, *dataArray->template getData<typename TypeParam::DataType>());
        EXPECT_EQ(data.size(), dataArray->getNumElements());
    }

    TYPED_TEST(ADataArray, FailsToCreateIfEmptyDataProvided)
    {
        EXPECT_EQ(nullptr, this->m_logicEngine.createDataArray(std::vector<typename TypeParam::DataType>{}, "dataarray"));
        ASSERT_FALSE(this->m_logicEngine.getErrors().empty());
        EXPECT_EQ("Cannot create DataArray 'dataarray' with empty data.", this->m_logicEngine.getErrors().front().message);
    }

    TYPED_TEST(ADataArray, IsDestroyed)
    {
        const auto data = SomeDataVector<typename TypeParam::DataType>();
        auto dataArray = this->m_logicEngine.createDataArray(data, "dataarray");

        EXPECT_TRUE(this->m_logicEngine.destroy(*dataArray));
        EXPECT_TRUE(this->m_logicEngine.getErrors().empty());
        EXPECT_EQ(nullptr, this->m_logicEngine.template findByName<DataArray>("dataarray"));
    }

    TYPED_TEST(ADataArray, FailsToBeDestroyedIfFromOtherLogicInstance)
    {
        LogicEngine otherEngine{ TypeParam::FeatureLevel };
        auto dataArray = otherEngine.createDataArray(SomeDataVector<typename TypeParam::DataType>(), "dataarray");

        EXPECT_FALSE(this->m_logicEngine.destroy(*dataArray));
        ASSERT_FALSE(this->m_logicEngine.getErrors().empty());
        EXPECT_EQ("Can't find data array in logic engine!", this->m_logicEngine.getErrors().front().message);
    }

    TYPED_TEST(ADataArray, ChangesName)
    {
        auto dataArray = this->m_logicEngine.createDataArray(SomeDataVector<typename TypeParam::DataType>(), "dataarray");

        dataArray->setName("da");
        EXPECT_EQ("da", dataArray->getName());
        EXPECT_EQ(dataArray, this->m_logicEngine.template findByName<DataArray>("da"));
        EXPECT_TRUE(this->m_logicEngine.getErrors().empty());
    }

    TYPED_TEST(ADataArray, ReturnsNullIfWrongDataTypeQueried)
    {
        const auto data = SomeDataVector<typename TypeParam::DataType>();
        auto dataArray = this->m_logicEngine.createDataArray(data, "dataarray");
        if (dataArray->getDataType() != EPropertyType::Vec3f)
        {
            EXPECT_EQ(nullptr, dataArray->template getData<vec3f>());
        }
    }

    TYPED_TEST(ADataArray, CanBeSerializedAndDeserialized)
    {
        WithTempDirectory tempDir;

        const auto data1 = SomeDataVector<typename TypeParam::DataType>();
        const auto data2 = SomeDataVector<float>();
        const auto data3 = SomeDataVector<vec2f>();
        const auto data4 = SomeDataVector<vec3i>();
        const auto data5 = SomeDataVector<std::vector<float>>();

        {
            LogicEngine otherEngine{ TypeParam::FeatureLevel };

            otherEngine.createDataArray(data1, "dataarray1");
            otherEngine.createDataArray(data2, "dataarray2");
            otherEngine.createDataArray(data3, "dataarray3");
            otherEngine.createDataArray(data4, "dataarray4");
            if (TypeParam::FeatureLevel >= EFeatureLevel_04)
                otherEngine.createDataArray(data5, "dataarray5");

            ASSERT_TRUE(otherEngine.saveToFile("LogicEngine.bin"));
        }

        ASSERT_TRUE(this->m_logicEngine.loadFromFile("LogicEngine.bin"));
        EXPECT_TRUE(this->m_logicEngine.getErrors().empty());

        EXPECT_EQ((TypeParam::FeatureLevel >= EFeatureLevel_04 ? 5u : 4u), this->m_logicEngine.template getCollection<DataArray>().size());
        const auto dataArray1 = this->m_logicEngine.template findByName<DataArray>("dataarray1");
        const auto dataArray2 = this->m_logicEngine.template findByName<DataArray>("dataarray2");
        const auto dataArray3 = this->m_logicEngine.template findByName<DataArray>("dataarray3");
        const auto dataArray4 = this->m_logicEngine.template findByName<DataArray>("dataarray4");
        ASSERT_TRUE(dataArray1 && dataArray2 && dataArray3 && dataArray4);

        EXPECT_EQ(data1.size(), dataArray1->getNumElements());
        EXPECT_EQ(data2.size(), dataArray2->getNumElements());
        EXPECT_EQ(data3.size(), dataArray3->getNumElements());
        EXPECT_EQ(data4.size(), dataArray4->getNumElements());

        const auto loadedData1 = dataArray1->template getData<typename TypeParam::DataType>();
        const auto loadedData2 = dataArray2->template getData<float>();
        const auto loadedData3 = dataArray3->template getData<vec2f>();
        const auto loadedData4 = dataArray4->template getData<vec3i>();
        ASSERT_TRUE(loadedData1 && loadedData2 && loadedData3 && loadedData4);

        EXPECT_EQ(data1, *loadedData1);
        EXPECT_EQ(data2, *loadedData2);
        EXPECT_EQ(data3, *loadedData3);
        EXPECT_EQ(data4, *loadedData4);

        if (TypeParam::FeatureLevel >= EFeatureLevel_04)
        {
            const auto dataArray5 = this->m_logicEngine.template findByName<DataArray>("dataarray5");
            ASSERT_TRUE(dataArray5);
            EXPECT_EQ(data5.size(), dataArray5->getNumElements());
            const auto loadedData5 = dataArray5->template getData<std::vector<float>>();
            ASSERT_TRUE(loadedData5);
            EXPECT_EQ(data5, *loadedData5);
        }
    }

    TEST(ADataArrayOfFloatArrays, FailsToBeCreatedWithFeatureLevelBelow04)
    {
        const auto dataVector = SomeDataVector<std::vector<float>>();

        for (EFeatureLevel featureLevel : AllFeatureLevels)
        {
            if (featureLevel < EFeatureLevel_04)
            {
                LogicEngine logicEngine{ featureLevel };
                EXPECT_EQ(nullptr, logicEngine.createDataArray(dataVector, "dataarray"));
                ASSERT_EQ(1u, logicEngine.getErrors().size());
                EXPECT_EQ(logicEngine.getErrors()[0].message, fmt::format("Cannot create DataArray of float arrays, feature level 04 or higher is required, feature level in this runtime set to 0{}.", featureLevel));
            }
        }
    }

    TEST(ADataArrayOfFloatArrays, FailsToBeCreatedIfArraysSizesNotEqual)
    {
        const std::vector<std::vector<float>> dataVector = {
            { 1.f, 2.f, 3.f },
            { 1.f, 2.f, 3.f, 4.f },
            { 1.f, 2.f, 3.f, 4.f, 5.f },
            { 1.f, 2.f, 3.f }
        };

        LogicEngine logicEngine{ EFeatureLevel_Latest };
        EXPECT_EQ(nullptr, logicEngine.createDataArray(dataVector, "dataarray"));
        ASSERT_EQ(1u, logicEngine.getErrors().size());
        EXPECT_EQ(logicEngine.getErrors()[0].message, "Failed to create DataArray of float arrays: all arrays must be of same size.");
    }

    class ADataArray_SerializationLifecycle : public ::testing::TestWithParam<EFeatureLevel>
    {
    protected:
        LogicEngine m_logicEngine{ GetParam() };

        enum class ESerializationIssue
        {
            AllValid,
            NameMissing,
            IdMissing,
            NoData,
            WrongDataType,
            CorruptArrayDataType,
            WrongDataSize,
        };

        std::unique_ptr<DataArrayImpl> deserializeSerializedData(rlogic_serialization::ArrayUnion unionType, rlogic_serialization::EDataArrayType dataType, bool wrongElementCount = false)
        {
            flatbuffers::FlatBufferBuilder builder;

            flatbuffers::Offset<void> dataOffset;
            if (unionType == rlogic_serialization::ArrayUnion::floatArr)
            {
                std::vector<float> dummyData{ 0.1f, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }; //12 elements works for all vecX types
                if (wrongElementCount)
                {
                    dummyData.push_back(13);
                }

                dataOffset = rlogic_serialization::CreatefloatArr(builder, builder.CreateVector(dummyData)).Union();
            }
            else
            {
                std::vector<int32_t> dummyData{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }; //12 elements works for all vecX types
                if (wrongElementCount)
                {
                    dummyData.push_back(13);
                }

                dataOffset = rlogic_serialization::CreateintArr(builder, builder.CreateVector(dummyData)).Union();
            }

            uint32_t numElements = 0u;
            switch (dataType)
            {
            case rlogic_serialization::EDataArrayType::Float:
            case rlogic_serialization::EDataArrayType::Int32:
                numElements = 12u;
                break;
            case rlogic_serialization::EDataArrayType::Vec2f:
            case rlogic_serialization::EDataArrayType::Vec2i:
                numElements = 6u;
                break;
            case rlogic_serialization::EDataArrayType::Vec3f:
            case rlogic_serialization::EDataArrayType::Vec3i:
                numElements = 4u;
                break;
            case rlogic_serialization::EDataArrayType::Vec4f:
            case rlogic_serialization::EDataArrayType::Vec4i:
                numElements = 3u;
                break;
            case rlogic_serialization::EDataArrayType::FloatArray:
                numElements = 2u;
                break;
            default:
                assert(false);
                break;
            }

            auto dataArrayFB = rlogic_serialization::CreateDataArray(
                builder,
                rlogic_serialization::CreateLogicObject(builder, builder.CreateString("dataarray"), 1u),
                dataType,
                unionType,
                dataOffset,
                numElements
            );

            builder.Finish(dataArrayFB);

            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::DataArray>(builder.GetBufferPointer());
            return DataArrayImpl::Deserialize(serialized, m_errorReporting);
        }

        std::unique_ptr<DataArrayImpl> deserializeSerializedDataWithIssue(ESerializationIssue issue)
        {
            flatbuffers::FlatBufferBuilder flatBufferBuilder;

            auto dataOffset = issue == ESerializationIssue::WrongDataSize ? flatBufferBuilder.CreateVector(std::vector<float>{ 0.f, 1.f, 2.f }) : flatBufferBuilder.CreateVector(std::vector<float>{ 0.f, 1.f });

            rlogic_serialization::ArrayUnion unionType = rlogic_serialization::ArrayUnion::floatArr;

            if (issue == ESerializationIssue::NoData)
            {
                unionType = rlogic_serialization::ArrayUnion::NONE;
            }
            else if (issue == ESerializationIssue::WrongDataType)
            {
                unionType = rlogic_serialization::ArrayUnion::intArr;
            }

            const auto dataArrayFB = rlogic_serialization::CreateDataArray(
                flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(flatBufferBuilder,
                    issue == ESerializationIssue::NameMissing ? 0 : flatBufferBuilder.CreateString("dataArray"),
                    issue == ESerializationIssue::IdMissing ? 0 : 1u),
                issue == ESerializationIssue::CorruptArrayDataType ? static_cast<rlogic_serialization::EDataArrayType>(128) : rlogic_serialization::EDataArrayType::Vec2f,
                unionType,
                issue == ESerializationIssue::NoData ? 0 : rlogic_serialization::CreatefloatArr(flatBufferBuilder, dataOffset).Union()
            );

            flatBufferBuilder.Finish(dataArrayFB);

            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::DataArray>(flatBufferBuilder.GetBufferPointer());
            return DataArrayImpl::Deserialize(serialized, m_errorReporting);
        }

        ErrorReporting m_errorReporting;
    };

    INSTANTIATE_TEST_SUITE_P(
        ADataArray_SerializationLifecycle_TestInstances,
        ADataArray_SerializationLifecycle,
        GetFeatureLevelTestValues());

    TEST_P(ADataArray_SerializationLifecycle, ReportsNoDeserializationErrorsWhenAllDataCorrect)
    {
        EXPECT_TRUE(deserializeSerializedDataWithIssue(ADataArray_SerializationLifecycle::ESerializationIssue::AllValid));
        EXPECT_TRUE(this->m_errorReporting.getErrors().empty());
    }

    TEST_P(ADataArray_SerializationLifecycle, ReportsErrorWhenDeserializedWithoutName)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ADataArray_SerializationLifecycle::ESerializationIssue::NameMissing));
        ASSERT_EQ(2u, this->m_errorReporting.getErrors().size());
        EXPECT_EQ("Fatal error during loading of LogicObject base from serialized data: missing name!", this->m_errorReporting.getErrors()[0].message);
        EXPECT_EQ("Fatal error during loading of DataArray from serialized data: missing name and/or ID!", this->m_errorReporting.getErrors()[1].message);
    }

    TEST_P(ADataArray_SerializationLifecycle, ReportsErrorWhenDeserializedWithoutId)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ADataArray_SerializationLifecycle::ESerializationIssue::IdMissing));
        ASSERT_EQ(2u, this->m_errorReporting.getErrors().size());
        EXPECT_EQ("Fatal error during loading of LogicObject base from serialized data: missing or invalid ID!", this->m_errorReporting.getErrors()[0].message);
        EXPECT_EQ("Fatal error during loading of DataArray from serialized data: missing name and/or ID!", this->m_errorReporting.getErrors()[1].message);
    }

    TEST_P(ADataArray_SerializationLifecycle, ReportsErrorWhenDeserializedWithoutData)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ADataArray_SerializationLifecycle::ESerializationIssue::NoData));
        EXPECT_FALSE(this->m_errorReporting.getErrors().empty());
        EXPECT_EQ("Fatal error during loading of DataArray from serialized data: unexpected data type!", this->m_errorReporting.getErrors().front().message);
    }

    TEST_P(ADataArray_SerializationLifecycle, ReportsErrorWhenDeserializedWithWrongDataType)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ADataArray_SerializationLifecycle::ESerializationIssue::WrongDataType));
        EXPECT_FALSE(this->m_errorReporting.getErrors().empty());
        EXPECT_EQ("Fatal error during loading of DataArray from serialized data: unexpected data type!", this->m_errorReporting.getErrors().front().message);
    }

    TEST_P(ADataArray_SerializationLifecycle, ReportsErrorWhenDeserializedWithCorruptedDataUnion)
    {
        std::vector<std::pair<rlogic_serialization::ArrayUnion, rlogic_serialization::EDataArrayType>> invalidDataUnionPairs = {
            {rlogic_serialization::ArrayUnion::intArr, rlogic_serialization::EDataArrayType::Float},
            {rlogic_serialization::ArrayUnion::intArr, rlogic_serialization::EDataArrayType::Vec2f},
            {rlogic_serialization::ArrayUnion::intArr, rlogic_serialization::EDataArrayType::Vec3f},
            {rlogic_serialization::ArrayUnion::intArr, rlogic_serialization::EDataArrayType::Vec4f},
            {rlogic_serialization::ArrayUnion::floatArr, rlogic_serialization::EDataArrayType::Int32},
            {rlogic_serialization::ArrayUnion::floatArr, rlogic_serialization::EDataArrayType::Vec2i},
            {rlogic_serialization::ArrayUnion::floatArr, rlogic_serialization::EDataArrayType::Vec3i},
            {rlogic_serialization::ArrayUnion::floatArr, rlogic_serialization::EDataArrayType::Vec4i}
        };
        if (GetParam() >= EFeatureLevel_04)
            invalidDataUnionPairs.emplace_back(rlogic_serialization::ArrayUnion::intArr, rlogic_serialization::EDataArrayType::FloatArray);

        for(const auto& [unionType, dataArrayType] : invalidDataUnionPairs)
        {
            EXPECT_FALSE(deserializeSerializedData(unionType, dataArrayType));
            EXPECT_FALSE(this->m_errorReporting.getErrors().empty());
            EXPECT_EQ("Fatal error during loading of DataArray from serialized data: unexpected data type!", this->m_errorReporting.getErrors().front().message);
            this->m_errorReporting.clear();
        }
    }

    TEST_P(ADataArray_SerializationLifecycle, ReportsErrorWhenDeserializedWithCorruptedElementSizes)
    {
        std::vector<std::pair<rlogic_serialization::ArrayUnion, rlogic_serialization::EDataArrayType>> dataUnionPairs = {
            {rlogic_serialization::ArrayUnion::intArr, rlogic_serialization::EDataArrayType::Vec2i},
            {rlogic_serialization::ArrayUnion::intArr, rlogic_serialization::EDataArrayType::Vec3i},
            {rlogic_serialization::ArrayUnion::intArr, rlogic_serialization::EDataArrayType::Vec4i},
            {rlogic_serialization::ArrayUnion::floatArr, rlogic_serialization::EDataArrayType::Vec2f},
            {rlogic_serialization::ArrayUnion::floatArr, rlogic_serialization::EDataArrayType::Vec3f},
            {rlogic_serialization::ArrayUnion::floatArr, rlogic_serialization::EDataArrayType::Vec4f}
        };
        if (GetParam() >= EFeatureLevel_04)
            dataUnionPairs.emplace_back(rlogic_serialization::ArrayUnion::floatArr, rlogic_serialization::EDataArrayType::FloatArray);

        for (const auto& [unionType, dataArrayType] : dataUnionPairs)
        {
            EXPECT_FALSE(deserializeSerializedData(unionType, dataArrayType, true));
            EXPECT_FALSE(this->m_errorReporting.getErrors().empty());
            EXPECT_EQ("Fatal error during loading of DataArray from serialized data: unexpected data size!", this->m_errorReporting.getErrors().front().message);
            this->m_errorReporting.clear();
        }
    }

    TEST_P(ADataArray_SerializationLifecycle, ReportsErrorWhenDeserializedWithCorruptDataType)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ADataArray_SerializationLifecycle::ESerializationIssue::CorruptArrayDataType));
        EXPECT_FALSE(this->m_errorReporting.getErrors().empty());
        EXPECT_EQ("Fatal error during loading of DataArray from serialized data: unsupported or corrupt data type '128'!", this->m_errorReporting.getErrors().front().message);
    }

    TEST_P(ADataArray_SerializationLifecycle, ReportsErrorWhenDeserializedWithWrongDataSize)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ADataArray_SerializationLifecycle::ESerializationIssue::WrongDataSize));
        EXPECT_FALSE(this->m_errorReporting.getErrors().empty());
        EXPECT_EQ("Fatal error during loading of DataArray from serialized data: unexpected data size!", this->m_errorReporting.getErrors().front().message);
    }

    TEST(AnimationChannel, EqualityOperatorsTests)
    {
        LogicEngine engine;

        const auto dataVector1 = SomeDataVector<vec2f>();
        const auto dataVector2 = SomeDataVector<std::vector<float>>();

        DataArray* dataArray1 = engine.createDataArray(dataVector1, "dataarray1");
        DataArray* dataArray2 = engine.createDataArray(dataVector2, "dataarray2");

        AnimationChannel emptyChannel;
        AnimationChannel channel1 {
            /// Name of the channel for identification when linking
            "channel1",
            dataArray1,
            dataArray1,
            EInterpolationType::Linear,
            dataArray2,
            dataArray2
        };
        AnimationChannel channel1_differentName{
            /// Name of the channel for identification when linking
            "differentName",
            dataArray1,
            dataArray1,
            EInterpolationType::Linear,
            dataArray2,
            dataArray2
        };
        AnimationChannel channel1_differentInterpType{
            /// Name of the channel for identification when linking
            "differentName",
            dataArray1,
            dataArray1,
            EInterpolationType::Cubic,
            dataArray2,
            dataArray2
        };
        AnimationChannel channel2{
            /// Name of the channel for identification when linking
            "differentName",
            dataArray2,
            dataArray2,
            EInterpolationType::Cubic,
            dataArray1,
            dataArray1
        };

        // All objects are equal to itself
        EXPECT_EQ(emptyChannel, emptyChannel);
        EXPECT_EQ(channel1, channel1);
        EXPECT_EQ(channel1_differentName, channel1_differentName);
        EXPECT_EQ(channel1_differentInterpType, channel1_differentInterpType);
        EXPECT_EQ(channel2, channel2);

        // Empty channel is not equal to non-empty channels
        EXPECT_NE(emptyChannel, channel1);
        EXPECT_NE(emptyChannel, channel2);

        // Name is not take into account in equality check
        EXPECT_EQ(channel1, channel1_differentName);

        // Different data -> not equal
        EXPECT_NE(channel1, channel1_differentInterpType);
        EXPECT_NE(channel1, channel2);
    }
}
