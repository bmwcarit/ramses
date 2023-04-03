//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internals/SolWrapper.h"
#include "ramses-logic/EPropertyType.h"
#include "internals/TypeData.h"

namespace rlogic::internal
{
    struct InterfaceTypeInfo;

    class PropertyTypeExtractor
    {
    public:
        PropertyTypeExtractor(std::string rootName, EPropertyType rootType);

        // Obtain collected type info
        [[nodiscard]] HierarchicalTypeData getExtractedTypeData() const;

        // Used for iteration from rl_(i)pairs functions
        [[nodiscard]] std::reference_wrapper<const PropertyTypeExtractor> getChildReference(size_t childIndex) const;
        [[nodiscard]] TypeData getRootTypeData() const;
        [[nodiscard]] const std::vector<PropertyTypeExtractor>& getNestedExtractors() const;


        // Lua overloads
        [[nodiscard]] std::reference_wrapper<PropertyTypeExtractor> index(const sol::object& propertyIndex);
        void newIndex(const sol::object& idx, const sol::object& value);

        // Register symbols for type extraction to sol protected environment
        static void RegisterTypes(sol::table& environment);
        static void UnregisterTypes(sol::table& environment);

    private:
        TypeData m_typeData;

        using ChildProperties = std::vector<PropertyTypeExtractor>;

        ChildProperties m_children;

        ChildProperties::iterator findChild(const std::string_view name);
        void extractPropertiesFromTable(const sol::lua_table& table);
        void addField(const std::string& name, const InterfaceTypeInfo& typeInfo);
    };
}
