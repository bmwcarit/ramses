//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <unordered_map>

namespace rlogic_serialization
{
    struct Property;
    struct DataArray;
}

namespace rlogic
{
    class DataArray;
}

namespace rlogic::internal
{
    class PropertyImpl;
    class LogicObjectImpl;

    // Remembers flatbuffers pointers to deserialized objects temporarily during deserialization
    class DeserializationMap
    {
    public:
        void storePropertyImpl(const rlogic_serialization::Property& flatbufferObject, PropertyImpl& impl)
        {
            Store(&flatbufferObject, &impl, m_properties);
        }

        PropertyImpl& resolvePropertyImpl(const rlogic_serialization::Property& flatbufferObject) const
        {
            return *Get(&flatbufferObject, m_properties);
        }

        void storeDataArray(const rlogic_serialization::DataArray& flatbufferObject, const DataArray& dataArray)
        {
            Store(&flatbufferObject, &dataArray, m_dataArrays);
        }

        const DataArray& resolveDataArray(const rlogic_serialization::DataArray& flatbufferObject) const
        {
            return *Get(&flatbufferObject, m_dataArrays);
        }

        void storeLogicObject(uint64_t id, LogicObjectImpl& obj)
        {
            Store(id, &obj, m_logicObjects);
        }

        template <typename ImplT>
        ImplT* resolveLogicObject(uint64_t id) const
        {
            // fail queries using IDs gracefully if given ID not found
            // file can be OK on flatbuffer schema level but might still contain corrupted ID value
            const auto it = m_logicObjects.find(id);
            if (it != m_logicObjects.cend())
                return dynamic_cast<ImplT*>(it->second);

            return nullptr;
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

        std::unordered_map<const rlogic_serialization::Property*, PropertyImpl*> m_properties;
        std::unordered_map<const rlogic_serialization::DataArray*, const DataArray*> m_dataArrays;
        std::unordered_map<uint64_t, LogicObjectImpl*> m_logicObjects;
    };

}
