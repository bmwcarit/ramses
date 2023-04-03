//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RamsesMeshNodeBindingImpl.h"
#include "impl/PropertyImpl.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-logic/Property.h"
#include "ramses-utils.h"
#include "internals/ErrorReporting.h"
#include "internals/RamsesObjectResolver.h"
#include "generated/RamsesMeshNodeBindingGen.h"
#include "fmt/format.h"

namespace rlogic::internal
{
    RamsesMeshNodeBindingImpl::RamsesMeshNodeBindingImpl(ramses::MeshNode& ramsesMeshNode, std::string_view name, uint64_t id)
        : RamsesBindingImpl{ name, id }
        , m_ramsesMeshNode{ ramsesMeshNode }
    {
    }

    void RamsesMeshNodeBindingImpl::createRootProperties()
    {
        HierarchicalTypeData inputsType = MakeStruct("", {
                TypeData{"vertexOffset", EPropertyType::Int32},  //EInputProperty::VertexOffset
                TypeData{"indexOffset", EPropertyType::Int32},   //EInputProperty::IndexOffset
                TypeData{"indexCount", EPropertyType::Int32},    //EInputProperty::IndexCount
                TypeData{"instanceCount", EPropertyType::Int32}  //EInputProperty::InstanceCount
            });
        auto inputs = std::make_unique<Property>(std::make_unique<PropertyImpl>(std::move(inputsType), EPropertySemantics::BindingInput));

        setRootInputs(std::move(inputs));

        ApplyRamsesValuesToInputProperties(*this, m_ramsesMeshNode);
    }

    flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding> RamsesMeshNodeBindingImpl::Serialize(
        const RamsesMeshNodeBindingImpl& meshNodeBinding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap,
        EFeatureLevel /*featureLevel*/)
    {
        const auto logicObject = LogicObjectImpl::Serialize(meshNodeBinding, builder);
        const auto fbRamsesRef = RamsesBindingImpl::SerializeRamsesReference(meshNodeBinding.m_ramsesMeshNode, builder);
        const auto propertyObject = PropertyImpl::Serialize(*meshNodeBinding.getInputs()->m_impl, builder, serializationMap);
        auto fbRamsesBinding = rlogic_serialization::CreateRamsesBinding(builder,
            logicObject,
            fbRamsesRef,
            propertyObject);

        auto fbMeshNodeBinding = rlogic_serialization::CreateRamsesMeshNodeBinding(builder, fbRamsesBinding);
        builder.Finish(fbMeshNodeBinding);

        return fbMeshNodeBinding;
    }

    std::unique_ptr<RamsesMeshNodeBindingImpl> RamsesMeshNodeBindingImpl::Deserialize(
        const rlogic_serialization::RamsesMeshNodeBinding& meshNodeBinding,
        const IRamsesObjectResolver& ramsesResolver,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        if (!meshNodeBinding.base())
        {
            errorReporting.add("Fatal error during loading of RamsesMeshNodeBinding from serialized data: missing base class info!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::string name;
        uint64_t id = 0u;
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(meshNodeBinding.base()->base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.add("Fatal error during loading of RamsesMeshNodeBinding from serialized data: missing name and/or ID!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!meshNodeBinding.base()->rootInput())
        {
            errorReporting.add("Fatal error during loading of RamsesMeshNodeBinding from serialized data: missing root input!", nullptr, EErrorType::BinaryVersionMismatch);
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
            errorReporting.add("Fatal error during loading of RamsesMeshNodeBinding from serialized data: corrupted root input!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const auto* boundObject = meshNodeBinding.base()->boundRamsesObject();
        if (!boundObject)
        {
            errorReporting.add("Fatal error during loading of RamsesMeshNodeBinding from serialized data: missing ramses object reference!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const ramses::sceneObjectId_t objectId{ boundObject->objectId() };
        ramses::SceneObject* ramsesObject = ramsesResolver.findRamsesSceneObjectInScene(name, objectId);
        if (!ramsesObject)
            return nullptr;

        if (ramsesObject->getType() != ramses::ERamsesObjectType_MeshNode || ramsesObject->getType() != static_cast<ramses::ERamsesObjectType>(boundObject->objectType()))
        {
            errorReporting.add("Fatal error during loading of RamsesMeshNodeBinding from serialized data: loaded object type does not match referenced object type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        auto* ramsesMeshNode = ramses::RamsesUtils::TryConvert<ramses::MeshNode>(*ramsesObject);
        assert(ramsesMeshNode);
        auto binding = std::make_unique<RamsesMeshNodeBindingImpl>(*ramsesMeshNode, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs(std::make_unique<Property>(std::move(deserializedRootInput)));

        ApplyRamsesValuesToInputProperties(*binding, *ramsesMeshNode);

        return binding;
    }

    std::optional<LogicNodeRuntimeError> RamsesMeshNodeBindingImpl::update()
    {
        auto prop = getInputs()->getChild(size_t(EInputProperty::VertexOffset))->m_impl.get();
        if (prop->checkForBindingInputNewValueAndReset())
        {
            const auto status = m_ramsesMeshNode.get().setStartVertex(prop->getValueAs<int32_t>());
            if (status != ramses::StatusOK)
                return LogicNodeRuntimeError{ m_ramsesMeshNode.get().getStatusMessage(status) };
        }

        prop = getInputs()->getChild(size_t(EInputProperty::IndexOffset))->m_impl.get();
        if (prop->checkForBindingInputNewValueAndReset())
        {
            const auto status = m_ramsesMeshNode.get().setStartIndex(prop->getValueAs<int32_t>());
            if (status != ramses::StatusOK)
                return LogicNodeRuntimeError{ m_ramsesMeshNode.get().getStatusMessage(status) };
        }

        prop = getInputs()->getChild(size_t(EInputProperty::IndexCount))->m_impl.get();
        if (prop->checkForBindingInputNewValueAndReset())
        {
            const auto status = m_ramsesMeshNode.get().setIndexCount(prop->getValueAs<int32_t>());
            if (status != ramses::StatusOK)
                return LogicNodeRuntimeError{ m_ramsesMeshNode.get().getStatusMessage(status) };
        }

        prop = getInputs()->getChild(size_t(EInputProperty::InstanceCount))->m_impl.get();
        if (prop->checkForBindingInputNewValueAndReset())
        {
            const auto status = m_ramsesMeshNode.get().setInstanceCount(prop->getValueAs<int32_t>());
            if (status != ramses::StatusOK)
                return LogicNodeRuntimeError{ m_ramsesMeshNode.get().getStatusMessage(status) };
        }

        return std::nullopt;
    }

    const ramses::MeshNode& RamsesMeshNodeBindingImpl::getRamsesMeshNode() const
    {
        return m_ramsesMeshNode;
    }

    ramses::MeshNode& RamsesMeshNodeBindingImpl::getRamsesMeshNode()
    {
        return m_ramsesMeshNode;
    }

    // Overwrites binding value cache silently (without triggering dirty check) - this code is only executed at initialization,
    // should not overwrite values unless set() or link explicitly called
    void RamsesMeshNodeBindingImpl::ApplyRamsesValuesToInputProperties(RamsesMeshNodeBindingImpl& binding, ramses::MeshNode& ramsesMeshNode)
    {
        binding.getInputs()->getChild(size_t(EInputProperty::VertexOffset))->m_impl->initializeBindingInputValue(static_cast<int32_t>(ramsesMeshNode.getStartVertex()));
        binding.getInputs()->getChild(size_t(EInputProperty::IndexOffset))->m_impl->initializeBindingInputValue(static_cast<int32_t>(ramsesMeshNode.getStartIndex()));
        binding.getInputs()->getChild(size_t(EInputProperty::IndexCount))->m_impl->initializeBindingInputValue(static_cast<int32_t>(ramsesMeshNode.getIndexCount()));
        binding.getInputs()->getChild(size_t(EInputProperty::InstanceCount))->m_impl->initializeBindingInputValue(static_cast<int32_t>(ramsesMeshNode.getInstanceCount()));
    }
}
