//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/MeshNodeBindingImpl.h"
#include "impl/logic/PropertyImpl.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/ramses-utils.h"
#include "impl/ErrorReporting.h"
#include "internal/logic/RamsesObjectResolver.h"
#include "internal/logic/flatbuffers/generated/MeshNodeBindingGen.h"
#include "fmt/format.h"

namespace ramses::internal
{
    MeshNodeBindingImpl::MeshNodeBindingImpl(SceneImpl& scene, ramses::MeshNode& ramsesMeshNode, std::string_view name, sceneObjectId_t id)
        : RamsesBindingImpl{ scene, name, id }
        , m_ramsesMeshNode{ ramsesMeshNode }
    {
    }

    void MeshNodeBindingImpl::createRootProperties()
    {
        HierarchicalTypeData inputsType = MakeStruct("", {
                TypeData{"vertexOffset", EPropertyType::Int32},  //EInputProperty::VertexOffset
                TypeData{"indexOffset", EPropertyType::Int32},   //EInputProperty::IndexOffset
                TypeData{"indexCount", EPropertyType::Int32},    //EInputProperty::IndexCount
                TypeData{"instanceCount", EPropertyType::Int32}  //EInputProperty::InstanceCount
            });
        auto inputs = std::make_unique<PropertyImpl>(std::move(inputsType), EPropertySemantics::BindingInput);

        setRootInputs(std::move(inputs));

        ApplyRamsesValuesToInputProperties(*this, m_ramsesMeshNode);
    }

    flatbuffers::Offset<rlogic_serialization::MeshNodeBinding> MeshNodeBindingImpl::Serialize(
        const MeshNodeBindingImpl& meshNodeBinding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        const auto logicObject = LogicObjectImpl::Serialize(meshNodeBinding, builder);
        const auto fbRamsesRef = RamsesBindingImpl::SerializeRamsesReference(meshNodeBinding.m_ramsesMeshNode, builder);
        const auto propertyObject = PropertyImpl::Serialize(meshNodeBinding.getInputs()->impl(), builder, serializationMap);
        auto fbRamsesBinding = rlogic_serialization::CreateRamsesBinding(builder,
            logicObject,
            fbRamsesRef,
            propertyObject);

        auto fbMeshNodeBinding = rlogic_serialization::CreateMeshNodeBinding(builder, fbRamsesBinding);
        builder.Finish(fbMeshNodeBinding);

        return fbMeshNodeBinding;
    }

    std::unique_ptr<MeshNodeBindingImpl> MeshNodeBindingImpl::Deserialize(
        const rlogic_serialization::MeshNodeBinding& meshNodeBinding,
        const IRamsesObjectResolver& ramsesResolver,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        if (!meshNodeBinding.base())
        {
            errorReporting.set("Fatal error during loading of MeshNodeBinding from serialized data: missing base class info!", nullptr);
            return nullptr;
        }

        std::string name;
        sceneObjectId_t id{};
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(meshNodeBinding.base()->base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.set("Fatal error during loading of MeshNodeBinding from serialized data: missing name and/or ID!", nullptr);
            return nullptr;
        }

        if (!meshNodeBinding.base()->rootInput())
        {
            errorReporting.set("Fatal error during loading of MeshNodeBinding from serialized data: missing root input!", nullptr);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootInput = PropertyImpl::Deserialize(*meshNodeBinding.base()->rootInput(), EPropertySemantics::BindingInput, errorReporting, deserializationMap);
        if (!deserializedRootInput)
            return nullptr;

        if (deserializedRootInput->getType() != EPropertyType::Struct ||
            deserializedRootInput->getChildCount() != size_t(EInputProperty::COUNT) ||
            deserializedRootInput->getChild(size_t(EInputProperty::VertexOffset))->getName() != "vertexOffset" ||
            deserializedRootInput->getChild(size_t(EInputProperty::IndexOffset))->getName() != "indexOffset" ||
            deserializedRootInput->getChild(size_t(EInputProperty::IndexCount))->getName() != "indexCount" ||
            deserializedRootInput->getChild(size_t(EInputProperty::InstanceCount))->getName() != "instanceCount")
        {
            errorReporting.set("Fatal error during loading of MeshNodeBinding from serialized data: corrupted root input!", nullptr);
            return nullptr;
        }

        const auto* boundObject = meshNodeBinding.base()->boundRamsesObject();
        if (!boundObject)
        {
            errorReporting.set("Fatal error during loading of MeshNodeBinding from serialized data: missing ramses object reference!", nullptr);
            return nullptr;
        }

        const ramses::sceneObjectId_t objectId{ boundObject->objectId() };
        ramses::SceneObject* ramsesObject = ramsesResolver.findRamsesSceneObjectInScene(name, objectId);
        if (!ramsesObject)
            return nullptr;

        if (ramsesObject->getType() != ramses::ERamsesObjectType::MeshNode || ramsesObject->getType() != static_cast<ramses::ERamsesObjectType>(boundObject->objectType()))
        {
            errorReporting.set("Fatal error during loading of MeshNodeBinding from serialized data: loaded object type does not match referenced object type!", nullptr);
            return nullptr;
        }

        auto* ramsesMeshNode = ramsesObject->as<MeshNode>();
        assert(ramsesMeshNode);
        auto binding = std::make_unique<MeshNodeBindingImpl>(deserializationMap.getScene(), *ramsesMeshNode, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs(std::move(deserializedRootInput));

        ApplyRamsesValuesToInputProperties(*binding, *ramsesMeshNode);

        return binding;
    }

    std::optional<LogicNodeRuntimeError> MeshNodeBindingImpl::update()
    {
        auto prop = &getInputs()->getChild(size_t(EInputProperty::VertexOffset))->impl();
        if (prop->checkForBindingInputNewValueAndReset())
        {
            const auto vtxOffset = prop->getValueAs<int32_t>();
            if (vtxOffset < 0)
                return LogicNodeRuntimeError{ "MeshNodeBinding vertex offset cannot be negative" };

            if (!m_ramsesMeshNode.get().setStartVertex(static_cast<uint32_t>(vtxOffset)))
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
        }

        prop = &getInputs()->getChild(size_t(EInputProperty::IndexOffset))->impl();
        if (prop->checkForBindingInputNewValueAndReset())
        {
            const auto idxOffset = prop->getValueAs<int32_t>();
            if (idxOffset < 0)
                return LogicNodeRuntimeError{ "MeshNodeBinding index offset cannot be negative" };

            if (!m_ramsesMeshNode.get().setStartIndex(static_cast<uint32_t>(idxOffset)))
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
        }

        prop = &getInputs()->getChild(size_t(EInputProperty::IndexCount))->impl();
        if (prop->checkForBindingInputNewValueAndReset())
        {
            const auto idxCount = prop->getValueAs<int32_t>();
            if (idxCount < 0)
                return LogicNodeRuntimeError{ "MeshNodeBinding index count cannot be negative" };

            if (!m_ramsesMeshNode.get().setIndexCount(static_cast<uint32_t>(idxCount)))
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
        }

        prop = &getInputs()->getChild(size_t(EInputProperty::InstanceCount))->impl();
        if (prop->checkForBindingInputNewValueAndReset())
        {
            const auto instCount = prop->getValueAs<int32_t>();
            if (instCount < 0)
                return LogicNodeRuntimeError{ "MeshNodeBinding instance count cannot be negative" };

            if (!m_ramsesMeshNode.get().setInstanceCount(static_cast<uint32_t>(instCount)))
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
        }

        return std::nullopt;
    }

    const ramses::MeshNode& MeshNodeBindingImpl::getRamsesMeshNode() const
    {
        return m_ramsesMeshNode;
    }

    ramses::MeshNode& MeshNodeBindingImpl::getRamsesMeshNode()
    {
        return m_ramsesMeshNode;
    }

    // Overwrites binding value cache silently (without triggering dirty check) - this code is only executed at creation or deserialization,
    // should not overwrite values unless set() or link explicitly called
    void MeshNodeBindingImpl::ApplyRamsesValuesToInputProperties(MeshNodeBindingImpl& binding, ramses::MeshNode& ramsesMeshNode)
    {
        binding.getInputs()->getChild(size_t(EInputProperty::VertexOffset))->impl().initializeBindingInputValue(static_cast<int32_t>(ramsesMeshNode.getStartVertex()));
        binding.getInputs()->getChild(size_t(EInputProperty::IndexOffset))->impl().initializeBindingInputValue(static_cast<int32_t>(ramsesMeshNode.getStartIndex()));
        binding.getInputs()->getChild(size_t(EInputProperty::IndexCount))->impl().initializeBindingInputValue(static_cast<int32_t>(ramsesMeshNode.getIndexCount()));
        binding.getInputs()->getChild(size_t(EInputProperty::InstanceCount))->impl().initializeBindingInputValue(static_cast<int32_t>(ramsesMeshNode.getInstanceCount()));
    }
}
