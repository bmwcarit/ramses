//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RamsesRenderPassBindingImpl.h"

#include "ramses-client-api/RenderPass.h"

#include "ramses-logic/Property.h"

#include "impl/PropertyImpl.h"

#include "internals/ErrorReporting.h"
#include "internals/RamsesObjectResolver.h"

#include "generated/RamsesRenderPassBindingGen.h"

namespace rlogic::internal
{
    RamsesRenderPassBindingImpl::RamsesRenderPassBindingImpl(ramses::RenderPass& ramsesRenderPass, std::string_view name, uint64_t id)
        : RamsesBindingImpl{ name, id }
        , m_ramsesRenderPass{ ramsesRenderPass }
    {
    }

    void RamsesRenderPassBindingImpl::createRootProperties()
    {
        // Attention! This order is important - it has to match the indices in EPropertyIndex!
        auto inputsType = MakeStruct("", {
                TypeData{"enabled", EPropertyType::Bool},
                TypeData{"renderOrder", EPropertyType::Int32},
                TypeData{"clearColor", EPropertyType::Vec4f},
                TypeData{"renderOnce", EPropertyType::Bool}
            });
        auto inputs = std::make_unique<Property>(std::make_unique<PropertyImpl>(std::move(inputsType), EPropertySemantics::BindingInput));

        setRootInputs(std::move(inputs));

        ApplyRamsesValuesToInputProperties(*this, m_ramsesRenderPass);
    }

    flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding> RamsesRenderPassBindingImpl::Serialize(
        const RamsesRenderPassBindingImpl& renderPassBinding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap,
        EFeatureLevel /*featureLevel*/)
    {
        const auto logicObject = LogicObjectImpl::Serialize(renderPassBinding, builder);
        const auto fbRamsesRef = RamsesBindingImpl::SerializeRamsesReference(renderPassBinding.m_ramsesRenderPass, builder);
        const auto propertyObject = PropertyImpl::Serialize(*renderPassBinding.getInputs()->m_impl, builder, serializationMap);
        auto fbRamsesBinding = rlogic_serialization::CreateRamsesBinding(builder,
            logicObject,
            fbRamsesRef,
            propertyObject);

        auto fbRenderPassBinding = rlogic_serialization::CreateRamsesRenderPassBinding(builder, fbRamsesBinding);
        builder.Finish(fbRenderPassBinding);

        return fbRenderPassBinding;
    }

    std::unique_ptr<RamsesRenderPassBindingImpl> RamsesRenderPassBindingImpl::Deserialize(
        const rlogic_serialization::RamsesRenderPassBinding& renderPassBinding,
        const IRamsesObjectResolver& ramsesResolver,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        if (!renderPassBinding.base())
        {
            errorReporting.add("Fatal error during loading of RamsesRenderPassBinding from serialized data: missing base class info!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::string name;
        uint64_t id = 0u;
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(renderPassBinding.base()->base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.add("Fatal error during loading of RamsesRenderPassBinding from serialized data: missing name and/or ID!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!renderPassBinding.base()->rootInput())
        {
            errorReporting.add("Fatal error during loading of RamsesRenderPassBinding from serialized data: missing root input!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootInput = PropertyImpl::Deserialize(*renderPassBinding.base()->rootInput(), EPropertySemantics::BindingInput, errorReporting, deserializationMap);
        if (!deserializedRootInput)
            return nullptr;

        if (deserializedRootInput->getType() != EPropertyType::Struct)
        {
            errorReporting.add("Fatal error during loading of RamsesRenderPassBinding from serialized data: root input has unexpected type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const auto* boundObject = renderPassBinding.base()->boundRamsesObject();
        if (!boundObject)
        {
            errorReporting.add("Fatal error during loading of RamsesRenderPassBinding from serialized data: missing ramses object reference!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const ramses::sceneObjectId_t objectId{ boundObject->objectId() };
        ramses::RenderPass* ramsesRenderPass = ramsesResolver.findRamsesRenderPassInScene(name, objectId);
        if (!ramsesRenderPass)
            return nullptr;

        if (ramsesRenderPass->getType() != static_cast<ramses::ERamsesObjectType>(boundObject->objectType()))
        {
            errorReporting.add("Fatal error during loading of RamsesRenderPassBinding from serialized data: loaded object type does not match referenced object type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        auto binding = std::make_unique<RamsesRenderPassBindingImpl>(*ramsesRenderPass, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs(std::make_unique<Property>(std::move(deserializedRootInput)));

        ApplyRamsesValuesToInputProperties(*binding, *ramsesRenderPass);

        return binding;
    }

    std::optional<LogicNodeRuntimeError> RamsesRenderPassBindingImpl::update()
    {
        ramses::status_t status = ramses::StatusOK;

        PropertyImpl& enabled = *getInputs()->getChild(EPropertyIndex_Enabled)->m_impl;
        if (enabled.checkForBindingInputNewValueAndReset())
        {
            status = m_ramsesRenderPass.get().setEnabled(enabled.getValueAs<bool>());
            if (status != ramses::StatusOK)
                return LogicNodeRuntimeError{ m_ramsesRenderPass.get().getStatusMessage(status) };
        }

        PropertyImpl& renderOrder = *getInputs()->getChild(EPropertyIndex_RenderOrder)->m_impl;
        if (renderOrder.checkForBindingInputNewValueAndReset())
        {
            status = m_ramsesRenderPass.get().setRenderOrder(renderOrder.getValueAs<int32_t>());
            if (status != ramses::StatusOK)
                return LogicNodeRuntimeError{ m_ramsesRenderPass.get().getStatusMessage(status) };
        }

        PropertyImpl& clearColor = *getInputs()->getChild(EPropertyIndex_ClearColor)->m_impl;
        if (clearColor.checkForBindingInputNewValueAndReset())
        {
            const auto clearColorValue = clearColor.getValueAs<vec4f>();
            status = m_ramsesRenderPass.get().setClearColor(clearColorValue[0], clearColorValue[1], clearColorValue[2], clearColorValue[3]);
            if (status != ramses::StatusOK)
                return LogicNodeRuntimeError{ m_ramsesRenderPass.get().getStatusMessage(status) };
        }

        PropertyImpl& renderOnce = *getInputs()->getChild(EPropertyIndex_RenderOnce)->m_impl;
        if (renderOnce.checkForBindingInputNewValueAndReset())
        {
            status = m_ramsesRenderPass.get().setRenderOnce(renderOnce.getValueAs<bool>());
            if (status != ramses::StatusOK)
                return LogicNodeRuntimeError{ m_ramsesRenderPass.get().getStatusMessage(status) };
        }

        return std::nullopt;
    }

    const ramses::RenderPass& RamsesRenderPassBindingImpl::getRamsesRenderPass() const
    {
        return m_ramsesRenderPass;
    }

    ramses::RenderPass& RamsesRenderPassBindingImpl::getRamsesRenderPass()
    {
        return m_ramsesRenderPass;
    }

    // Overwrites binding value cache silently (without triggering dirty check) - this code is only executed at initialization,
    // should not overwrite values unless set() or link explicitly called
    void RamsesRenderPassBindingImpl::ApplyRamsesValuesToInputProperties(RamsesRenderPassBindingImpl& binding, ramses::RenderPass& ramsesRenderPass)
    {
        binding.getInputs()->getChild(EPropertyIndex_Enabled)->m_impl->initializeBindingInputValue(PropertyValue{ ramsesRenderPass.isEnabled() });

        binding.getInputs()->getChild(EPropertyIndex_RenderOrder)->m_impl->initializeBindingInputValue(PropertyValue{ ramsesRenderPass.getRenderOrder() });

        vec4f clearColor;
        ramsesRenderPass.getClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        binding.getInputs()->getChild(EPropertyIndex_ClearColor)->m_impl->initializeBindingInputValue(PropertyValue{ clearColor });

        binding.getInputs()->getChild(EPropertyIndex_RenderOnce)->m_impl->initializeBindingInputValue(PropertyValue{ ramsesRenderPass.isRenderOnce() });
    }
}
