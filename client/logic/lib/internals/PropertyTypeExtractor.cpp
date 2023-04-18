//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/PropertyTypeExtractor.h"
#include "internals/SolHelper.h"
#include "internals/TypeUtils.h"
#include "internals/InterfaceTypeInfo.h"
#include "internals/InterfaceTypeFunctions.h"
#include "internals/LuaTypeConversions.h"

#include "fmt/format.h"
#include <algorithm>

namespace rlogic::internal
{
    PropertyTypeExtractor::PropertyTypeExtractor(std::string rootName, EPropertyType rootType)
        : m_typeData(std::move(rootName), rootType)
    {
    }

    std::reference_wrapper<PropertyTypeExtractor> PropertyTypeExtractor::index(const sol::object& propertyIndex)
    {
        auto childIter = m_children.end();
        if (m_typeData.type == EPropertyType::Struct)
        {
            const DataOrError<std::string_view> childName = LuaTypeConversions::ExtractSpecificType<std::string_view>(propertyIndex);
            if (childName.hasError())
            {
                sol_helper::throwSolException("Bad index access to struct '{}': {}", m_typeData.name, childName.getError());
            }

            childIter = findChild(childName.getData());

            if (childIter == m_children.end())
            {
                sol_helper::throwSolException("Field '{}' does not exist in struct '{}'!", childName.getData(), m_typeData.name);
            }
        }
        else if(m_typeData.type == EPropertyType::Array)
        {
            const DataOrError<size_t> childIndex = LuaTypeConversions::ExtractSpecificType<size_t>(propertyIndex);

            if (childIndex.hasError())
            {
                sol_helper::throwSolException("Invalid index access in array '{}': {}", m_typeData.name, childIndex.getError());
            }

            if (childIndex.getData() >= m_children.size())
            {
                sol_helper::throwSolException("Invalid index access in array '{}'. Expected index in the range [0, {}] but got {} instead!",
                    m_typeData.name, m_children.size(), childIndex.getData());
            }

            childIter = m_children.begin() + static_cast<int>(childIndex.getData());
        }

        PropertyTypeExtractor& refToChild = *childIter;
        return refToChild;
    }

    void PropertyTypeExtractor::newIndex(const sol::object& idx, const sol::object& value)
    {
        const DataOrError<std::string_view> potentiallyIndex = LuaTypeConversions::ExtractSpecificType<std::string_view>(idx);

        if (potentiallyIndex.hasError())
        {
            sol_helper::throwSolException("Invalid index for new field on struct '{}': {}", m_typeData.name, potentiallyIndex.getError());
        }

        const std::string idxAsStr(potentiallyIndex.getData());

        auto childIter = findChild(idxAsStr);
        if (childIter != m_children.end())
        {
            sol_helper::throwSolException("Field '{}' already exists! Can't declare the same field twice!", idxAsStr);
        }

        // TODO Violin consider if we want to forbid this
        if (value.get_type() == sol::type::table)
        {
            const std::optional<sol::lua_table> potentiallyTable = LuaTypeConversions::ExtractLuaTable(value);
            assert (potentiallyTable);
            PropertyTypeExtractor structProperty(idxAsStr, EPropertyType::Struct);
            structProperty.extractPropertiesFromTable(*potentiallyTable);
            m_children.emplace_back(std::move(structProperty));
        }
        else
        {
            if (value.is<InterfaceTypeInfo>())
            {
                const InterfaceTypeInfo& typeInfo = value.as<const InterfaceTypeInfo&>();
                addField(idxAsStr, typeInfo);
            }
            else
            {
                // TODO Violin maybe list all available types here in the error msg?
                sol_helper::throwSolException("Invalid type of field '{}'! Expected Type:T() syntax where T=Float,Int32,... Found a value of type '{}' instead",
                    idxAsStr, sol_helper::GetSolTypeName(value.get_type()));
            }
        }
    }

    void PropertyTypeExtractor::addField(const std::string& name, const InterfaceTypeInfo& typeInfo)
    {
        const EPropertyType typeId = typeInfo.typeId;

        if (!TypeUtils::IsValidType(typeId))
        {
            // TODO Violin can we sandbox this properly so it can never happen?
            sol_helper::throwSolException("Internal error: Invalid type id '{}'!", static_cast<int>(typeId));
        }

        switch (typeId)
        {
        case EPropertyType::Float:
        case EPropertyType::Vec2f:
        case EPropertyType::Vec3f:
        case EPropertyType::Vec4f:
        case EPropertyType::Int32:
        case EPropertyType::Int64:
        case EPropertyType::Vec2i:
        case EPropertyType::Vec3i:
        case EPropertyType::Vec4i:
        case EPropertyType::Bool:
        case EPropertyType::String:
            m_children.emplace_back(PropertyTypeExtractor{ std::string(name), typeId });
            break;
        case EPropertyType::Struct:
        {
            const std::optional<sol::lua_table> structFieldsTable = LuaTypeConversions::ExtractLuaTable(typeInfo.complexType);
            if (!structFieldsTable)
            {
                sol_helper::throwSolException("Invalid type of struct field '{}'! Expected Type:Struct(T) Where T is a table with named fields. Found a value of type '{}' instead!",
                    name, sol_helper::GetSolTypeName(typeInfo.complexType.get_type()));
            }

            PropertyTypeExtractor structProperty(name, EPropertyType::Struct);
            structProperty.extractPropertiesFromTable(*structFieldsTable);
            m_children.emplace_back(std::move(structProperty));
            break;
        }
        case EPropertyType::Array:
        {
            sol::optional<InterfaceTypeInfo> potentiallyArrayElementTypeInfo = typeInfo.complexType.as<sol::optional<InterfaceTypeInfo>>();

            // Special case for struct/tables. TODO Violin forbid?
            if (!potentiallyArrayElementTypeInfo)
            {
                const std::optional<sol::lua_table> maybeTable = LuaTypeConversions::ExtractLuaTable(typeInfo.complexType);
                if (maybeTable)
                {
                    potentiallyArrayElementTypeInfo = InterfaceTypeInfo{ EPropertyType::Struct, 0, *maybeTable };
                }
            }

            if (!potentiallyArrayElementTypeInfo)
            {
                sol_helper::throwSolException("Invalid element type T of array field '{}'! Found a value of type T='{}' instead of T=Type:<type>() in call {} = Type:Array(N, T)!",
                    name, sol_helper::GetSolTypeName(typeInfo.complexType.get_type()), name);
            }

            const InterfaceTypeInfo& elementTypeInfo = *potentiallyArrayElementTypeInfo;

            if (!TypeUtils::IsValidType(elementTypeInfo.typeId))
            {
                // TODO Violin can we sandbox this properly so it can never happen?
                sol_helper::throwSolException("Internal error: Invalid type id '{}'!", static_cast<int>(elementTypeInfo.typeId));
            }

            PropertyTypeExtractor arrayContainer(name, EPropertyType::Array);

            if (TypeUtils::IsPrimitiveType(elementTypeInfo.typeId))
            {
                arrayContainer.m_children.resize(typeInfo.arraySize, PropertyTypeExtractor("", elementTypeInfo.typeId));
            }
            else
            {
                // TODO Violin add tests for nested arrays
                assert(elementTypeInfo.typeId == EPropertyType::Array || elementTypeInfo.typeId == EPropertyType::Struct);

                // TODO Violin rework this hack to more elegant sol code
                PropertyTypeExtractor tmp("", EPropertyType::Struct);
                tmp.addField("", elementTypeInfo);

                arrayContainer.m_children.resize(typeInfo.arraySize, tmp.m_children[0]);
            }

            m_children.emplace_back(std::move(arrayContainer));
            break;
        }
        }
    }

    PropertyTypeExtractor::ChildProperties::iterator PropertyTypeExtractor::findChild(const std::string_view name)
    {
        return std::find_if(m_children.begin(), m_children.end(), [&name](const PropertyTypeExtractor& child) {return child.m_typeData.name == name; });
    }

    void PropertyTypeExtractor::extractPropertiesFromTable(const sol::lua_table& table)
    {
        for (const auto& tableEntry : table)
        {
            newIndex(tableEntry.first, tableEntry.second);
        }
    }

    void PropertyTypeExtractor::RegisterTypes(sol::table& environment)
    {
        environment.new_usertype<PropertyTypeExtractor>("LuaScriptPropertyExtractor",
            sol::meta_method::new_index, &PropertyTypeExtractor::newIndex,
            sol::meta_method::index, &PropertyTypeExtractor::index);
        environment.new_usertype<InterfaceTypeInfo>("InterfaceTypeInfo");
        sol::usertype<InterfaceTypeFunctions> usertypeTypeInfoFuncs = environment.new_usertype<InterfaceTypeFunctions>("Type");

        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::Float)] = &InterfaceTypeFunctions::CreatePrimitiveType<EPropertyType::Float>;
        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::Vec2f)] = &InterfaceTypeFunctions::CreatePrimitiveType<EPropertyType::Vec2f>;
        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::Vec3f)] = &InterfaceTypeFunctions::CreatePrimitiveType<EPropertyType::Vec3f>;
        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::Vec4f)] = &InterfaceTypeFunctions::CreatePrimitiveType<EPropertyType::Vec4f>;
        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::Int32)] = &InterfaceTypeFunctions::CreatePrimitiveType<EPropertyType::Int32>;
        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::Int64)] = &InterfaceTypeFunctions::CreatePrimitiveType<EPropertyType::Int64>;
        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::Vec2i)] = &InterfaceTypeFunctions::CreatePrimitiveType<EPropertyType::Vec2i>;
        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::Vec3i)] = &InterfaceTypeFunctions::CreatePrimitiveType<EPropertyType::Vec3i>;
        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::Vec4i)] = &InterfaceTypeFunctions::CreatePrimitiveType<EPropertyType::Vec4i>;
        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::String)] = &InterfaceTypeFunctions::CreatePrimitiveType<EPropertyType::String>;
        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::Bool)] = &InterfaceTypeFunctions::CreatePrimitiveType<EPropertyType::Bool>;
        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::Array)] = &InterfaceTypeFunctions::CreateArray;
        usertypeTypeInfoFuncs[GetLuaPrimitiveTypeName(EPropertyType::Struct)] = &InterfaceTypeFunctions::CreateStruct;
    }

    void PropertyTypeExtractor::UnregisterTypes(sol::table& environment)
    {
        auto propertyExtractorUserType = environment.get<sol::metatable>("LuaScriptPropertyExtractor");
        assert(propertyExtractorUserType.valid());
        propertyExtractorUserType.unregister();

        auto typeInfo = environment.get<sol::metatable>("InterfaceTypeInfo");
        assert(typeInfo.valid());
        typeInfo.unregister();

        auto usertypeTypeInfoFuncs = environment.get<sol::metatable>("Type");
        assert(usertypeTypeInfoFuncs.valid());
        usertypeTypeInfoFuncs.unregister();
    }

    HierarchicalTypeData PropertyTypeExtractor::getExtractedTypeData() const
    {
        // extract children type info first
        std::vector<HierarchicalTypeData> children;
        children.reserve(m_children.size());
        std::transform(m_children.begin(), m_children.end(), std::back_inserter(children),
            [](const PropertyTypeExtractor& childExtractor) { return childExtractor.getExtractedTypeData(); });

        // sort struct children by name lexicographically
        if (m_typeData.type == EPropertyType::Struct)
        {
            std::sort(children.begin(), children.end(), [](const HierarchicalTypeData& child1, const HierarchicalTypeData& child2)
                {
                    assert(!child1.typeData.name.empty() && !child2.typeData.name.empty());
                    return child1.typeData.name < child2.typeData.name;
                });
        }

        return HierarchicalTypeData(m_typeData, std::move(children));
    }

    std::reference_wrapper<const PropertyTypeExtractor> PropertyTypeExtractor::getChildReference(size_t childIndex) const
    {
        assert(childIndex < m_children.size());
        return m_children.at(childIndex);
    }

    TypeData PropertyTypeExtractor::getRootTypeData() const
    {
        return m_typeData;
    }

    const std::vector<PropertyTypeExtractor>& PropertyTypeExtractor::getNestedExtractors() const
    {
        return m_children;
    }
}
