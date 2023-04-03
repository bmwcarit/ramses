//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "generated/PropertyGen.h"
#include "generated/DataArrayGen.h"
#include <unordered_map>
#include <string_view>

namespace rlogic::internal
{
    class PropertyImpl;

    // Remembers flatbuffer offsets for select objects during serialization
    class SerializationMap
    {
    public:
        void storePropertyOffset(const PropertyImpl& prop, flatbuffers::Offset<rlogic_serialization::Property> offset)
        {
            Store(&prop, offset, m_properties);
        }

        [[nodiscard]] flatbuffers::Offset<rlogic_serialization::Property> resolvePropertyOffset(const PropertyImpl& prop) const
        {
            return Get(&prop, m_properties);
        }

        void storeDataArray(uint64_t id, flatbuffers::Offset<rlogic_serialization::DataArray> offset)
        {
            Store(id, offset, m_dataArrays);
        }

        [[nodiscard]] flatbuffers::Offset<rlogic_serialization::DataArray> resolveDataArrayOffset(uint64_t dataArrayId) const
        {
            return Get(dataArrayId, m_dataArrays);
        }

        void storeByteCodeOffset(std::string byteCode, flatbuffers::Offset<flatbuffers::Vector<uint8_t>> offset)
        {
            m_byteCodeOffsets.emplace(std::move(byteCode), offset);
        }

        [[nodiscard]] flatbuffers::Offset<flatbuffers::Vector<uint8_t>> resolveByteCodeOffsetIfFound(const std::string& v)
        {
            const auto it = m_byteCodeOffsets.find(v);
            if (it != m_byteCodeOffsets.cend())
            {
                return it->second;
            }
            return 0;
        }

    private:
        template <typename Key, typename Value>
        static void Store(Key key, Value value, std::unordered_map<Key, Value>& container)
        {
            static_assert(std::is_trivially_copyable_v<Key> && std::is_trivially_copyable_v<Value>, "Performance warning");
            assert(container.count(key) == 0 && "one time store only");
            container.emplace(std::move(key), std::move(value));
        }

        template <typename Key, typename Value>
        [[nodiscard]] static Value Get(Key key, const std::unordered_map<Key, Value>& container)
        {
            const auto it = container.find(key);
            assert(it != container.cend());
            return it->second;
        }

        std::unordered_map<const PropertyImpl*, flatbuffers::Offset<rlogic_serialization::Property>> m_properties;
        std::unordered_map<uint64_t, flatbuffers::Offset<rlogic_serialization::DataArray>> m_dataArrays;
        std::unordered_map<std::string, flatbuffers::Offset<flatbuffers::Vector<uint8_t>>> m_byteCodeOffsets;
    };
}
