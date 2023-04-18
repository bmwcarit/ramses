//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/TypeData.h"

namespace rlogic::internal
{
    TypeData::TypeData(std::string _name, EPropertyType _type)
        : name(std::move(_name))
        , type(_type)
    {
    }

    bool TypeData::operator!=(const TypeData& other) const
    {
        return !operator==(other);
    }

    bool TypeData::operator==(const TypeData& other) const
    {
        return name == other.name && type == other.type;
    }

    bool HierarchicalTypeData::operator!=(const HierarchicalTypeData& other) const
    {
        return !operator==(other);
    }

    bool HierarchicalTypeData::operator==(const HierarchicalTypeData& other) const
    {
        return typeData == other.typeData && children == other.children;
    }

    HierarchicalTypeData::HierarchicalTypeData(TypeData _typeData, std::vector<HierarchicalTypeData> _children)
        : typeData(std::move(_typeData))
        , children(std::move(_children))
    {

    }

}
