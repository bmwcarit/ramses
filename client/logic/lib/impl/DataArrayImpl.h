//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/EPropertyType.h"
#include "ramses-logic/EFeatureLevel.h"
#include "impl/LogicObjectImpl.h"
#include <string>
#include <variant>
#include <vector>
#include <memory>

namespace rlogic_serialization
{
    struct DataArray;
}

namespace flatbuffers
{
    template<typename T> struct Offset;
    class FlatBufferBuilder;
}

namespace rlogic::internal
{
    class ErrorReporting;
    class SerializationMap;

    class DataArrayImpl : public LogicObjectImpl
    {
    public:
        template <typename T>
        DataArrayImpl(std::vector<T>&& data, std::string_view name, uint64_t id);

        template <typename T>
        [[nodiscard]] const std::vector<T>* getData() const;
        [[nodiscard]] size_t getNumElements() const;
        [[nodiscard]] EPropertyType getDataType() const;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::DataArray> Serialize(
            const DataArrayImpl& data,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap,
            EFeatureLevel featureLevel);

        [[nodiscard]] static std::unique_ptr<DataArrayImpl> Deserialize(
            const rlogic_serialization::DataArray& data,
            ErrorReporting& errorReporting);

        using DataArrayVariant = std::variant<
            std::vector<float>,
            std::vector<vec2f>,
            std::vector<vec3f>,
            std::vector<vec4f>,
            std::vector<int32_t>,
            std::vector<vec2i>,
            std::vector<vec3i>,
            std::vector<vec4i>,
            std::vector<std::vector<float>>
        >;
        [[nodiscard]] const DataArrayVariant& getDataVariant() const;

    private:
        EPropertyType m_dataType = EPropertyType::Float;
        DataArrayVariant m_data;
    };
}
