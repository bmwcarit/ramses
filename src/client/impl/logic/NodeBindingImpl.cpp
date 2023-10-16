//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/NodeBindingImpl.h"

#include "ramses/client/Node.h"

#include "ramses/client/logic/Property.h"

#include "impl/logic/PropertyImpl.h"
#include "internal/Core/Utils/LogMacros.h"

#include "impl/ErrorReporting.h"
#include "internal/logic/RamsesObjectResolver.h"

#include "internal/logic/flatbuffers/generated/NodeBindingGen.h"
#include "glm/gtc/type_ptr.hpp"

namespace ramses::internal
{
    NodeBindingImpl::NodeBindingImpl(SceneImpl& scene, ramses::Node& ramsesNode, ramses::ERotationType rotationType, std::string_view name, sceneObjectId_t id)
        : RamsesBindingImpl{ scene, name, id }
        , m_ramsesNode{ ramsesNode }
        , m_rotationType{ rotationType }
    {
    }

    void NodeBindingImpl::createRootProperties()
    {
        // Attention! This order is important - it has to match the indices in ENodePropertyStaticIndex!
        auto inputsType = MakeStruct("", {
                TypeData{"visibility", EPropertyType::Bool},
                TypeData{"rotation", m_rotationType == ramses::ERotationType::Quaternion ? EPropertyType::Vec4f : EPropertyType::Vec3f},
                TypeData{"translation", EPropertyType::Vec3f},
                TypeData{"scaling", EPropertyType::Vec3f},
                TypeData{"enabled", EPropertyType::Bool}
            });
        auto inputs = std::make_unique<PropertyImpl>(inputsType, EPropertySemantics::BindingInput);

        setRootInputs(std::move(inputs));

        ApplyRamsesValuesToInputProperties(*this, m_ramsesNode);
    }

    flatbuffers::Offset<rlogic_serialization::NodeBinding> NodeBindingImpl::Serialize(
        const NodeBindingImpl& binding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        auto ramsesReference = RamsesBindingImpl::SerializeRamsesReference(binding.m_ramsesNode, builder);

        const auto logicObject = LogicObjectImpl::Serialize(binding, builder);
        const auto propertyObject = PropertyImpl::Serialize(binding.getInputs()->impl(), builder, serializationMap);
        auto ramsesBinding = rlogic_serialization::CreateRamsesBinding(builder,
            logicObject,
            ramsesReference,
            propertyObject);
        builder.Finish(ramsesBinding);

        auto nodeBinding = rlogic_serialization::CreateNodeBinding(builder,
            ramsesBinding,
            static_cast<uint8_t>(binding.m_rotationType)
        );
        builder.Finish(nodeBinding);

        return nodeBinding;
    }

    std::unique_ptr<NodeBindingImpl> NodeBindingImpl::Deserialize(
        const rlogic_serialization::NodeBinding& nodeBinding,
        const IRamsesObjectResolver& ramsesResolver,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        if (!nodeBinding.base())
        {
            errorReporting.set("Fatal error during loading of NodeBinding from serialized data: missing base class info!", nullptr);
            return nullptr;
        }

        std::string name;
        sceneObjectId_t id{};
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(nodeBinding.base()->base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.set("Fatal error during loading of NodeBinding from serialized data: missing name and/or ID!", nullptr);
            return nullptr;
        }

        if (!nodeBinding.base()->rootInput())
        {
            errorReporting.set("Fatal error during loading of NodeBinding from serialized data: missing root input!", nullptr);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootInput = PropertyImpl::Deserialize(*nodeBinding.base()->rootInput(), EPropertySemantics::BindingInput, errorReporting, deserializationMap);

        if (!deserializedRootInput)
        {
            return nullptr;
        }

        if (deserializedRootInput->getType() != EPropertyType::Struct)
        {
            errorReporting.set("Fatal error during loading of NodeBinding from serialized data: root input has unexpected type!", nullptr);
            return nullptr;
        }

        const auto* boundObject = nodeBinding.base()->boundRamsesObject();
        if (!boundObject)
        {
            errorReporting.set("Fatal error during loading of NodeBinding from serialized data: missing ramses object reference!", nullptr);
            return nullptr;
        }

        const ramses::sceneObjectId_t objectId(boundObject->objectId());

        ramses::Node* ramsesNode = ramsesResolver.findRamsesNodeInScene(name, objectId);
        if (!ramsesNode)
        {
            // TODO Violin improve error reporting for this particular error (it's reported in ramsesResolver currently): provide better message and scene/node ids
            return nullptr;
        }

        if (ramsesNode->getType() != static_cast<ramses::ERamsesObjectType>(boundObject->objectType()))
        {
            errorReporting.set("Fatal error during loading of NodeBinding from serialized data: loaded node type does not match referenced node type!", nullptr);
            return nullptr;
        }

        const auto rotationType (static_cast<ramses::ERotationType>(nodeBinding.rotationType()));

        auto binding = std::make_unique<NodeBindingImpl>(deserializationMap.getScene(), *ramsesNode, rotationType, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs(std::move(deserializedRootInput));

        ApplyRamsesValuesToInputProperties(*binding, *ramsesNode);

        return binding;
    }

    std::optional<LogicNodeRuntimeError> NodeBindingImpl::update()
    {
        PropertyImpl& visibility = getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Visibility))->impl();
        PropertyImpl& enabled = getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Enabled))->impl();

        // Ramses uses 3-state visibility mode, transform the 2 bool properties 'visibility' and 'enabled' into 3-state
        const bool visibilityChanged = visibility.checkForBindingInputNewValueAndReset();
        const bool enabledChanged = enabled.checkForBindingInputNewValueAndReset();
        if (visibilityChanged || enabledChanged)
        {
            ramses::EVisibilityMode visibilityMode = (visibility.getValueAs<bool>() ? ramses::EVisibilityMode::Visible : ramses::EVisibilityMode::Invisible);
            // if 'enabled' false it overrides visibility mode to OFF, otherwise (enabled==true) the mode is determined by 'visibility' only
            if (!enabled.getValueAs<bool>())
                visibilityMode = ramses::EVisibilityMode::Off;

            if (!m_ramsesNode.get().setVisibility(visibilityMode))
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
        }

        PropertyImpl& rotation = getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Rotation))->impl();
        if (rotation.checkForBindingInputNewValueAndReset())
        {
            bool status = false;
            if (m_rotationType == ramses::ERotationType::Quaternion)
            {
                const auto& value = rotation.getValueAs<vec4f>();
                status = m_ramsesNode.get().setRotation(quat(value[3], value[0], value[1], value[2]));
            }
            else
            {
                const auto& valuesEuler = rotation.getValueAs<vec3f>();
                status = m_ramsesNode.get().setRotation(valuesEuler, m_rotationType);
            }

            if (!status)
                return LogicNodeRuntimeError{getErrorReporting().getError()->message};
        }

        PropertyImpl& translation = getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Translation))->impl();
        if (translation.checkForBindingInputNewValueAndReset())
        {
            const auto& value = translation.getValueAs<vec3f>();
            if (!m_ramsesNode.get().setTranslation(value))
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
        }

        PropertyImpl& scaling = getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Scaling))->impl();
        if (scaling.checkForBindingInputNewValueAndReset())
        {
            const auto& value = scaling.getValueAs<vec3f>();
            if (!m_ramsesNode.get().setScaling(value))
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
        }

        return std::nullopt;
    }

    ramses::Node& NodeBindingImpl::getRamsesNode() const
    {
        return m_ramsesNode;
    }

    ramses::ERotationType NodeBindingImpl::getRotationType() const
    {
        return m_rotationType;
    }

    // Overwrites binding value cache silently (without triggering dirty check) - this code is only executed at creation or deserialization,
    // should not overwrite values unless set() or link explicitly called
    void NodeBindingImpl::ApplyRamsesValuesToInputProperties(NodeBindingImpl& binding, ramses::Node& ramsesNode)
    {
        // The 3-state ramses visibility mode is transformed into 2 bool properties in node binding - 'visibility' and 'enabled'.
        const ramses::EVisibilityMode visibilityMode = ramsesNode.getVisibility();
        const bool enabled = (visibilityMode == ramses::EVisibilityMode::Visible || visibilityMode == ramses::EVisibilityMode::Invisible);
        binding.getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Enabled))->impl().initializeBindingInputValue(PropertyValue{ enabled });
        // converting 3-state into 2 bools (4 states) will always be ambiguous, for the ambiguous combination (enabled=false, visibility=false/true?)
        // we leave the visibility property untouched so it holds the state that was serialized to file
        if (enabled)
        {
            const bool visible = (visibilityMode == ramses::EVisibilityMode::Visible);
            binding.getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Visibility))->impl().initializeBindingInputValue(PropertyValue{ visible });
        }

        vec3f translationValue;
        ramsesNode.getTranslation(translationValue);
        binding.getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Translation))->impl().initializeBindingInputValue(PropertyValue{ translationValue });

        vec3f scalingValue;
        ramsesNode.getScaling(scalingValue);
        binding.getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Scaling))->impl().initializeBindingInputValue(PropertyValue{ scalingValue });

        const ramses::ERotationType rotationType = ramsesNode.getRotationType();
        if (binding.m_rotationType == ramses::ERotationType::Quaternion)
        {
            vec4f rotationValue = {0.f, 0.f, 0.f, 1.f};
            if (rotationType == ramses::ERotationType::Quaternion)
            {
                quat quaternion;
                ramsesNode.getRotation(quaternion);
                rotationValue = {quaternion.x, quaternion.y, quaternion.z, quaternion.w};
            }
            else
            {
                vec3f euler;
                ramsesNode.getRotation(euler);
                // Allow special case where rotation is not set (i.e. zero) -> mismatching rotationType is OK in this case
                // Otherwise issue a warning
                if (euler[0] != 0.f || euler[1] != 0.f || euler[2] != 0.f)
                {
                    LOG_WARN_P(CONTEXT_CLIENT, "Initial rotation values for NodeBinding '{}' will not be imported from bound Ramses node due to mismatching rotation type. Expected Quaternion, got Euler.",
                             binding.getIdentificationString());
                }
            }
            binding.getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Rotation))->impl().initializeBindingInputValue(rotationValue);
        }
        else
        {
            vec3f rotationValue;
            ramsesNode.getRotation(rotationValue);

            if (binding.m_rotationType != rotationType)
            {
                // Allow special case where rotation is not set (i.e. zero) -> mismatching convention is OK in this case
                // Otherwise issue a warning
                if (rotationValue[0] != 0.f || rotationValue[1] != 0.f || rotationValue[2] != 0.f)
                {
                    LOG_WARN_P(CONTEXT_CLIENT, "Initial rotation values for NodeBinding '{}' will not be imported from bound Ramses node due to mismatching rotation type.", binding.getIdentificationString());
                }
            }
            else
            {
                binding.getInputs()->getChild(static_cast<size_t>(ENodePropertyStaticIndex::Rotation))->impl().initializeBindingInputValue(PropertyValue{ rotationValue });
            }
        }
    }
}
