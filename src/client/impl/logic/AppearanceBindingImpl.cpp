//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/AppearanceBindingImpl.h"

#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"
#include "ramses/client/UniformInput.h"

#include "ramses/client/logic/EPropertyType.h"
#include "ramses/client/logic/Property.h"

#include "impl/logic/PropertyImpl.h"

#include "internal/logic/RamsesHelper.h"
#include "impl/ErrorReporting.h"
#include "internal/logic/TypeUtils.h"
#include "internal/logic/RamsesObjectResolver.h"

#include "internal/logic/flatbuffers/generated/AppearanceBindingGen.h"

namespace ramses::internal
{
    AppearanceBindingImpl::AppearanceBindingImpl(SceneImpl& scene, ramses::Appearance& ramsesAppearance, std::string_view name, sceneObjectId_t id)
        : RamsesBindingImpl{ scene, name, id, ramsesAppearance }
        , m_ramsesAppearance(ramsesAppearance)
    {
        const auto& effect = m_ramsesAppearance.get().getEffect();
        const size_t uniformCount = effect.getUniformInputCount();
        m_uniforms.reserve(uniformCount);

        // store uniforms that will be exposed as properties, these must match properties (either created or deserialized)
        for (size_t i = 0; i < uniformCount; ++i)
        {
            std::optional<ramses::UniformInput> uniformInput = effect.getUniformInput(i);
            assert(uniformInput.has_value());
            if (GetPropertyTypeForUniform(*uniformInput))
                m_uniforms.push_back(std::move(*uniformInput));
        }
    }

    void AppearanceBindingImpl::createRootProperties()
    {
        std::vector<HierarchicalTypeData> bindingInputs;
        bindingInputs.reserve(m_uniforms.size());
        for (const auto& uniformInput : m_uniforms)
        {
            const std::optional<EPropertyType> convertedType = GetPropertyTypeForUniform(uniformInput);
            if (convertedType)
            {
                // Non-array case
                if (uniformInput.getElementCount() == 1)
                {
                    bindingInputs.emplace_back(MakeType(uniformInput.getName(), *convertedType));
                }
                // Array case
                else
                {
                    bindingInputs.emplace_back(MakeArray(uniformInput.getName(), uniformInput.getElementCount(), *convertedType));
                }
            }
        }
        assert(bindingInputs.size() == m_uniforms.size());

        HierarchicalTypeData bindingInputsType(TypeData{ "", EPropertyType::Struct }, bindingInputs);

        setRootInputs(std::make_unique<PropertyImpl>(bindingInputsType, EPropertySemantics::BindingInput));
    }

    flatbuffers::Offset<rlogic_serialization::AppearanceBinding> AppearanceBindingImpl::Serialize(
        const AppearanceBindingImpl& binding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        auto ramsesReference = RamsesBindingImpl::SerializeRamsesReference(binding.m_ramsesAppearance, builder);

        const auto logicObject = LogicObjectImpl::Serialize(binding, builder);
        const auto propertyObject = PropertyImpl::Serialize(binding.getInputs()->impl(), builder, serializationMap);
        auto ramsesBinding = rlogic_serialization::CreateRamsesBinding(builder,
            logicObject,
            ramsesReference,
            propertyObject);
        builder.Finish(ramsesBinding);

        rlogic_serialization::ResourceId parentEffectResourceId;
        parentEffectResourceId = rlogic_serialization::ResourceId(binding.m_ramsesAppearance.get().getEffect().getResourceId().lowPart, binding.m_ramsesAppearance.get().getEffect().getResourceId().highPart);

        auto appearanceBinding = rlogic_serialization::CreateAppearanceBinding(builder,
            ramsesBinding,
            &parentEffectResourceId
            );
        builder.Finish(appearanceBinding);

        return appearanceBinding;
    }

    std::unique_ptr<AppearanceBindingImpl> AppearanceBindingImpl::Deserialize(
        const rlogic_serialization::AppearanceBinding& appearanceBinding,
        const IRamsesObjectResolver& ramsesResolver,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        if (!appearanceBinding.base())
        {
            errorReporting.set("Fatal error during loading of AppearanceBinding from serialized data: missing base class info!", nullptr);
            return nullptr;
        }

        std::string name;
        sceneObjectId_t id{};
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(appearanceBinding.base()->base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.set("Fatal error during loading of AppearanceBinding from serialized data: missing name and/or ID!", nullptr);
            return nullptr;
        }

        if (!appearanceBinding.base()->rootInput())
        {
            errorReporting.set("Fatal error during loading of AppearanceBinding from serialized data: missing root input!", nullptr);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootInput = PropertyImpl::Deserialize(*appearanceBinding.base()->rootInput(), EPropertySemantics::BindingInput, errorReporting, deserializationMap);

        if (!deserializedRootInput)
        {
            return nullptr;
        }

        if (deserializedRootInput->getType() != EPropertyType::Struct)
        {
            errorReporting.set("Fatal error during loading of AppearanceBinding from serialized data: root input has unexpected type!", nullptr);
            return nullptr;
        }

        const auto* boundObject = appearanceBinding.base()->boundRamsesObject();
        if (!boundObject)
        {
            errorReporting.set("Fatal error during loading of AppearanceBinding from serialized data: no reference to appearance!", nullptr);
            return nullptr;
        }

        const ramses::sceneObjectId_t objectId(boundObject->objectId());
        ramses::Appearance* resolvedAppearance = ramsesResolver.findRamsesAppearanceInScene(name, objectId);
        if (!resolvedAppearance)
        {
            // TODO Violin improve error reporting for this particular error (it's reported in ramsesResolver currently): provide better message and scene/app ids
            return nullptr;
        }

        const ramses::Effect& effect = resolvedAppearance->getEffect();
        const ramses::resourceId_t effectResourceId = effect.getResourceId();
        if (effectResourceId.lowPart != appearanceBinding.parentEffectId()->resourceIdLow() || effectResourceId.highPart != appearanceBinding.parentEffectId()->resourceIdHigh())
        {
            errorReporting.set("Fatal error during loading of AppearanceBinding from serialized data: effect signature doesn't match after loading!", nullptr);
            return nullptr;
        }

        auto binding = std::make_unique<AppearanceBindingImpl>(deserializationMap.getScene(), *resolvedAppearance, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs(std::move(deserializedRootInput));

        return binding;
    }

    std::optional<LogicNodeRuntimeError> AppearanceBindingImpl::update()
    {
        const size_t childCount = getInputs()->getChildCount();
        for (size_t i = 0; i < childCount; ++i)
        {
            setInputValueToUniform(i);
        }

        return std::nullopt;
    }

    void AppearanceBindingImpl::setInputValueToUniform(size_t inputIndex)
    {
        assert(inputIndex < m_uniforms.size());
        PropertyImpl& inputProperty = getInputs()->getChild(inputIndex)->impl();
        const EPropertyType propertyType = inputProperty.getType();

        if (TypeUtils::IsPrimitiveType(propertyType))
        {
            if (inputProperty.checkForBindingInputNewValueAndReset())
            {
                std::visit([&](auto v) {
                    using ValueType = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
                    if constexpr (ramses::IsUniformInputDataType<ValueType>())
                    {
                        m_ramsesAppearance.get().setInputValue(m_uniforms[inputIndex], ValueType{ std::move(v) });
                    }
                    else
                    {
                        assert(false && "This should never happen");
                    }
                }, inputProperty.getValue());
            }
        }
        else
        {
            assert(propertyType == EPropertyType::Array);

            // A new value on any of the array element causes the whole array to be updated
            // Ramses does not allow partial updates so this is the only option here
            bool anyArrayElementWasSet = false;
            const size_t arraySize = inputProperty.getChildCount();
            for (size_t i = 0; i < arraySize; ++i)
            {
                if (inputProperty.getChild(i)->impl().checkForBindingInputNewValueAndReset())
                    anyArrayElementWasSet = true;
            }

            if (anyArrayElementWasSet)
            {
                std::visit([&](const auto& v) {
                    using ValueType = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
                    if constexpr (ramses::IsUniformInputDataType<ValueType>())
                    {
                        if constexpr (std::is_same_v<ValueType, bool>) // special handling for bool array, cannot use vector<bool>
                        {
                            // NOLINTNEXTLINE(modernize-avoid-c-arrays)
                            auto values = std::make_unique<bool[]>(inputProperty.getChildCount());
                            for (size_t i = 0u; i < inputProperty.getChildCount(); ++i)
                                values[i] = *inputProperty.getChild(i)->get<bool>();

                            m_ramsesAppearance.get().setInputValue(m_uniforms[inputIndex], inputProperty.getChildCount(), values.get());
                        }
                        else
                        {
                            std::vector<ValueType> values;
                            values.reserve(inputProperty.getChildCount());
                            for (size_t i = 0u; i < inputProperty.getChildCount(); ++i)
                                values.push_back(ValueType{ *inputProperty.getChild(i)->get<ValueType>() });

                            m_ramsesAppearance.get().setInputValue(m_uniforms[inputIndex], values.size(), values.data());
                        }
                    }
                    else
                        assert(false && "This should never happen");
                }, inputProperty.getChild(0u)->impl().getValue()); // small trick to determine the element type so that the flattened array can be declared in templated code
            }
        }
    }

    ramses::Appearance& AppearanceBindingImpl::getRamsesAppearance() const
    {
        return m_ramsesAppearance;
    }

    std::optional<EPropertyType> AppearanceBindingImpl::GetPropertyTypeForUniform(const ramses::UniformInput& uniform)
    {
        // Can't bind semantic uniforms
        if (uniform.getSemantics() != ramses::EEffectUniformSemantic::Invalid)
            return std::nullopt;

        return ConvertRamsesUniformTypeToPropertyType(uniform.getDataType());
    }
}
