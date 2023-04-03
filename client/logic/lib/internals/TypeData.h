//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/EPropertyType.h"

#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <iterator>

namespace rlogic::internal
{
    struct TypeData
    {
        TypeData(std::string _name, EPropertyType _type);

        bool operator==(const TypeData& other) const;
        bool operator!=(const TypeData& other) const;

        std::string name;
        EPropertyType type;
    };

    struct HierarchicalTypeData
    {
        HierarchicalTypeData(TypeData _typeData, std::vector<HierarchicalTypeData> _children);

        // Treats different ordering of child types as different types
        bool operator==(const HierarchicalTypeData& other) const;
        bool operator!=(const HierarchicalTypeData& other) const;

        TypeData typeData;
        std::vector<HierarchicalTypeData> children;
    };

    inline HierarchicalTypeData MakeType(std::string name, EPropertyType type)
    {
        return HierarchicalTypeData(TypeData(std::move(name), type), {});
    }

    inline HierarchicalTypeData MakeArray(std::string name, size_t size, EPropertyType type)
    {
        return HierarchicalTypeData(TypeData(std::move(name), EPropertyType::Array), std::vector<HierarchicalTypeData>(size, HierarchicalTypeData(TypeData("", type), {})));
    }

    inline HierarchicalTypeData MakeStruct(std::string name, std::initializer_list<TypeData> properties)
    {
        std::vector<HierarchicalTypeData> children;
        children.reserve(properties.size());
        std::transform(properties.begin(), properties.end(), std::back_inserter(children), [](const TypeData& propData){return HierarchicalTypeData(propData, {});});
        return HierarchicalTypeData(TypeData(std::move(name), EPropertyType::Struct), std::move(children));
    }

    inline HierarchicalTypeData MakeStruct(std::string name, const std::vector<TypeData>& properties)
    {
        std::vector<HierarchicalTypeData> children;
        children.reserve(properties.size());
        std::transform(properties.begin(), properties.end(), std::back_inserter(children), [](const TypeData& propData) {return HierarchicalTypeData(propData, {}); });
        return HierarchicalTypeData(TypeData(std::move(name), EPropertyType::Struct), std::move(children));
    }
}
