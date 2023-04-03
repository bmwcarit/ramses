//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/DataArrayImpl.h"
#include "generated/DataArrayGen.h"
#include "flatbuffers/flatbuffers.h"
#include "internals/ErrorReporting.h"
#include "LoggerImpl.h"
#include <cassert>

namespace rlogic::internal
{
    template <typename T>
    DataArrayImpl::DataArrayImpl(std::vector<T>&& data, std::string_view name, uint64_t id)
        : LogicObjectImpl(name, id)
        , m_dataType{ PropertyTypeToEnum<T>::TYPE }
        , m_data{ std::move(data) }
    {
    }

    template <typename T>
    const std::vector<T>* rlogic::internal::DataArrayImpl::getData() const
    {
        if (PropertyTypeToEnum<T>::TYPE != m_dataType)
        {
            LOG_ERROR("DataArray::getData failed for '{}', correct template that matches stored data type must be used.", getIdentificationString());
            return nullptr;
        }

        return &std::get<std::vector<T>>(m_data);
    }

    EPropertyType DataArrayImpl::getDataType() const
    {
        return m_dataType;
    }

    template <typename T>
    constexpr size_t getNumComponents(const std::vector<T>& data)
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            (void)data;
            return 1u;
        }
        else
        {
            assert(!data.empty());
            return data.front().size();
        }
    }

    template <typename T, typename fbT>
    std::vector<fbT> flattenArrayOfVec(const DataArrayImpl::DataArrayVariant& data)
    {
        const auto& dataVec = std::get<std::vector<T>>(data);
        std::vector<fbT> dataFlattened;
        dataFlattened.reserve(dataVec.size() * getNumComponents(dataVec));
        for (const auto& v : dataVec)
            dataFlattened.insert(dataFlattened.end(), v.cbegin(), v.cend());
        return dataFlattened;
    }

    flatbuffers::Offset<rlogic_serialization::DataArray> DataArrayImpl::Serialize(const DataArrayImpl& data, flatbuffers::FlatBufferBuilder& builder, SerializationMap& /*serializationMap*/, EFeatureLevel /*featureLevel*/)
    {
        rlogic_serialization::ArrayUnion unionType = rlogic_serialization::ArrayUnion::NONE;
        rlogic_serialization::EDataArrayType arrayType = rlogic_serialization::EDataArrayType::Float;
        flatbuffers::Offset<void> dataOffset;

        switch (data.m_dataType)
        {
        case EPropertyType::Float:
            unionType = rlogic_serialization::ArrayUnion::floatArr;
            arrayType = rlogic_serialization::EDataArrayType::Float;
            dataOffset = rlogic_serialization::CreatefloatArr(builder, builder.CreateVector(std::get<std::vector<float>>(data.m_data))).Union();
            break;
        case EPropertyType::Vec2f:
            unionType = rlogic_serialization::ArrayUnion::floatArr;
            arrayType = rlogic_serialization::EDataArrayType::Vec2f;
            dataOffset = rlogic_serialization::CreatefloatArr(builder, builder.CreateVector(flattenArrayOfVec<vec2f, float>(data.m_data))).Union();
            break;
        case EPropertyType::Vec3f:
            unionType = rlogic_serialization::ArrayUnion::floatArr;
            arrayType = rlogic_serialization::EDataArrayType::Vec3f;
            dataOffset = rlogic_serialization::CreatefloatArr(builder, builder.CreateVector(flattenArrayOfVec<vec3f, float>(data.m_data))).Union();
            break;
        case EPropertyType::Vec4f:
            unionType = rlogic_serialization::ArrayUnion::floatArr;
            arrayType = rlogic_serialization::EDataArrayType::Vec4f;
            dataOffset = rlogic_serialization::CreatefloatArr(builder, builder.CreateVector(flattenArrayOfVec<vec4f, float>(data.m_data))).Union();
            break;
        case EPropertyType::Int32:
            unionType = rlogic_serialization::ArrayUnion::intArr;
            arrayType = rlogic_serialization::EDataArrayType::Int32;
            dataOffset = rlogic_serialization::CreateintArr(builder, builder.CreateVector(std::get<std::vector<int32_t>>(data.m_data))).Union();
            break;
        case EPropertyType::Vec2i:
            unionType = rlogic_serialization::ArrayUnion::intArr;
            arrayType = rlogic_serialization::EDataArrayType::Vec2i;
            dataOffset = rlogic_serialization::CreateintArr(builder, builder.CreateVector(flattenArrayOfVec<vec2i, int32_t>(data.m_data))).Union();
            break;
        case EPropertyType::Vec3i:
            unionType = rlogic_serialization::ArrayUnion::intArr;
            arrayType = rlogic_serialization::EDataArrayType::Vec3i;
            dataOffset = rlogic_serialization::CreateintArr(builder, builder.CreateVector(flattenArrayOfVec<vec3i, int32_t>(data.m_data))).Union();
            break;
        case EPropertyType::Vec4i:
            unionType = rlogic_serialization::ArrayUnion::intArr;
            arrayType = rlogic_serialization::EDataArrayType::Vec4i;
            dataOffset = rlogic_serialization::CreateintArr(builder, builder.CreateVector(flattenArrayOfVec<vec4i, int32_t>(data.m_data))).Union();
            break;
        case EPropertyType::Array:
            unionType = rlogic_serialization::ArrayUnion::floatArr;
            arrayType = rlogic_serialization::EDataArrayType::FloatArray;
            dataOffset = rlogic_serialization::CreatefloatArr(builder, builder.CreateVector(flattenArrayOfVec<std::vector<float>, float>(data.m_data))).Union();
            break;
        case EPropertyType::Bool:
        default:
            assert(!"missing implementation");
            break;
        }

        const auto logicObject = LogicObjectImpl::Serialize(data, builder);
        auto animDataFB = rlogic_serialization::CreateDataArray(
            builder,
            logicObject,
            arrayType,
            unionType,
            dataOffset,
            static_cast<uint32_t>(data.getNumElements())
        );

        return animDataFB;
    }

    template <typename T, typename fbT, typename fbArrayT>
    bool checkFlatbufferVectorValidity(const rlogic_serialization::DataArray& data, ErrorReporting& errorReporting, uint32_t numComponents)
    {
        if (!data.data_as<fbArrayT>() || !data.data_as<fbArrayT>()->data())
        {
            errorReporting.add("Fatal error during loading of DataArray from serialized data: unexpected data type!", nullptr, EErrorType::BinaryVersionMismatch);
            return false;
        }
        const auto fbVec = data.data_as<fbArrayT>()->data();
        static_assert(std::is_arithmetic_v<fbT>, "wrong base type used");
        if (numComponents == 0u || fbVec->size() == 0u || (fbVec->size() % numComponents != 0))
        {
            errorReporting.add("Fatal error during loading of DataArray from serialized data: unexpected data size!", nullptr, EErrorType::BinaryVersionMismatch);
            return false;
        }

        return true;
    }

    template <typename T, typename fbT>
    std::vector<T> unflattenIntoArrayOfVec(const flatbuffers::Vector<fbT>& fbDataFlattened, uint32_t numComponents)
    {
        std::vector<T> dataVec;
        assert(fbDataFlattened.size() % numComponents == 0u); //checked in validation above
        dataVec.resize(fbDataFlattened.size() / numComponents);
        auto destIt = dataVec.begin();
        for (auto it = fbDataFlattened.cbegin(); it != fbDataFlattened.cend(); it += numComponents, ++destIt)
        {
            if constexpr(std::is_same_v<T, std::vector<float>>)
            {
                destIt->insert(destIt->end(), it, it + numComponents);
            }
            else
            {
                std::copy(it, it + numComponents, destIt->begin());
            }
        }
        return dataVec;
    }

    template <typename T, typename fbArrayT>
    uint32_t determineNumComponentsPerDataElement(const rlogic_serialization::DataArray& data)
    {
        if constexpr (std::is_same_v<T, std::vector<float>>)
        {
            if (!data.data_as<fbArrayT>() || !data.data_as<fbArrayT>()->data() || data.numElements() == 0u)
                return 0u;

            const auto numSerializedElements = data.data_as<fbArrayT>()->data()->size();
            // NOLINTNEXTLINE(clang-analyzer-core.DivideZero) wrong assumption from clang, tested right above for zero
            return numSerializedElements / data.numElements();
        }
        else
        {
            if constexpr (std::is_arithmetic_v<T>)
            {
                return 1u;
            }
            else
            {
                return std::tuple_size_v<T>;
            }
        }
    }

    std::unique_ptr<DataArrayImpl> DataArrayImpl::Deserialize(const rlogic_serialization::DataArray& data, ErrorReporting& errorReporting)
    {
        std::string name;
        uint64_t id = 0u;
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(data.base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.add("Fatal error during loading of DataArray from serialized data: missing name and/or ID!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::unique_ptr<DataArrayImpl> deserialized;
        switch (data.type())
        {
        case rlogic_serialization::EDataArrayType::Float:
        {
            const uint32_t numComponents = determineNumComponentsPerDataElement<float, rlogic_serialization::floatArr>(data);
            if (!checkFlatbufferVectorValidity<float, float, rlogic_serialization::floatArr>(data, errorReporting, numComponents))
                return nullptr;
            const auto& fbData = *data.data_as<rlogic_serialization::floatArr>()->data();
            auto dataVec = std::vector<float>{ fbData.cbegin(), fbData.cend() };
            deserialized = std::make_unique<DataArrayImpl>(std::move(dataVec), name, id);
            break;
        }
        case rlogic_serialization::EDataArrayType::Vec2f:
        {
            const uint32_t numComponents = determineNumComponentsPerDataElement<vec2f, rlogic_serialization::floatArr>(data);
            if (!checkFlatbufferVectorValidity<vec2f, float, rlogic_serialization::floatArr>(data, errorReporting, numComponents))
                return nullptr;
            auto dataVec = unflattenIntoArrayOfVec<vec2f, float>(*data.data_as<rlogic_serialization::floatArr>()->data(), numComponents);
            return std::make_unique<DataArrayImpl>(std::move(dataVec), name, id);
        }
        case rlogic_serialization::EDataArrayType::Vec3f:
        {
            const uint32_t numComponents = determineNumComponentsPerDataElement<vec3f, rlogic_serialization::floatArr>(data);
            if (!checkFlatbufferVectorValidity<vec3f, float, rlogic_serialization::floatArr>(data, errorReporting, numComponents))
                return nullptr;
            auto dataVec = unflattenIntoArrayOfVec<vec3f, float>(*data.data_as<rlogic_serialization::floatArr>()->data(), numComponents);
            deserialized = std::make_unique<DataArrayImpl>(std::move(dataVec), name, id);
            break;
        }
        case rlogic_serialization::EDataArrayType::Vec4f:
        {
            const uint32_t numComponents = determineNumComponentsPerDataElement<vec4f, rlogic_serialization::floatArr>(data);
            if (!checkFlatbufferVectorValidity<vec4f, float, rlogic_serialization::floatArr>(data, errorReporting, numComponents))
                return nullptr;
            auto dataVec = unflattenIntoArrayOfVec<vec4f, float>(*data.data_as<rlogic_serialization::floatArr>()->data(), numComponents);
            deserialized = std::make_unique<DataArrayImpl>(std::move(dataVec), name, id);
            break;
        }
        case rlogic_serialization::EDataArrayType::Int32:
        {
            const uint32_t numComponents = determineNumComponentsPerDataElement<int32_t, rlogic_serialization::intArr>(data);
            if (!checkFlatbufferVectorValidity<int32_t, int32_t, rlogic_serialization::intArr>(data, errorReporting, numComponents))
                return nullptr;
            const auto& fbData = *data.data_as<rlogic_serialization::intArr>()->data();
            auto dataVec = std::vector<int32_t>{ fbData.cbegin(), fbData.cend() };
            deserialized = std::make_unique<DataArrayImpl>(std::move(dataVec), name, id);
            break;
        }
        case rlogic_serialization::EDataArrayType::Vec2i:
        {
            const uint32_t numComponents = determineNumComponentsPerDataElement<vec2i, rlogic_serialization::intArr>(data);
            if (!checkFlatbufferVectorValidity<vec2i, int32_t, rlogic_serialization::intArr>(data, errorReporting, numComponents))
                return nullptr;
            auto dataVec = unflattenIntoArrayOfVec<vec2i, int32_t>(*data.data_as<rlogic_serialization::intArr>()->data(), numComponents);
            deserialized = std::make_unique<DataArrayImpl>(std::move(dataVec), name, id);
            break;
        }
        case rlogic_serialization::EDataArrayType::Vec3i:
        {
            const uint32_t numComponents = determineNumComponentsPerDataElement<vec3i, rlogic_serialization::intArr>(data);
            if (!checkFlatbufferVectorValidity<vec3i, int32_t, rlogic_serialization::intArr>(data, errorReporting, numComponents))
                return nullptr;
            auto dataVec = unflattenIntoArrayOfVec<vec3i, int32_t>(*data.data_as<rlogic_serialization::intArr>()->data(), numComponents);
            deserialized = std::make_unique<DataArrayImpl>(std::move(dataVec), name, id);
            break;
        }
        case rlogic_serialization::EDataArrayType::Vec4i:
        {
            const uint32_t numComponents = determineNumComponentsPerDataElement<vec4i, rlogic_serialization::intArr>(data);
            if (!checkFlatbufferVectorValidity<vec4i, int32_t, rlogic_serialization::intArr>(data, errorReporting, numComponents))
                return nullptr;
            auto dataVec = unflattenIntoArrayOfVec<vec4i, int32_t>(*data.data_as<rlogic_serialization::intArr>()->data(), numComponents);
            deserialized = std::make_unique<DataArrayImpl>(std::move(dataVec), name, id);
            break;
        }
        case rlogic_serialization::EDataArrayType::FloatArray:
        {
            const uint32_t numComponents = determineNumComponentsPerDataElement<std::vector<float>, rlogic_serialization::floatArr>(data);
            if (!checkFlatbufferVectorValidity<std::vector<float>, float, rlogic_serialization::floatArr>(data, errorReporting, numComponents))
                return nullptr;
            auto dataVec = unflattenIntoArrayOfVec<std::vector<float>, float>(*data.data_as<rlogic_serialization::floatArr>()->data(), numComponents);
            deserialized = std::make_unique<DataArrayImpl>(std::move(dataVec), name, id);
            break;
        }
        default:
            errorReporting.add(fmt::format("Fatal error during loading of DataArray from serialized data: unsupported or corrupt data type '{}'!", data.type()), nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        deserialized->setUserId(userIdHigh, userIdLow);

        return deserialized;
    }

    size_t DataArrayImpl::getNumElements() const
    {
        size_t numElements = 0u;
        std::visit([&numElements](const auto& v) { numElements = v.size(); }, m_data);
        return numElements;
    }

    const DataArrayImpl::DataArrayVariant& DataArrayImpl::getDataVariant() const
    {
        return m_data;
    }

    template DataArrayImpl::DataArrayImpl(std::vector<float>&& data, std::string_view name, uint64_t id);
    template DataArrayImpl::DataArrayImpl(std::vector<vec2f>&& data, std::string_view name, uint64_t id);
    template DataArrayImpl::DataArrayImpl(std::vector<vec3f>&& data, std::string_view name, uint64_t id);
    template DataArrayImpl::DataArrayImpl(std::vector<vec4f>&& data, std::string_view name, uint64_t id);
    template DataArrayImpl::DataArrayImpl(std::vector<int32_t>&& data, std::string_view name, uint64_t id);
    template DataArrayImpl::DataArrayImpl(std::vector<vec2i>&& data, std::string_view name, uint64_t id);
    template DataArrayImpl::DataArrayImpl(std::vector<vec3i>&& data, std::string_view name, uint64_t id);
    template DataArrayImpl::DataArrayImpl(std::vector<vec4i>&& data, std::string_view name, uint64_t id);
    template DataArrayImpl::DataArrayImpl(std::vector<std::vector<float>>&& data, std::string_view name, uint64_t id);

    template const std::vector<float>* DataArrayImpl::getData<float>() const;
    template const std::vector<vec2f>* DataArrayImpl::getData<vec2f>() const;
    template const std::vector<vec3f>* DataArrayImpl::getData<vec3f>() const;
    template const std::vector<vec4f>* DataArrayImpl::getData<vec4f>() const;
    template const std::vector<int32_t>* DataArrayImpl::getData<int32_t>() const;
    template const std::vector<vec2i>* DataArrayImpl::getData<vec2i>() const;
    template const std::vector<vec3i>* DataArrayImpl::getData<vec3i>() const;
    template const std::vector<vec4i>* DataArrayImpl::getData<vec4i>() const;
    template const std::vector<std::vector<float>>* DataArrayImpl::getData<std::vector<float>>() const;
}
