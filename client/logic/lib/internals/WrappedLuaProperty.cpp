//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/WrappedLuaProperty.h"
#include "internals/SolHelper.h"
#include "internals/LuaTypeConversions.h"
#include "internals/TypeUtils.h"

#include "impl/PropertyImpl.h"

namespace rlogic::internal
{
    class BadStructAccess : public sol::error
    {
    public:
        BadStructAccess(std::string _fieldName, std::string solError)
            : sol::error(std::move(solError))
            , fieldName(std::move(_fieldName))
        {
        }

        std::string fieldName;
    };

    WrappedLuaProperty::WrappedLuaProperty(PropertyImpl& propertyToWrap)
        : m_wrappedProperty(propertyToWrap)
    {
        m_wrappedChildProperties.reserve(propertyToWrap.getChildCount());

        for (size_t i = 0; i < propertyToWrap.getChildCount(); ++i)
        {
            m_wrappedChildProperties.emplace_back(*propertyToWrap.getChild(i)->m_impl);
        }
    }

    sol::object WrappedLuaProperty::index(sol::this_state solState, const sol::object& index) const
    {
        switch (m_wrappedProperty.get().getType())
        {
        // Primitive types don't have sub-types and thus can't be indexed (we never return a wrapper of primitive
        // type to Lua, instead we resolve it first to a Lua built-in (e.g. string or number)
        case EPropertyType::Float:
        case EPropertyType::Int32:
        case EPropertyType::Int64:
        case EPropertyType::String:
        case EPropertyType::Bool:
            sol_helper::throwSolException("Implementation error!");
            break;
        case EPropertyType::Vec2f:
            return extractVectorComponent<float, 2>(solState, index);
        case EPropertyType::Vec3f:
            return extractVectorComponent<float, 3>(solState, index);
        case EPropertyType::Vec4f:
            return extractVectorComponent<float, 4>(solState, index);
        case EPropertyType::Vec2i:
            return extractVectorComponent<int32_t, 2>(solState, index);
        case EPropertyType::Vec3i:
            return extractVectorComponent<int32_t, 3>(solState, index);
        case EPropertyType::Vec4i:
            return extractVectorComponent<int32_t, 4>(solState, index);
        case EPropertyType::Array:
        case EPropertyType::Struct:
            return resolveChild(solState, resolvePropertyIndex(index));
        }

        assert(false && "Missing type implementation!");
        return sol::lua_nil;
    }

    sol::object WrappedLuaProperty::resolveChild(sol::this_state solState, size_t childIndex) const
    {
        const WrappedLuaProperty& child = m_wrappedChildProperties[childIndex];
        const PropertyImpl& childProperty = child.m_wrappedProperty;

        switch (childProperty.getType())
        {
        case EPropertyType::Float:
            return sol::make_object(solState, childProperty.getValueAs<float>());
        case EPropertyType::Int32:
            return sol::make_object(solState, childProperty.getValueAs<int32_t>());
        case EPropertyType::Int64:
            return sol::make_object(solState, childProperty.getValueAs<int64_t>());
        case EPropertyType::String:
            return sol::make_object(solState, childProperty.getValueAs<std::string>());
        case EPropertyType::Bool:
            return sol::make_object(solState, childProperty.getValueAs<bool>());
        case EPropertyType::Vec2f:
        case EPropertyType::Vec3f:
        case EPropertyType::Vec4f:
        case EPropertyType::Vec2i:
        case EPropertyType::Vec3i:
        case EPropertyType::Vec4i:
        case EPropertyType::Array:
        case EPropertyType::Struct:
            return sol::make_object(solState, std::ref(child));
        }

        assert(false && "Missing type implementation!");
        return sol::lua_nil;
    }

    sol::object WrappedLuaProperty::resolveVectorElement(sol::this_state solState, size_t elementIndex) const
    {
        const EPropertyType vecType = m_wrappedProperty.get().getType();
        assert(TypeUtils::IsPrimitiveVectorType(vecType));
        assert(elementIndex > 0 && elementIndex <= TypeUtils::ComponentsSizeForPropertyType(vecType));

        switch (vecType)
        {
        case EPropertyType::Vec2f:
            return sol::make_object(solState, m_wrappedProperty.get().getValueAs<PropertyEnumToType<EPropertyType::Vec2f>::TYPE>()[elementIndex - 1]);
        case EPropertyType::Vec3f:
            return sol::make_object(solState, m_wrappedProperty.get().getValueAs<PropertyEnumToType<EPropertyType::Vec3f>::TYPE>()[elementIndex - 1]);
        case EPropertyType::Vec4f:
            return sol::make_object(solState, m_wrappedProperty.get().getValueAs<PropertyEnumToType<EPropertyType::Vec4f>::TYPE>()[elementIndex - 1]);
        case EPropertyType::Vec2i:
            return sol::make_object(solState, m_wrappedProperty.get().getValueAs<PropertyEnumToType<EPropertyType::Vec2i>::TYPE>()[elementIndex - 1]);
        case EPropertyType::Vec3i:
            return sol::make_object(solState, m_wrappedProperty.get().getValueAs<PropertyEnumToType<EPropertyType::Vec3i>::TYPE>()[elementIndex - 1]);
        case EPropertyType::Vec4i:
            return sol::make_object(solState, m_wrappedProperty.get().getValueAs<PropertyEnumToType<EPropertyType::Vec4i>::TYPE>()[elementIndex - 1]);
        default:
            assert(false && "unexpected");
            return sol::lua_nil;
        }
    }

    void WrappedLuaProperty::newIndex(const sol::object& index, const sol::object& rhs)
    {
        if (TypeUtils::IsPrimitiveVectorType(m_wrappedProperty.get().getType()))
        {
            sol_helper::throwSolException("Error while writing to '{}'. Can't assign individual components of vector types, must assign the whole vector!", m_wrappedProperty.get().getName());
        }

        const size_t childIndex = resolvePropertyIndex(index);

        if (m_wrappedProperty.get().getPropertySemantics() != EPropertySemantics::ScriptOutput)
        {
            sol_helper::throwSolException("Error while writing to '{}'. Writing input values is not allowed, only outputs!", getChildDebugName(childIndex));
        }

        setChildValue(childIndex, rhs);
    }

    void WrappedLuaProperty::setChildValue(size_t index, const sol::object& rhs)
    {
        WrappedLuaProperty& childProperty = m_wrappedChildProperties[index];

        if (rhs.get_type() == sol::type::userdata)
        {
            if (!rhs.is<WrappedLuaProperty>())
            {
                // If we ever add other user data objects, should modify this block
                // For now, we check the type explicitly before converting for a better user message
                sol_helper::throwSolException("Implementation error: Unexpected userdata");
            }

            childProperty.setComplex(rhs.as<WrappedLuaProperty>());
        }
        else
        {
            switch (childProperty.m_wrappedProperty.get().getType())
            {
            case EPropertyType::Array:
                childProperty.setArray(rhs);
                break;
            case EPropertyType::Struct:
                childProperty.setStruct(rhs);
                break;
            case EPropertyType::Vec2f:
                childProperty.setVectorComponents<float, 2>(rhs);
                break;
            case EPropertyType::Vec3f:
                childProperty.setVectorComponents<float, 3>(rhs);
                break;
            case EPropertyType::Vec4f:
                childProperty.setVectorComponents<float, 4>(rhs);
                break;
            case EPropertyType::Vec2i:
                childProperty.setVectorComponents<int32_t, 2>(rhs);
                break;
            case EPropertyType::Vec3i:
                childProperty.setVectorComponents<int32_t, 3>(rhs);
                break;
            case EPropertyType::Vec4i:
                childProperty.setVectorComponents<int32_t, 4>(rhs);
                break;
            case EPropertyType::String:
                childProperty.setString(rhs);
                break;
            case EPropertyType::Bool:
                childProperty.setBool(rhs);
                break;
            case EPropertyType::Float:
                childProperty.setFloat(rhs);
                break;
            case EPropertyType::Int32:
                childProperty.setInt32(rhs);
                break;
            case EPropertyType::Int64:
                childProperty.setInt64(rhs);
                break;
            default:
                assert(false && "Missing implementation");
            }
        }
    }

    size_t WrappedLuaProperty::resolvePropertyIndex(const sol::object& propertyIndex) const
    {
        const EPropertyType propertyType = m_wrappedProperty.get().getType();
        if (propertyType == EPropertyType::Struct)
        {
            const DataOrError<std::string_view> structFieldName = LuaTypeConversions::ExtractSpecificType<std::string_view>(propertyIndex);

            if (structFieldName.hasError())
            {
                sol_helper::throwSolException("Bad access to property '{}'! {}", m_wrappedProperty.get().getName(), structFieldName.getError());
            }

            for (size_t i = 0; i < m_wrappedChildProperties.size(); ++i)
            {
                if (m_wrappedChildProperties[i].m_wrappedProperty.get().getName() == structFieldName.getData())
                {
                    return i;
                }
            }

            throw BadStructAccess(std::string(structFieldName.getData()), fmt::format("Tried to access undefined struct property '{}'", structFieldName.getData()));
        }

        if (propertyType == EPropertyType::Array)
        {
            const DataOrError<size_t> maybeUInt = LuaTypeConversions::ExtractSpecificType<size_t>(propertyIndex);
            if (maybeUInt.hasError())
            {
                sol_helper::throwSolException("Bad access to property '{}'! {}", m_wrappedProperty.get().getName(), maybeUInt.getError());
            }
            const size_t childCount = m_wrappedChildProperties.size();
            const size_t indexAsUInt = maybeUInt.getData();
            if (indexAsUInt == 0 || indexAsUInt > childCount)
            {
                sol_helper::throwSolException("Index out of range! Expected 0 < index <= {} but received index == {}", childCount, indexAsUInt);
            }
            return indexAsUInt - 1;
        }

        if (TypeUtils::IsPrimitiveVectorType(propertyType))
        {
            const DataOrError<size_t> elementIdx = LuaTypeConversions::ExtractSpecificType<size_t>(propertyIndex);
            if (elementIdx.hasError())
                sol_helper::throwSolException("Bad access to property '{}'! {}", m_wrappedProperty.get().getName(), elementIdx.getError());

            const size_t numElements = TypeUtils::ComponentsSizeForPropertyType(propertyType);
            const size_t indexAsUInt = elementIdx.getData();
            if (indexAsUInt == 0 || indexAsUInt > numElements)
                sol_helper::throwSolException("Index out of range! Expected 0 < index <= {} but received index == {}", numElements, indexAsUInt);

            return indexAsUInt;
        }

        sol_helper::throwSolException("Implementation error");
        return 0;
    }

    void WrappedLuaProperty::setComplex(const WrappedLuaProperty& other)
    {
        verifyTypeCompatibility(other);

        if (TypeUtils::IsPrimitiveType(m_wrappedProperty.get().getType()))
        {
            m_wrappedProperty.get().setValue(other.m_wrappedProperty.get().getValue());
        }
        else
        {
            for (size_t i = 0; i < m_wrappedChildProperties.size(); ++i)
            {
                m_wrappedChildProperties[i].setComplex(other.m_wrappedChildProperties[i]);
            }
        }
    }

    template<typename T, int N>
    sol::object WrappedLuaProperty::extractVectorComponent(sol::this_state solState, const sol::object& index) const
    {
        assert(TypeUtils::IsPrimitiveVectorType(m_wrappedProperty.get().getType()));

        const DataOrError<size_t> potentiallyIndex = LuaTypeConversions::ExtractSpecificType<size_t>(index);
        if (potentiallyIndex.hasError())
        {
            sol_helper::throwSolException("Only non-negative integers supported as array index type! {}", potentiallyIndex.getError());
        }
        const size_t indexAsInt = potentiallyIndex.getData();
        if (indexAsInt == 0 || indexAsInt > N)
        {
            sol_helper::throwSolException("Bad index '{}', expected 1 <= i <= {}!", indexAsInt, N);
        }

        return sol::make_object(solState, m_wrappedProperty.get().getValueAs<std::array<T, N>>()[indexAsInt - 1]);
    }

    template<typename T, int N>
    void WrappedLuaProperty::setVectorComponents(const sol::object& rhs)
    {
        const DataOrError potentialArrayData = LuaTypeConversions::ExtractArray<T, N>(rhs);

        if (potentialArrayData.hasError())
        {
            sol_helper::throwSolException("Error while assigning output Vec{} property '{}'. {}",
                N, m_wrappedProperty.get().getName(),
                potentialArrayData.getError());
        }

        m_wrappedProperty.get().setValue(potentialArrayData.getData());
    }

    // Overrides the '#' operator in Lua (sol3 template substitution)
    size_t WrappedLuaProperty::size() const
    {
        switch (m_wrappedProperty.get().getType())
        {
        case EPropertyType::Array:
        case EPropertyType::Struct:
            return m_wrappedChildProperties.size();
        case EPropertyType::Vec2f:
        case EPropertyType::Vec2i:
            return 2u;
        case EPropertyType::Vec3f:
        case EPropertyType::Vec3i:
            return 3u;
        case EPropertyType::Vec4f:
        case EPropertyType::Vec4i:
            return 4u;
        // This is unreachable code (Lua handles size of primitive types)
        case EPropertyType::Float:
        case EPropertyType::Int32:
        case EPropertyType::Int64:
        case EPropertyType::Bool:
        case EPropertyType::String:
            break;
        }

        assert(false && "Unreachable code!");
        return 0u;
    }

    void WrappedLuaProperty::badTypeAssignment(const sol::type rhsType)
    {
        sol_helper::throwSolException("Assigning {} to '{}' output '{}'!",
            sol_helper::GetSolTypeName(rhsType),
            GetLuaPrimitiveTypeName(m_wrappedProperty.get().getType()),
            m_wrappedProperty.get().getName());
    }

    void WrappedLuaProperty::setInt32(const sol::object& rhs)
    {
        if (rhs.get_type() != sol::type::number)
        {
            badTypeAssignment(rhs.get_type());
        }

        const DataOrError<int32_t> potentiallyInt32 = LuaTypeConversions::ExtractSpecificType<int32_t>(rhs);
        if (potentiallyInt32.hasError())
        {
            sol_helper::throwSolException("Error during assignment of property '{}'! {}",
                m_wrappedProperty.get().getName(),
                potentiallyInt32.getError());
        }
        m_wrappedProperty.get().setValue(potentiallyInt32.getData());
    }

    void WrappedLuaProperty::setInt64(const sol::object& rhs)
    {
        if (rhs.get_type() != sol::type::number)
        {
            badTypeAssignment(rhs.get_type());
        }

        const DataOrError<int64_t> potentiallyInt64 = LuaTypeConversions::ExtractSpecificType<int64_t>(rhs);
        if (potentiallyInt64.hasError())
        {
            sol_helper::throwSolException("Error during assignment of property '{}'! {}",
                m_wrappedProperty.get().getName(),
                potentiallyInt64.getError());
        }
        m_wrappedProperty.get().setValue(potentiallyInt64.getData());
    }

    void WrappedLuaProperty::setFloat(const sol::object& rhs)
    {
        if (rhs.get_type() != sol::type::number)
        {
            badTypeAssignment(rhs.get_type());
        }

        const DataOrError<float> potentiallyFloat = LuaTypeConversions::ExtractSpecificType<float>(rhs);
        if (potentiallyFloat.hasError())
        {
            sol_helper::throwSolException("Error during assignment of property '{}'! {}",
                m_wrappedProperty.get().getName(),
                potentiallyFloat.getError());
        }
        m_wrappedProperty.get().setValue(potentiallyFloat.getData());
    }

    void WrappedLuaProperty::setString(const sol::object& rhs)
    {
        if (!rhs.is<std::string>())
        {
            badTypeAssignment(rhs.get_type());
        }

        m_wrappedProperty.get().setValue(rhs.as<std::string>());
    }

    void WrappedLuaProperty::setBool(const sol::object& rhs)
    {
        if (!rhs.is<bool>())
        {
            badTypeAssignment(rhs.get_type());
        }

        m_wrappedProperty.get().setValue(rhs.as<bool>());
    }

    void WrappedLuaProperty::setStruct(const sol::object& rhs)
    {
        const std::optional<sol::lua_table> potentialLuaTable = LuaTypeConversions::ExtractLuaTable(rhs);
        if (!potentialLuaTable)
        {
            sol_helper::throwSolException("Unexpected type ({}) while assigning value of struct field '{}' (expected a table or another struct)!",
                sol_helper::GetSolTypeName(rhs.get_type()),
                m_wrappedProperty.get().getName());
        }

        const sol::lua_table& solTable = *potentialLuaTable;

        const size_t expectedTableEntries = m_wrappedChildProperties.size();

        // Collect values first before applying to avoid modify-on-iteration (causes stack overflows)
        std::vector<sol::object> childValuesOrderedByIndex(expectedTableEntries);

        size_t actualTableEntries = 0u;
        for (const auto& tableEntry : solTable)
        {
            size_t childIndex = 0;
            try {
                childIndex = resolvePropertyIndex(tableEntry.first);
            }
            catch(const BadStructAccess& badAccess){
                sol_helper::throwSolException("Unexpected property '{}' while assigning values to struct '{}'!", badAccess.fieldName, m_wrappedProperty.get().getName());
            }

            // Sanity check, not possible have two properties resolve to the same field index
            assert (childValuesOrderedByIndex[childIndex] == sol::nil);
            childValuesOrderedByIndex[childIndex] = tableEntry.second;
            ++actualTableEntries;

            if (actualTableEntries > expectedTableEntries)
            {
                sol_helper::throwSolException("Element size mismatch when assigning struct property '{}'! Expected: {} entries, received more",
                    m_wrappedProperty.get().getName(),
                    expectedTableEntries);
            }
        }

        for (size_t i = 0; i < childValuesOrderedByIndex.size(); ++i)
        {
            if (childValuesOrderedByIndex[i] == sol::nil)
            {
                sol_helper::throwSolException("Error while assigning struct '{}', expected a value for property '{}' but found none!",
                    m_wrappedProperty.get().getName(),
                    m_wrappedChildProperties[i].getWrappedProperty().getName()
                );
            }

            setChildValue(i, childValuesOrderedByIndex[i]);
        }
    }

    void WrappedLuaProperty::setArray(const sol::object& rhs)
    {
        if (!rhs.is<sol::lua_table>())
        {
            sol_helper::throwSolException("Unexpected type ({}) while assigning value of array field '{}' (expected a table or another array)!",
                sol_helper::GetSolTypeName(rhs.get_type()),
                m_wrappedProperty.get().getName());
        }

        const sol::lua_table& table = rhs.as<sol::lua_table>();

        for (size_t i = 1u; i <= m_wrappedChildProperties.size(); ++i)
        {
            const sol::object& field = table[i];

            if (field == sol::nil)
            {
                sol_helper::throwSolException("Error during assignment of array property '{}'! Expected a value at index {}",
                    m_wrappedProperty.get().getName(), i);
            }

            // Convert to C+style index by subtracting 1
            setChildValue(i-1, field);
        }

        // According to Lua semantics, table size is N iff table[N+1] is nil -> this check mimics that semantics
        const sol::object potentiallySuperfluousField = table[m_wrappedChildProperties.size()+1];
        if (potentiallySuperfluousField != sol::nil)
        {
            sol_helper::throwSolException("Element size mismatch when assigning array property '{}'! Expected array size: {}",
                m_wrappedProperty.get().getName(),
                m_wrappedChildProperties.size());
        }

    }

    void WrappedLuaProperty::RegisterTypes(sol::state& state)
    {
        state.new_usertype<WrappedLuaProperty>("WrappedLuaProperty",
            sol::meta_method::new_index, &WrappedLuaProperty::newIndex,
            sol::meta_method::index, &WrappedLuaProperty::index);
    }

    std::string WrappedLuaProperty::getChildDebugName(size_t childIndex) const
    {
        if (m_wrappedProperty.get().getType() == EPropertyType::Struct)
        {
            return std::string(m_wrappedChildProperties[childIndex].m_wrappedProperty.get().getName());
        }

        // Convert to Lua-style index (+1)
        return fmt::format("idx: {}", childIndex + 1);
    }

    void WrappedLuaProperty::verifyTypeCompatibility(const WrappedLuaProperty& other) const
    {
        const EPropertyType myType = m_wrappedProperty.get().getType() ;
        if (myType != other.m_wrappedProperty.get().getType())
        {
            sol_helper::throwSolException("Can't assign property '{}' (type {}) to property '{}' (type {})!",
                other.m_wrappedProperty.get().getName(),
                GetLuaPrimitiveTypeName(other.m_wrappedProperty.get().getType()),
                m_wrappedProperty.get().getName(),
                GetLuaPrimitiveTypeName(myType));
        }

        if (m_wrappedChildProperties.size() != other.m_wrappedChildProperties.size())
        {
            sol_helper::throwSolException("Can't assign property '{}' (#fields={}) to property '{}' (#fields={})!",
                other.m_wrappedProperty.get().getName(),
                other.m_wrappedChildProperties.size(),
                m_wrappedProperty.get().getName(),
                m_wrappedChildProperties.size());
        }

        // Verify struct fields recursively
        if (myType == EPropertyType::Struct)
        {
            for (size_t i = 0; i < m_wrappedChildProperties.size(); ++i)
            {
                m_wrappedChildProperties[i].verifyTypeCompatibility(other.m_wrappedChildProperties[i]);
            }
        }

        // Verify first array element, assuming arrays are homogeneous (ensured during creation)
        if (myType == EPropertyType::Array)
        {
            m_wrappedChildProperties[0].verifyTypeCompatibility(other.m_wrappedChildProperties[0]);
        }
    }

    const PropertyImpl& WrappedLuaProperty::getWrappedProperty() const
    {
        return m_wrappedProperty.get();
    }
}
