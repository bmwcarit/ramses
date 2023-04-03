//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internals/SolWrapper.h"

#include "impl/PropertyImpl.h"
#include "internals/SolState.h"

namespace rlogic
{
    class Property;
}

namespace rlogic::internal
{
    // This class provides a Lua-like interface to the logic engine types
    class WrappedLuaProperty
    {
    public:
        explicit WrappedLuaProperty(PropertyImpl& propertyToWrap);
        ~WrappedLuaProperty() = default;

        // Non-copyable, move-able
        WrappedLuaProperty(WrappedLuaProperty && other) noexcept = default;
        WrappedLuaProperty& operator=(WrappedLuaProperty && other) noexcept = default;
        WrappedLuaProperty(const WrappedLuaProperty& other) = delete;
        WrappedLuaProperty& operator=(const WrappedLuaProperty& other) = delete;

        // Interface metamethods used by Lua
        // Called on 'obj.index = rhs'
        void        newIndex(const sol::object& index, const sol::object& rhs);
        // Called on 'X = obj.index'
        [[nodiscard]] sol::object index(sol::this_state solState, const sol::object& index) const;
        // Called on '#obj'
        [[nodiscard]] size_t size() const;
        [[nodiscard]] sol::object resolveChild(sol::this_state solState, size_t childIndex) const;
        [[nodiscard]] sol::object resolveVectorElement(sol::this_state solState, size_t elementIndex) const;
        [[nodiscard]] size_t resolvePropertyIndex(const sol::object& propertyIndex) const;

        [[nodiscard]] const PropertyImpl& getWrappedProperty() const;

        // Register symbols for type extraction to sol state globally
        static void RegisterTypes(sol::state& state);

    private:
        std::reference_wrapper<PropertyImpl> m_wrappedProperty;
        std::vector<WrappedLuaProperty> m_wrappedChildProperties;

        template <typename T, int N>
        [[nodiscard]] sol::object extractVectorComponent(sol::this_state solState, const sol::object& index) const;
        template <typename T, int N>
        void setVectorComponents(const sol::object& rhs);

        void setChildValue(size_t index, const sol::object& rhs);
        void setComplex(const WrappedLuaProperty& other);

        void setStruct(const sol::object& rhs);
        void setArray(const sol::object& rhs);
        void setInt32(const sol::object& rhs);
        void setInt64(const sol::object& rhs);
        void setFloat(const sol::object& rhs);
        void setString(const sol::object& rhs);
        void setBool(const sol::object& rhs);

        [[nodiscard]] std::string getChildDebugName(size_t childIndex) const;
        void verifyTypeCompatibility(const WrappedLuaProperty& other) const;
        void badTypeAssignment(const sol::type rhsType);
    };
}
