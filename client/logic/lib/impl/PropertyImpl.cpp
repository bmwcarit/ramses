//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/Property.h"

#include "impl/PropertyImpl.h"
#include "impl/LogicNodeImpl.h"
#include "impl/LoggerImpl.h"

#include "internals/SerializationHelper.h"
#include "internals/TypeUtils.h"
#include "internals/ErrorReporting.h"
#include "internals/TypeUtils.h"

#include "generated/PropertyGen.h"

#include <cassert>
#include <algorithm>

namespace rlogic::internal
{
    PropertyImpl::PropertyImpl(HierarchicalTypeData type, EPropertySemantics semantics)
        : m_typeData(std::move(type.typeData))
        , m_semantics(semantics)
    {
        if (TypeUtils::IsPrimitiveType(m_typeData.type))
        {
            switch (m_typeData.type)
            {
            case EPropertyType::Float:
                m_value = PropertyEnumToType<EPropertyType::Float>::TYPE{ 0.0f };
                break;
            case EPropertyType::Vec2f:
                m_value = PropertyEnumToType<EPropertyType::Vec2f>::TYPE{ 0.0f, 0.0f };
                break;
            case EPropertyType::Vec3f:
                m_value = PropertyEnumToType<EPropertyType::Vec3f>::TYPE{ 0.0f, 0.0f, 0.0f };
                break;
            case EPropertyType::Vec4f:
                m_value = PropertyEnumToType<EPropertyType::Vec4f>::TYPE{ 0.0f, 0.0f, 0.0f, 0.0f };
                break;
            case EPropertyType::Int32:
                m_value = PropertyEnumToType<EPropertyType::Int32>::TYPE{ 0 };
                break;
            case EPropertyType::Int64:
                m_value = PropertyEnumToType<EPropertyType::Int64>::TYPE{ 0 };
                break;
            case EPropertyType::Vec2i:
                m_value = PropertyEnumToType<EPropertyType::Vec2i>::TYPE{ 0, 0 };
                break;
            case EPropertyType::Vec3i:
                m_value = PropertyEnumToType<EPropertyType::Vec3i>::TYPE{ 0, 0, 0 };
                break;
            case EPropertyType::Vec4i:
                m_value = PropertyEnumToType<EPropertyType::Vec4i>::TYPE{ 0, 0, 0, 0 };
                break;
            case EPropertyType::String:
                m_value = PropertyEnumToType<EPropertyType::String>::TYPE{};
                break;
            case EPropertyType::Bool:
                m_value = PropertyEnumToType<EPropertyType::Bool>::TYPE{ false };
                break;
            case EPropertyType::Array:
            case EPropertyType::Struct:
                assert(false);
                break;
            }
        }
        else
        {
            for (const auto& childType : type.children)
            {
                m_children.emplace_back(std::make_unique<Property>(std::make_unique<PropertyImpl>(childType, semantics)));
            }
        }
    }

    PropertyImpl::PropertyImpl(HierarchicalTypeData type, EPropertySemantics semantics, PropertyValue initialValue)
        : PropertyImpl(std::move(type), semantics)
    {
        assert(TypeUtils::IsPrimitiveType(m_typeData.type) && "Don't use this constructor with non-primitive types!");
        m_value = std::move(initialValue);
    }

    PropertyImpl::~PropertyImpl() noexcept
    {
        // TODO Violin/Vaclav discuss if we want to handle this here
        if (m_incomingLink.property != nullptr)
        {
            resetIncomingLink();
        }

        for (auto outgoingLink : m_outgoingLinks)
        {
            assert(outgoingLink.property->m_incomingLink.property == this);
            outgoingLink.property->m_incomingLink = { nullptr, false };
        }
    }

    flatbuffers::Offset<rlogic_serialization::Property> PropertyImpl::Serialize(const PropertyImpl& prop, flatbuffers::FlatBufferBuilder& builder, SerializationMap& serializationMap)
    {
        auto result = SerializeRecursive(prop, builder, serializationMap);
        builder.Finish(result);
        return result;
    }

    flatbuffers::Offset<rlogic_serialization::Property> PropertyImpl::SerializeRecursive(
        const PropertyImpl& prop,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        std::vector<flatbuffers::Offset<rlogic_serialization::Property>> child_vector;
        child_vector.reserve(prop.m_children.size());

        std::transform(prop.m_children.begin(), prop.m_children.end(), std::back_inserter(child_vector), [&builder, &serializationMap](const std::vector<std::unique_ptr<Property>>::value_type& child) {
            return SerializeRecursive(*child->m_impl, builder, serializationMap);
            });

        // Assume primitive property, override only for structs/arrays based on m_typeData.type
        rlogic_serialization::EPropertyRootType propertyRootType = rlogic_serialization::EPropertyRootType::Primitive;
        rlogic_serialization::PropertyValue valueType = rlogic_serialization::PropertyValue::NONE;
        flatbuffers::Offset<void> valueOffset;

        switch (prop.m_typeData.type)
        {
        case EPropertyType::Bool:
        {
            const rlogic_serialization::bool_s bool_struct(prop.getValueAs<bool>());
            valueOffset = builder.CreateStruct(bool_struct).Union();
            valueType = rlogic_serialization::PropertyValueTraits<rlogic_serialization::bool_s>::enum_value;
        }
        break;
        case EPropertyType::Float:
        {
            const rlogic_serialization::float_s float_struct(prop.getValueAs<float>());
            valueOffset = builder.CreateStruct(float_struct).Union();
            valueType = rlogic_serialization::PropertyValueTraits<rlogic_serialization::float_s>::enum_value;
        }
        break;
        case EPropertyType::Vec2f:
        {
            const auto& valueVec2f = prop.getValueAs<vec2f>();
            const rlogic_serialization::vec2f_s vec2f_struct(valueVec2f[0], valueVec2f[1]);
            valueOffset = builder.CreateStruct(vec2f_struct).Union();
            valueType = rlogic_serialization::PropertyValueTraits<rlogic_serialization::vec2f_s>::enum_value;
        }
        break;
        case EPropertyType::Vec3f:
        {
            const auto& valueVec3f = prop.getValueAs<vec3f>();
            const rlogic_serialization::vec3f_s vec3f_struct(valueVec3f[0], valueVec3f[1], valueVec3f[2]);
            valueOffset = builder.CreateStruct(vec3f_struct).Union();
            valueType = rlogic_serialization::PropertyValueTraits<rlogic_serialization::vec3f_s>::enum_value;
        }
        break;
        case EPropertyType::Vec4f:
        {
            const auto& valueVec4f = prop.getValueAs<vec4f>();
            const rlogic_serialization::vec4f_s vec4f_struct(valueVec4f[0], valueVec4f[1], valueVec4f[2], valueVec4f[3]);
            valueOffset = builder.CreateStruct(vec4f_struct).Union();
            valueType = rlogic_serialization::PropertyValueTraits<rlogic_serialization::vec4f_s>::enum_value;
        }
        break;
        case EPropertyType::Int32:
        {
            const rlogic_serialization::int32_s int32_struct(prop.getValueAs<int32_t>());
            valueOffset = builder.CreateStruct(int32_struct).Union();
            valueType = rlogic_serialization::PropertyValueTraits<rlogic_serialization::int32_s>::enum_value;
        }
        break;
        case EPropertyType::Int64:
        {
            const rlogic_serialization::int64_s int64_struct(prop.getValueAs<int64_t>());
            valueOffset = builder.CreateStruct(int64_struct).Union();
            valueType = rlogic_serialization::PropertyValueTraits<rlogic_serialization::int64_s>::enum_value;
        }
        break;
        case EPropertyType::Vec2i:
        {
            const auto& valueVec2i = prop.getValueAs<vec2i>();
            const rlogic_serialization::vec2i_s vec2i_struct(valueVec2i[0], valueVec2i[1]);
            valueOffset = builder.CreateStruct(vec2i_struct).Union();
            valueType = rlogic_serialization::PropertyValueTraits<rlogic_serialization::vec2i_s>::enum_value;
        }
        break;
        case EPropertyType::Vec3i:
        {
            const auto& valueVec3i = prop.getValueAs<vec3i>();
            const rlogic_serialization::vec3i_s vec3i_struct(valueVec3i[0], valueVec3i[1], valueVec3i[2]);
            valueOffset = builder.CreateStruct(vec3i_struct).Union();
            valueType = rlogic_serialization::PropertyValueTraits<rlogic_serialization::vec3i_s>::enum_value;
        }
        break;
        case EPropertyType::Vec4i:
        {
            const auto& valueVec4i = prop.getValueAs<vec4i>();
            const rlogic_serialization::vec4i_s vec4i_struct(valueVec4i[0], valueVec4i[1], valueVec4i[2], valueVec4i[3]);
            valueOffset = builder.CreateStruct(vec4i_struct).Union();
            valueType = rlogic_serialization::PropertyValueTraits<rlogic_serialization::vec4i_s>::enum_value;
        }
        break;
        case EPropertyType::String:
        {
            valueOffset = rlogic_serialization::Createstring_s(builder, builder.CreateString(prop.getValueAs<std::string>())).Union();
            valueType = rlogic_serialization::PropertyValueTraits<rlogic_serialization::string_s>::enum_value;
        }
        break;
        case EPropertyType::Array:
            propertyRootType = rlogic_serialization::EPropertyRootType::Array;
            break;
        case EPropertyType::Struct:
            propertyRootType = rlogic_serialization::EPropertyRootType::Struct;
            break;
        }

        const auto fbName = builder.CreateString(prop.m_typeData.name);
        const auto fbChildrenVec = builder.CreateVector(child_vector);

        auto propertyFB = rlogic_serialization::CreateProperty(builder,
            fbName,
            propertyRootType,
            fbChildrenVec,
            valueType,
            valueOffset
        );

        serializationMap.storePropertyOffset(prop, propertyFB);

        return propertyFB;
    }

    std::unique_ptr<PropertyImpl> PropertyImpl::Deserialize(
        const rlogic_serialization::Property& prop,
        EPropertySemantics semantics,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        // TODO Violin we can make name optional - e.g. array fields don't need a name, no need to serialize empty strings
        if (!prop.name())
        {
            errorReporting.add("Fatal error during loading of Property from serialized data: missing name!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const std::optional convertedType = ConvertSerializationTypeToEPropertyType(prop.rootType(), prop.value_type());

        if (!convertedType)
        {
            errorReporting.add("Fatal error during loading of Property from serialized data: invalid type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> impl(new PropertyImpl(MakeType(std::string(prop.name()->string_view()), *convertedType), semantics));

        // If primitive: set value; otherwise load children
        if (prop.rootType() == rlogic_serialization::EPropertyRootType::Primitive)
        {
            // TODO Violin investigate possibilities to unit-test enum mismatches (and perhaps collapse
            // the if-s in the switch below if possible)
            switch (prop.value_type())
            {
            case rlogic_serialization::PropertyValue::float_s:
                if (!prop.value_as_float_s())
                {
                    errorReporting.add("Fatal error during loading of Property from serialized data: invalid union!", nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }
                impl->m_value = prop.value_as_float_s()->v();
                break;
            case rlogic_serialization::PropertyValue::vec2f_s:
            {
                auto vec2fValue = prop.value_as_vec2f_s();
                if (!vec2fValue)
                {
                    errorReporting.add("Fatal error during loading of Property from serialized data: invalid union!", nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }
                impl->m_value         = vec2f{vec2fValue->x(), vec2fValue->y()};
                break;
            }
            case rlogic_serialization::PropertyValue::vec3f_s:
            {
                auto vec3fValue = prop.value_as_vec3f_s();
                if (!vec3fValue)
                {
                    errorReporting.add("Fatal error during loading of Property from serialized data: invalid union!", nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }
                impl->m_value         = vec3f{vec3fValue->x(), vec3fValue->y(), vec3fValue->z()};
                break;
            }
            case rlogic_serialization::PropertyValue::vec4f_s:
            {
                auto vec4fValue = prop.value_as_vec4f_s();
                if (!vec4fValue)
                {
                    errorReporting.add("Fatal error during loading of Property from serialized data: invalid union!", nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }
                impl->m_value         = vec4f{vec4fValue->x(), vec4fValue->y(), vec4fValue->z(), vec4fValue->w()};
                break;
            }
            case rlogic_serialization::PropertyValue::int32_s:
                if (!prop.value_as_int32_s())
                {
                    errorReporting.add("Fatal error during loading of Property from serialized data: invalid union!", nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }
                impl->m_value = prop.value_as_int32_s()->v();
                break;
            case rlogic_serialization::PropertyValue::int64_s:
                if (!prop.value_as_int64_s())
                {
                    errorReporting.add("Fatal error during loading of Property from serialized data: invalid union!", nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }
                impl->m_value = prop.value_as_int64_s()->v();
                break;
            case rlogic_serialization::PropertyValue::vec2i_s:
            {
                auto vec2iValue = prop.value_as_vec2i_s();
                if (!vec2iValue)
                {
                    errorReporting.add("Fatal error during loading of Property from serialized data: invalid union!", nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }
                impl->m_value         = vec2i{vec2iValue->x(), vec2iValue->y()};
                break;
            }
            case rlogic_serialization::PropertyValue::vec3i_s:
            {
                auto vec3iValue = prop.value_as_vec3i_s();
                if (!vec3iValue)
                {
                    errorReporting.add("Fatal error during loading of Property from serialized data: invalid union!", nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }
                impl->m_value         = vec3i{vec3iValue->x(), vec3iValue->y(), vec3iValue->z()};
                break;
            }
            case rlogic_serialization::PropertyValue::vec4i_s:
            {
                auto vec4iValue = prop.value_as_vec4i_s();
                if (!vec4iValue)
                {
                    errorReporting.add("Fatal error during loading of Property from serialized data: invalid union!", nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }
                impl->m_value         = vec4i{vec4iValue->x(), vec4iValue->y(), vec4iValue->z(), vec4iValue->w()};
                break;
            }
            case rlogic_serialization::PropertyValue::string_s:
                if (!prop.value_as_string_s())
                {
                    errorReporting.add("Fatal error during loading of Property from serialized data: invalid union!", nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }
                impl->m_value = prop.value_as_string_s()->v()->str();
                break;
            case rlogic_serialization::PropertyValue::bool_s:
                if (!prop.value_as_bool_s())
                {
                    errorReporting.add("Fatal error during loading of Property from serialized data: invalid union!", nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }
                impl->m_value = prop.value_as_bool_s()->v();
                break;
            case rlogic_serialization::PropertyValue::NONE:
            default:
                assert(false && "Should never reach this line - invalid types should be handled in ConvertSerializationTypeToEPropertyType above");
                return nullptr;
            }
        }
        else
        {
            // Invalid types are handled above
            assert (prop.rootType() == rlogic_serialization::EPropertyRootType::Struct || prop.rootType() == rlogic_serialization::EPropertyRootType::Array);

            if (!prop.children())
            {
                errorReporting.add("Fatal error during loading of Property from serialized data: complex type has no child type info!", nullptr, EErrorType::BinaryVersionMismatch);
                return nullptr;
            }

            for (const auto* child : *prop.children())
            {
                if (!child)
                {
                    // TODO Violin find ways to unit-test this case
                    errorReporting.add("Fatal error during loading of Property from serialized data: corrupt child data!", nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }

                std::unique_ptr<PropertyImpl> deserializedChild = PropertyImpl::Deserialize(*child, semantics, errorReporting, deserializationMap);

                if (!deserializedChild)
                {
                    return nullptr;
                }

                impl->m_children.emplace_back(std::make_unique<Property>(std::move(deserializedChild)));
            }
        }

        deserializationMap.storePropertyImpl(prop, *impl);

        return impl;
    }

    size_t PropertyImpl::getChildCount() const
    {
        return m_children.size();
    }

    EPropertyType PropertyImpl::getType() const
    {
        return m_typeData.type;
    }

    std::string_view PropertyImpl::getName() const
    {
        return m_typeData.name;
    }

    Property* PropertyImpl::getChild(size_t index)
    {
        if (index < m_children.size())
        {
            return m_children[index].get();
        }

        LOG_ERROR("No child property with index '{}' found in '{}'", index, m_typeData.name);
        return nullptr;
    }

    const Property* PropertyImpl::getChild(size_t index) const
    {
        if (index < m_children.size())
        {
            return m_children[index].get();
        }

        LOG_ERROR("No child property with index '{}' found in '{}'", index, m_typeData.name);
        return nullptr;
    }

    Property* PropertyImpl::getChild(std::string_view name)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) non-const version of getChild cast to its const version to avoid duplicating code
        return const_cast<Property*>((const_cast<const PropertyImpl&>(*this)).getChild(name));
    }

    const Property* PropertyImpl::getChild(std::string_view name) const
    {
        auto it = std::find_if(m_children.begin(), m_children.end(), [&name](const std::vector<std::unique_ptr<Property>>::value_type& property) {
            return property->getName() == name;
        });
        if (it != m_children.end())
        {
            return it->get();
        }
        LOG_ERROR("No child property with name '{}' found in '{}'", name, m_typeData.name);
        return nullptr;
    }

    bool PropertyImpl::hasChild(std::string_view name) const
    {
        return m_children.end() != std::find_if(m_children.begin(), m_children.end(), [&name](const std::vector<std::unique_ptr<Property>>::value_type& property) {
            return property->getName() == name;
            });
    }

    std::vector<const Property*> PropertyImpl::collectLeafChildren() const
    {
        std::vector<const Property*> result;

        std::vector<const Property*> toTraverse;
        toTraverse.push_back(m_propertyInstance);
        while (!toTraverse.empty())
        {
            const auto* current = toTraverse.back();
            toTraverse.pop_back();

            if (TypeUtils::IsPrimitiveType(current->getType()))
            {
                result.push_back(current);
            }
            else
            {
                for (uint32_t i = 0; i < current->getChildCount(); ++i)
                {
                    toTraverse.emplace_back(current->getChild(i));
                }
            }
        }

        return result;
    }

    template <typename T> std::optional<T> PropertyImpl::getValue_PublicApi() const
    {
        if (PropertyTypeToEnum<T>::TYPE == m_typeData.type)
        {
            assert(std::holds_alternative<T>(m_value));
            return std::get<T>(m_value);
        }
        LOG_ERROR("Invalid type '{}' when accessing property '{}', correct type is '{}'",
            GetLuaPrimitiveTypeName(PropertyTypeToEnum<T>::TYPE), m_typeData.name, GetLuaPrimitiveTypeName(m_typeData.type));
        return std::nullopt;
    }

    template std::optional<float>       PropertyImpl::getValue_PublicApi<float>() const;
    template std::optional<vec2f>       PropertyImpl::getValue_PublicApi<vec2f>() const;
    template std::optional<vec3f>       PropertyImpl::getValue_PublicApi<vec3f>() const;
    template std::optional<vec4f>       PropertyImpl::getValue_PublicApi<vec4f>() const;
    template std::optional<int32_t>     PropertyImpl::getValue_PublicApi<int32_t>() const;
    template std::optional<int64_t>     PropertyImpl::getValue_PublicApi<int64_t>() const;
    template std::optional<vec2i>       PropertyImpl::getValue_PublicApi<vec2i>() const;
    template std::optional<vec3i>       PropertyImpl::getValue_PublicApi<vec3i>() const;
    template std::optional<vec4i>       PropertyImpl::getValue_PublicApi<vec4i>() const;
    template std::optional<std::string> PropertyImpl::getValue_PublicApi<std::string>() const;
    template std::optional<bool>        PropertyImpl::getValue_PublicApi<bool>() const;

    bool PropertyImpl::setValue_PublicApi(PropertyValue value)
    {
        if (m_semantics == EPropertySemantics::ScriptOutput)
        {
            LOG_ERROR("Cannot set property '{}' which is an output.", m_typeData.name);
            return false;
        }

        if (m_incomingLink.property != nullptr)
        {
            LOG_ERROR("Property '{}' is currently linked (to property '{}'). Unlink it first before setting its value!", m_typeData.name, m_incomingLink.property->getName());
            return false;
        }

        if (!TypeUtils::IsPrimitiveType(m_typeData.type))
        {
            LOG_ERROR("Property '{}' is not a primitive type, can't set its value directly!", m_typeData.name);
            return false;
        }

        if (value.index() != m_value.index())
        {
            LOG_ERROR("Invalid type when setting property '{}', correct type is '{}'", m_typeData.name, GetLuaPrimitiveTypeName(m_typeData.type));
            return false;
        }

        if (std::holds_alternative<int64_t>(value))
        {
            // Lua uses (by default) double for internal storage of numerical values.
            // IEEE 754 64-bit double can represent higher integers than this (DBL_MAX) but this is the maximum
            // for which double can represent this value and all values below correctly
            static constexpr auto maxIntegerAsDouble = static_cast<int64_t>(1LLU << 53u);
            const auto int64Value = std::get<int64_t>(value);
            if (int64Value > maxIntegerAsDouble || int64Value < -maxIntegerAsDouble)
            {
                LOG_ERROR("Invalid value when setting property '{}', Lua cannot handle full range of 64-bit integer, trying to set '{}' which is out of this range!",
                    m_typeData.name, int64Value);
                return false;
            }
        }

        // Marks corresponding node dirty if value changed
        const bool valueChanged = setValue(std::move(value));
        // TODO Violin possibly remove interface properties from this check, add tests first that interface objects dont need to be set
        // dirty if their inputs were set
        if (valueChanged || m_semantics == EPropertySemantics::AnimationInput || m_semantics == EPropertySemantics::BindingInput || m_semantics == EPropertySemantics::Interface)
        {
            m_logicNode->setDirty(true);
        }

        return true;
    }

    bool PropertyImpl::bindingInputHasNewValue() const
    {
        // TODO Violin can we make this assert the bindings semantics?
        return m_bindingInputHasNewValue;
    }

    bool PropertyImpl::checkForBindingInputNewValueAndReset()
    {
        // TODO Violin can we make this assert the bindings semantics?
        const bool newValue = m_bindingInputHasNewValue;
        m_bindingInputHasNewValue = false;
        return newValue;
    }

    bool PropertyImpl::setValue(PropertyValue value)
    {
        assert(m_value.index() == value.index());
        assert(TypeUtils::IsPrimitiveType(m_typeData.type));

        if (m_semantics == EPropertySemantics::BindingInput)
        {
            m_bindingInputHasNewValue = true;
        }

        const bool valueChanged = (m_value != value);

        m_value = std::move(value);

        return valueChanged;
    }

    void PropertyImpl::setPropertyInstance(Property& property)
    {
        assert(m_propertyInstance == nullptr);
        m_propertyInstance = &property;
    }

    Property& PropertyImpl::getPropertyInstance()
    {
        assert(m_propertyInstance != nullptr);
        return *m_propertyInstance;
    }

    const Property& PropertyImpl::getPropertyInstance() const
    {
        assert(m_propertyInstance != nullptr);
        return *m_propertyInstance;
    }

    void PropertyImpl::setLogicNode(LogicNodeImpl& logicNode)
    {
        assert(m_logicNode == nullptr && "Properties are not transferrable across logic nodes!");
        m_logicNode = &logicNode;
        for (auto& child : m_children)
        {
            child->m_impl->setLogicNode(logicNode);
        }
    }

    LogicNodeImpl& PropertyImpl::getLogicNode()
    {
        assert(m_logicNode != nullptr);
        return *m_logicNode;
    }

    const LogicNodeImpl& PropertyImpl::getLogicNode() const
    {
        assert(m_logicNode != nullptr);
        return *m_logicNode;
    }

    bool PropertyImpl::isInput() const
    {
        return m_semantics == EPropertySemantics::ScriptInput || m_semantics == EPropertySemantics::BindingInput || m_semantics == EPropertySemantics::AnimationInput || m_semantics == EPropertySemantics::Interface;
    }

    bool PropertyImpl::isOutput() const
    {
        return m_semantics == EPropertySemantics::ScriptOutput || m_semantics == EPropertySemantics::AnimationOutput || m_semantics == EPropertySemantics::Interface;
    }

    EPropertySemantics PropertyImpl::getPropertySemantics() const
    {
        return m_semantics;
    }

    const PropertyValue& PropertyImpl::getValue() const
    {
        return m_value;
    }

    bool PropertyImpl::isLinked() const
    {
        return (m_incomingLink.property != nullptr) || !m_outgoingLinks.empty();
    }

    bool PropertyImpl::hasIncomingLink() const
    {
        return (m_incomingLink.property != nullptr);
    }

    bool PropertyImpl::hasOutgoingLink() const
    {
        return !m_outgoingLinks.empty();
    }

    const PropertyImpl::Link& PropertyImpl::getIncomingLink() const
    {
        assert(isInput());
        return m_incomingLink;
    }

    const std::vector<PropertyImpl::Link>& PropertyImpl::getOutgoingLinks() const
    {
        assert(isOutput());
        return m_outgoingLinks;
    }

    void PropertyImpl::setIncomingLink(PropertyImpl& output, bool isWeakLink)
    {
        assert(TypeUtils::IsPrimitiveType(getType()));
        assert(TypeUtils::IsPrimitiveType(output.getType()));
        assert(m_incomingLink.property == nullptr);
        assert(std::find_if(output.m_outgoingLinks.begin(), output.m_outgoingLinks.end(), [this](const auto& p) {
            return p.property == this; }) == output.m_outgoingLinks.end());

        output.m_outgoingLinks.push_back({ this, isWeakLink });
        m_incomingLink = { &output, isWeakLink };
    }

    void PropertyImpl::resetIncomingLink()
    {
        assert(isInput() && m_incomingLink.property != nullptr);
        auto& srcPropertyLinks = m_incomingLink.property->m_outgoingLinks;
        auto linkIter = std::find_if(srcPropertyLinks.begin(), srcPropertyLinks.end(), [this](const auto& p) { return p.property == this; });
        assert(linkIter != srcPropertyLinks.end());
        srcPropertyLinks.erase(linkIter);
        m_incomingLink = { nullptr, false };
    }

    void PropertyImpl::initializeBindingInputValue(PropertyValue value)
    {
        setValue(std::move(value));
        m_bindingInputHasNewValue = false;
    }
}
