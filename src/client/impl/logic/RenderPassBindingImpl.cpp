//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/RenderPassBindingImpl.h"

#include "ramses/client/RenderPass.h"

#include "ramses/client/logic/Property.h"

#include "impl/logic/PropertyImpl.h"

#include "impl/ErrorReporting.h"
#include "internal/logic/RamsesObjectResolver.h"

#include "internal/logic/flatbuffers/generated/RenderPassBindingGen.h"

namespace ramses::internal
{
    RenderPassBindingImpl::RenderPassBindingImpl(SceneImpl& scene, ramses::RenderPass& ramsesRenderPass, std::string_view name, sceneObjectId_t id)
        : RamsesBindingImpl{ scene, name, id, ramsesRenderPass }
        , m_ramsesRenderPass{ ramsesRenderPass }
    {
    }

    void RenderPassBindingImpl::createRootProperties()
    {
        // Attention! This order is important - it has to match the indices in EPropertyIndex!
        auto inputsType = MakeStruct("", {
                TypeData{"enabled", EPropertyType::Bool},
                TypeData{"renderOrder", EPropertyType::Int32},
                TypeData{"clearColor", EPropertyType::Vec4f},
                TypeData{"renderOnce", EPropertyType::Bool}
            });
        auto inputs = std::make_unique<PropertyImpl>(std::move(inputsType), EPropertySemantics::BindingInput);

        setRootInputs(std::move(inputs));

        ApplyRamsesValuesToInputProperties(*this, m_ramsesRenderPass);
    }

    flatbuffers::Offset<rlogic_serialization::RenderPassBinding> RenderPassBindingImpl::Serialize(
        const RenderPassBindingImpl& renderPassBinding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        const auto logicObject = LogicObjectImpl::Serialize(renderPassBinding, builder);
        const auto fbRamsesRef = RamsesBindingImpl::SerializeRamsesReference(renderPassBinding.m_ramsesRenderPass, builder);
        const auto propertyObject = PropertyImpl::Serialize(renderPassBinding.getInputs()->impl(), builder, serializationMap);
        auto fbRamsesBinding = rlogic_serialization::CreateRamsesBinding(builder,
            logicObject,
            fbRamsesRef,
            propertyObject);

        auto fbRenderPassBinding = rlogic_serialization::CreateRenderPassBinding(builder, fbRamsesBinding);
        builder.Finish(fbRenderPassBinding);

        return fbRenderPassBinding;
    }

    std::unique_ptr<RenderPassBindingImpl> RenderPassBindingImpl::Deserialize(
        const rlogic_serialization::RenderPassBinding& renderPassBinding,
        const IRamsesObjectResolver& ramsesResolver,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        if (!renderPassBinding.base())
        {
            errorReporting.set("Fatal error during loading of RenderPassBinding from serialized data: missing base class info!", nullptr);
            return nullptr;
        }

        std::string name;
        sceneObjectId_t id{};
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(renderPassBinding.base()->base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.set("Fatal error during loading of RenderPassBinding from serialized data: missing name and/or ID!", nullptr);
            return nullptr;
        }

        if (!renderPassBinding.base()->rootInput())
        {
            errorReporting.set("Fatal error during loading of RenderPassBinding from serialized data: missing root input!", nullptr);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootInput = PropertyImpl::Deserialize(*renderPassBinding.base()->rootInput(), EPropertySemantics::BindingInput, errorReporting, deserializationMap);
        if (!deserializedRootInput)
            return nullptr;

        if (deserializedRootInput->getType() != EPropertyType::Struct)
        {
            errorReporting.set("Fatal error during loading of RenderPassBinding from serialized data: root input has unexpected type!", nullptr);
            return nullptr;
        }

        const auto* boundObject = renderPassBinding.base()->boundRamsesObject();
        if (!boundObject)
        {
            errorReporting.set("Fatal error during loading of RenderPassBinding from serialized data: missing ramses object reference!", nullptr);
            return nullptr;
        }

        const ramses::sceneObjectId_t objectId{ boundObject->objectId() };
        ramses::RenderPass* ramsesRenderPass = ramsesResolver.findRamsesRenderPassInScene(name, objectId);
        if (!ramsesRenderPass)
            return nullptr;

        if (ramsesRenderPass->getType() != static_cast<ramses::ERamsesObjectType>(boundObject->objectType()))
        {
            errorReporting.set("Fatal error during loading of RenderPassBinding from serialized data: loaded object type does not match referenced object type!", nullptr);
            return nullptr;
        }

        auto binding = std::make_unique<RenderPassBindingImpl>(deserializationMap.getScene(), *ramsesRenderPass, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs(std::move(deserializedRootInput));

        ApplyRamsesValuesToInputProperties(*binding, *ramsesRenderPass);

        return binding;
    }

    std::optional<LogicNodeRuntimeError> RenderPassBindingImpl::update()
    {
        PropertyImpl& enabled = getInputs()->getChild(EPropertyIndex_Enabled)->impl();
        if (enabled.checkForBindingInputNewValueAndReset())
        {
            if (!m_ramsesRenderPass.get().setEnabled(enabled.getValueAs<bool>()))
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
        }

        PropertyImpl& renderOrder = getInputs()->getChild(EPropertyIndex_RenderOrder)->impl();
        if (renderOrder.checkForBindingInputNewValueAndReset())
        {
            if (!m_ramsesRenderPass.get().setRenderOrder(renderOrder.getValueAs<int32_t>()))
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
        }

        PropertyImpl& clearColor = getInputs()->getChild(EPropertyIndex_ClearColor)->impl();
        if (clearColor.checkForBindingInputNewValueAndReset())
        {
            if (!m_ramsesRenderPass.get().setClearColor(clearColor.getValueAs<vec4f>()))
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
        }

        PropertyImpl& renderOnce = getInputs()->getChild(EPropertyIndex_RenderOnce)->impl();
        if (renderOnce.checkForBindingInputNewValueAndReset())
        {
            if (!m_ramsesRenderPass.get().setRenderOnce(renderOnce.getValueAs<bool>()))
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
        }

        return std::nullopt;
    }

    const ramses::RenderPass& RenderPassBindingImpl::getRamsesRenderPass() const
    {
        return m_ramsesRenderPass;
    }

    ramses::RenderPass& RenderPassBindingImpl::getRamsesRenderPass()
    {
        return m_ramsesRenderPass;
    }

    // Overwrites binding value cache silently (without triggering dirty check) - this code is only executed at creation or deserialization,
    // should not overwrite values unless set() or link explicitly called
    void RenderPassBindingImpl::ApplyRamsesValuesToInputProperties(RenderPassBindingImpl& binding, ramses::RenderPass& ramsesRenderPass)
    {
        binding.getInputs()->getChild(EPropertyIndex_Enabled)->impl().initializeBindingInputValue(PropertyValue{ ramsesRenderPass.isEnabled() });

        binding.getInputs()->getChild(EPropertyIndex_RenderOrder)->impl().initializeBindingInputValue(PropertyValue{ ramsesRenderPass.getRenderOrder() });

        vec4f clearColor = ramsesRenderPass.getClearColor();
        binding.getInputs()->getChild(EPropertyIndex_ClearColor)->impl().initializeBindingInputValue(PropertyValue{ clearColor });

        binding.getInputs()->getChild(EPropertyIndex_RenderOnce)->impl().initializeBindingInputValue(PropertyValue{ ramsesRenderPass.isRenderOnce() });
    }
}
