//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/RenderBufferBindingImpl.h"
#include "impl/logic/PropertyImpl.h"
#include "impl/ErrorReporting.h"
#include "impl/RenderBufferImpl.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/ramses-utils.h"
#include "internal/logic/RamsesObjectResolver.h"
#include "internal/logic/flatbuffers/generated/RenderBufferBindingGen.h"
#include "fmt/format.h"

namespace ramses::internal
{
    RenderBufferBindingImpl::RenderBufferBindingImpl(SceneImpl& scene, ramses::RenderBuffer& renderBuffer, std::string_view name, sceneObjectId_t id)
        : RamsesBindingImpl{ scene, name, id, renderBuffer }
        , m_renderBuffer{ renderBuffer }
    {
    }

    void RenderBufferBindingImpl::createRootProperties()
    {
        HierarchicalTypeData inputsType = MakeStruct("", {
                TypeData{"width", EPropertyType::Int32},          //EInputProperty::Width
                TypeData{"height", EPropertyType::Int32},         //EInputProperty::Height
                TypeData{"sampleCount", EPropertyType::Int32},    //EInputProperty::SampleCount
            });
        auto inputs = std::make_unique<PropertyImpl>(std::move(inputsType), EPropertySemantics::BindingInput);

        setRootInputs(std::move(inputs));

        ApplyValuesToInputProperties(*this, m_renderBuffer);
    }

    flatbuffers::Offset<rlogic_serialization::RenderBufferBinding> RenderBufferBindingImpl::Serialize(
        const RenderBufferBindingImpl& renderBufferBinding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        const auto logicObject = LogicObjectImpl::Serialize(renderBufferBinding, builder);
        const auto fbRamsesRef = RamsesBindingImpl::SerializeRamsesReference(renderBufferBinding.m_renderBuffer, builder);
        const auto propertyObject = PropertyImpl::Serialize(renderBufferBinding.getInputs()->impl(), builder, serializationMap);
        auto fbRamsesBinding = rlogic_serialization::CreateRamsesBinding(builder,
            logicObject,
            fbRamsesRef,
            propertyObject);

        auto fbRenderBufferBinding = rlogic_serialization::CreateRenderBufferBinding(builder, fbRamsesBinding);
        builder.Finish(fbRenderBufferBinding);

        return fbRenderBufferBinding;
    }

    std::unique_ptr<RenderBufferBindingImpl> RenderBufferBindingImpl::Deserialize(
        const rlogic_serialization::RenderBufferBinding& renderBufferBinding,
        const IRamsesObjectResolver& ramsesResolver,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        if (!renderBufferBinding.base())
        {
            errorReporting.set("Fatal error during loading of RenderBufferBinding from serialized data: missing base class info!", nullptr);
            return nullptr;
        }

        std::string name;
        sceneObjectId_t id{};
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(renderBufferBinding.base()->base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.set("Fatal error during loading of RenderBufferBinding from serialized data: missing name and/or ID!", nullptr);
            return nullptr;
        }

        if (!renderBufferBinding.base()->rootInput())
        {
            errorReporting.set("Fatal error during loading of RenderBufferBinding from serialized data: missing root input!", nullptr);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootInput = PropertyImpl::Deserialize(*renderBufferBinding.base()->rootInput(), EPropertySemantics::BindingInput, errorReporting, deserializationMap);
        if (!deserializedRootInput)
            return nullptr;

        if (deserializedRootInput->getType() != EPropertyType::Struct ||
            deserializedRootInput->getChildCount() != size_t(EInputProperty::COUNT) ||
            deserializedRootInput->getChild(size_t(EInputProperty::Width))->getName() != "width" ||
            deserializedRootInput->getChild(size_t(EInputProperty::Height))->getName() != "height" ||
            deserializedRootInput->getChild(size_t(EInputProperty::SampleCount))->getName() != "sampleCount")
        {
            errorReporting.set("Fatal error during loading of RenderBufferBinding from serialized data: corrupted root input!", nullptr);
            return nullptr;
        }

        const auto* boundObject = renderBufferBinding.base()->boundRamsesObject();
        if (!boundObject)
        {
            errorReporting.set("Fatal error during loading of RenderBufferBinding from serialized data: missing ramses object reference!", nullptr);
            return nullptr;
        }

        const ramses::sceneObjectId_t objectId{ boundObject->objectId() };
        ramses::SceneObject* ramsesObject = ramsesResolver.findRamsesSceneObjectInScene(name, objectId);
        if (!ramsesObject)
            return nullptr;

        if (ramsesObject->getType() != ramses::ERamsesObjectType::RenderBuffer || ramsesObject->getType() != static_cast<ramses::ERamsesObjectType>(boundObject->objectType()))
        {
            errorReporting.set("Fatal error during loading of RenderBufferBinding from serialized data: loaded object type does not match referenced object type!", nullptr);
            return nullptr;
        }

        auto* ramsesRenderBuffer = ramsesObject->as<RenderBuffer>();
        assert(ramsesRenderBuffer);
        auto binding = std::make_unique<RenderBufferBindingImpl>(deserializationMap.getScene(), *ramsesRenderBuffer, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs(std::move(deserializedRootInput));

        ApplyValuesToInputProperties(*binding, *ramsesRenderBuffer);

        return binding;
    }

    std::optional<LogicNodeRuntimeError> RenderBufferBindingImpl::update()
    {
        static constexpr auto PropertyCount = static_cast<size_t>(EInputProperty::COUNT);
        std::array<PropertyImpl*, PropertyCount> props;
        std::array<bool, PropertyCount> propChanges;
        for (size_t i = 0; i < PropertyCount; ++i)
        {
            props[i] = &getInputs()->getChild(i)->impl();
            propChanges[i] = props[i]->checkForBindingInputNewValueAndReset();
        }
        // early out if no change
        if (std::none_of(propChanges.cbegin(), propChanges.cend(), [](bool x) { return x; }))
            return std::nullopt;

        std::array<uint32_t, PropertyCount> propValues;
        for (size_t i = 0; i < PropertyCount; ++i)
        {
            const auto propValue = props[i]->getValueAs<int32_t>();
            if (propValue < 0)
                return LogicNodeRuntimeError{ "RenderBufferBinding input cannot be negative" };
            propValues[i] = static_cast<uint32_t>(propValue);
        }

        const std::array<uint32_t, PropertyCount> currValues{ m_renderBuffer.get().getWidth(), m_renderBuffer.get().getHeight(), m_renderBuffer.get().getSampleCount() };
        if (propValues != currValues)
        {
            if (!m_renderBuffer.get().impl().setProperties(
                propValues[size_t(EInputProperty::Width)],
                propValues[size_t(EInputProperty::Height)],
                propValues[size_t(EInputProperty::SampleCount)]))
            {
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
            }
        }

        return std::nullopt;
    }

    const ramses::RenderBuffer& RenderBufferBindingImpl::getRenderBuffer() const
    {
        return m_renderBuffer;
    }

    ramses::RenderBuffer& RenderBufferBindingImpl::getRenderBuffer()
    {
        return m_renderBuffer;
    }

    // Overwrites binding value cache silently (without triggering dirty check) - this code is only executed at creation or deserialization,
    // should not overwrite values unless set() or link explicitly called
    void RenderBufferBindingImpl::ApplyValuesToInputProperties(RenderBufferBindingImpl& binding, ramses::RenderBuffer& renderBuffer)
    {
        binding.getInputs()->getChild(size_t(EInputProperty::Width))->impl().initializeBindingInputValue(static_cast<int32_t>(renderBuffer.getWidth()));
        binding.getInputs()->getChild(size_t(EInputProperty::Height))->impl().initializeBindingInputValue(static_cast<int32_t>(renderBuffer.getHeight()));
        binding.getInputs()->getChild(size_t(EInputProperty::SampleCount))->impl().initializeBindingInputValue(static_cast<int32_t>(renderBuffer.getSampleCount()));
    }
}
