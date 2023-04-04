//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/EPropertyType.h"
#include "internals/EPropertySemantics.h"
#include "internals/SerializationMap.h"
#include "internals/DeserializationMap.h"
#include "internals/TypeData.h"

#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <variant>
#include <memory>

namespace rlogic
{
    class Property;
}
namespace rlogic_serialization
{
    struct Property;
}

namespace flatbuffers
{
    template<typename T> struct Offset;
    class FlatBufferBuilder;
}

namespace rlogic::internal
{
    class LogicNodeImpl;
    class ErrorReporting;

    using PropertyValue = std::variant<int32_t, int64_t, float, bool, std::string, vec2f, vec3f, vec4f, vec2i, vec3i, vec4i>;
    using PropertyList = std::vector<std::unique_ptr<Property>>;

    class PropertyImpl
    {
    public:
        PropertyImpl(HierarchicalTypeData type, EPropertySemantics semantics);
        PropertyImpl(HierarchicalTypeData type, EPropertySemantics semantics, PropertyValue initialValue);

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::Property> Serialize(
            const PropertyImpl& prop,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<PropertyImpl> Deserialize(
            const rlogic_serialization::Property& prop,
            EPropertySemantics semantics,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);


        // Move-able (noexcept); Not copy-able
        ~PropertyImpl() noexcept;
        PropertyImpl& operator=(PropertyImpl&& other) noexcept = default;
        PropertyImpl(PropertyImpl&& other) noexcept = default;
        PropertyImpl& operator=(const PropertyImpl& other) = delete;
        PropertyImpl(const PropertyImpl& other) = delete;

        [[nodiscard]] size_t getChildCount() const;
        [[nodiscard]] EPropertyType getType() const;
        [[nodiscard]] std::string_view getName() const;

        [[nodiscard]] bool bindingInputHasNewValue() const;
        [[nodiscard]] bool checkForBindingInputNewValueAndReset();

        [[nodiscard]] const Property* getChild(size_t index) const;

        // TODO Violin these 3 methods have redundancy, refactor
        [[nodiscard]] bool isInput() const;
        [[nodiscard]] bool isOutput() const;
        [[nodiscard]] EPropertySemantics getPropertySemantics() const;
        [[nodiscard]] bool isLinked() const;
        [[nodiscard]] bool hasIncomingLink() const;
        [[nodiscard]] bool hasOutgoingLink() const;

        [[nodiscard]] Property* getChild(size_t index);
        [[nodiscard]] Property* getChild(std::string_view name);
        [[nodiscard]] const Property* getChild(std::string_view name) const;
        [[nodiscard]] bool hasChild(std::string_view name) const;

        [[nodiscard]] std::vector<const Property*> collectLeafChildren() const;

        // Public API access - only ever called by user, full error check and logs
        template <typename T>
        [[nodiscard]] std::optional<T> getValue_PublicApi() const;
        [[nodiscard]] bool setValue_PublicApi(PropertyValue value);

        // Generic setter. Can optionally skip dirty-check
        bool setValue(PropertyValue value);
        // Special setter for binding value init
        void initializeBindingInputValue(PropertyValue value);

        // Generic getter for use in other non-template code
        [[nodiscard]] const PropertyValue& getValue() const;
        // std::get wrapper for use in template code
        template <typename T>
        [[nodiscard]] const T& getValueAs() const
        {
            return std::get<T>(m_value);
        }

        void setPropertyInstance(Property& property);
        [[nodiscard]] Property& getPropertyInstance();
        [[nodiscard]] const Property& getPropertyInstance() const;

        void setLogicNode(LogicNodeImpl& logicNode);
        [[nodiscard]] LogicNodeImpl& getLogicNode();
        [[nodiscard]] const LogicNodeImpl& getLogicNode() const;

        // Link handling
        struct Link
        {
            PropertyImpl* property = nullptr;
            bool isWeakLink = false;
        };

        [[nodiscard]] const Link& getIncomingLink() const;
        [[nodiscard]] const std::vector<Link>& getOutgoingLinks() const;

        void setIncomingLink(PropertyImpl& output, bool isWeakLink);
        void resetIncomingLink();

    private:
        TypeData        m_typeData;
        PropertyList    m_children;
        PropertyValue   m_value;

        Link m_incomingLink;
        std::vector<Link> m_outgoingLinks;

        Property* m_propertyInstance = nullptr;
        LogicNodeImpl* m_logicNode = nullptr;

        bool m_bindingInputHasNewValue = false;
        EPropertySemantics m_semantics;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::Property> SerializeRecursive(
            const PropertyImpl& prop,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);
    };
}
