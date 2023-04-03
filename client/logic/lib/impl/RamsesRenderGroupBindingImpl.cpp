//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RamsesRenderGroupBindingImpl.h"
#include "impl/PropertyImpl.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-logic/Property.h"
#include "ramses-utils.h"
#include "internals/ErrorReporting.h"
#include "internals/RamsesObjectResolver.h"
#include "generated/RamsesRenderGroupBindingGen.h"
#include "fmt/format.h"

namespace rlogic::internal
{
    RamsesRenderGroupBindingImpl::RamsesRenderGroupBindingImpl(ramses::RenderGroup& ramsesRenderGroup, const RamsesRenderGroupBindingElementsImpl& elements, std::string_view name, uint64_t id)
        : RamsesBindingImpl{ name, id }
        , m_ramsesRenderGroup{ ramsesRenderGroup }
        , m_elements{ elements.getElements() }
    {
        assert(!m_elements.empty());
        assert(std::none_of(m_elements.cbegin(), m_elements.cend(), [](const auto& e) { return e.second == nullptr; }));
    }

    void RamsesRenderGroupBindingImpl::createRootProperties()
    {
        HierarchicalTypeData inputsType = MakeStruct("", {
                TypeData{"renderOrders", EPropertyType::Struct}
            });

        auto& elementInputs = inputsType.children.front().children;
        elementInputs.reserve(m_elements.size());
        for (const auto& e : m_elements)
            elementInputs.push_back(MakeType(e.first, EPropertyType::Int32));

        auto inputs = std::make_unique<Property>(std::make_unique<PropertyImpl>(std::move(inputsType), EPropertySemantics::BindingInput));

        setRootInputs(std::move(inputs));

        ApplyRamsesValuesToInputProperties(*this, m_ramsesRenderGroup);
    }

    flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding> RamsesRenderGroupBindingImpl::Serialize(
        const RamsesRenderGroupBindingImpl& renderGroupBinding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap,
        EFeatureLevel /*featureLevel*/)
    {
        const auto logicObject = LogicObjectImpl::Serialize(renderGroupBinding, builder);
        const auto fbRamsesRef = RamsesBindingImpl::SerializeRamsesReference(renderGroupBinding.m_ramsesRenderGroup, builder);
        const auto propertyObject = PropertyImpl::Serialize(*renderGroupBinding.getInputs()->m_impl, builder, serializationMap);
        auto fbRamsesBinding = rlogic_serialization::CreateRamsesBinding(builder,
            logicObject,
            fbRamsesRef,
            propertyObject);

        std::vector<flatbuffers::Offset<rlogic_serialization::Element>> elementsFB;
        elementsFB.reserve(renderGroupBinding.m_elements.size());
        for (const auto& element : renderGroupBinding.m_elements)
        {
            const auto ramsesReferenceFB = RamsesBindingImpl::SerializeRamsesReference(*element.second, builder);
            elementsFB.push_back(rlogic_serialization::CreateElement(
                builder,
                builder.CreateString(element.first),
                ramsesReferenceFB));
        }

        auto fbRenderGroupBinding = rlogic_serialization::CreateRamsesRenderGroupBinding(builder, fbRamsesBinding, builder.CreateVector(elementsFB));
        builder.Finish(fbRenderGroupBinding);

        return fbRenderGroupBinding;
    }

    std::unique_ptr<RamsesRenderGroupBindingImpl> RamsesRenderGroupBindingImpl::Deserialize(
        const rlogic_serialization::RamsesRenderGroupBinding& renderGroupBinding,
        const IRamsesObjectResolver& ramsesResolver,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        if (!renderGroupBinding.base())
        {
            errorReporting.add("Fatal error during loading of RamsesRenderGroupBinding from serialized data: missing base class info!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::string name;
        uint64_t id = 0u;
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(renderGroupBinding.base()->base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.add("Fatal error during loading of RamsesRenderGroupBinding from serialized data: missing name and/or ID!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!renderGroupBinding.base()->rootInput())
        {
            errorReporting.add("Fatal error during loading of RamsesRenderGroupBinding from serialized data: missing root input!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootInput = PropertyImpl::Deserialize(*renderGroupBinding.base()->rootInput(), EPropertySemantics::BindingInput, errorReporting, deserializationMap);
        if (!deserializedRootInput)
            return nullptr;

        if (deserializedRootInput->getType() != EPropertyType::Struct)
        {
            errorReporting.add("Fatal error during loading of RamsesRenderGroupBinding from serialized data: root input has unexpected type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const auto* boundObject = renderGroupBinding.base()->boundRamsesObject();
        if (!boundObject)
        {
            errorReporting.add("Fatal error during loading of RamsesRenderGroupBinding from serialized data: missing ramses object reference!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const ramses::sceneObjectId_t objectId{ boundObject->objectId() };
        ramses::RenderGroup* ramsesRenderGroup = ramsesResolver.findRamsesRenderGroupInScene(name, objectId);
        if (!ramsesRenderGroup)
            return nullptr;

        if (ramsesRenderGroup->getType() != static_cast<ramses::ERamsesObjectType>(boundObject->objectType()))
        {
            errorReporting.add("Fatal error during loading of RamsesRenderGroupBinding from serialized data: loaded object type does not match referenced object type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!deserializedRootInput->getChild("renderOrders") || deserializedRootInput->getChild("renderOrders")->getChildCount() != renderGroupBinding.elements()->size())
        {
            errorReporting.add("Fatal error during loading of RamsesRenderGroupBinding from serialized data: input properties do not match RenderGroup's elements!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        RamsesRenderGroupBindingElementsImpl elements;
        for (const auto* elementFB : *renderGroupBinding.elements())
        {
            if (!elementFB->name() || !elementFB->ramsesObject())
            {
                errorReporting.add(fmt::format("Fatal error during loading of RamsesRenderGroupBinding '{}' elements data: missing name or Ramses reference!", name), nullptr, EErrorType::BinaryVersionMismatch);
                return nullptr;
            }

            auto elementObject = ramsesResolver.findRamsesSceneObjectInScene(name, ramses::sceneObjectId_t{ elementFB->ramsesObject()->objectId() });
            if (!elementObject)
                return nullptr;

            if (elementObject->getType() != static_cast<ramses::ERamsesObjectType>(elementFB->ramsesObject()->objectType()))
            {
                errorReporting.add(fmt::format("Fatal error during loading of RamsesRenderGroupBinding '{}' elements data: loaded element object type does not match referenced object type!", name),
                    nullptr, EErrorType::BinaryVersionMismatch);
                return nullptr;
            }

            if (!elements.addElement(*elementObject, elementFB->name()->string_view()))
            {
                errorReporting.add(fmt::format("Fatal error during loading of RamsesRenderGroupBinding '{}' elements data '{}' corrupted!", name, elementFB->name()->string_view()), nullptr, EErrorType::BinaryVersionMismatch);
                return nullptr;
            }
        }

        auto binding = std::make_unique<RamsesRenderGroupBindingImpl>(*ramsesRenderGroup, elements, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs(std::make_unique<Property>(std::move(deserializedRootInput)));

        ApplyRamsesValuesToInputProperties(*binding, *ramsesRenderGroup);

        return binding;
    }

    std::optional<LogicNodeRuntimeError> RamsesRenderGroupBindingImpl::update()
    {
        // The input properties for render order must match the ramses object references stored in m_elements.
        // This is asserted in ApplyRamsesValuesToInputProperties which is always executed once at creation/deserialization,
        // asserting everything here again would be redundant.

        auto renderOrdersProps = getInputs()->getChild(0u);
        for (std::size_t i = 0u; i < renderOrdersProps->getChildCount(); ++i)
        {
            auto& elementPropImpl = *renderOrdersProps->getChild(i)->m_impl;
            if (elementPropImpl.checkForBindingInputNewValueAndReset())
            {
                const int32_t renderOrder = elementPropImpl.getValueAs<int32_t>();
                const auto& obj = *m_elements[i].second;
                assert(obj.isOfType(ramses::ERamsesObjectType_MeshNode) || obj.isOfType(ramses::ERamsesObjectType_RenderGroup));

                ramses::status_t status = ramses::StatusOK;
                if (obj.isOfType(ramses::ERamsesObjectType_MeshNode))
                {
                    const auto meshNode = ramses::RamsesUtils::TryConvert<ramses::MeshNode>(obj);
                    if (!m_ramsesRenderGroup.get().containsMeshNode(*meshNode))
                        return LogicNodeRuntimeError{ "Cannot set render order of MeshNode which is not contained in bound RenderGroup." };
                    status = m_ramsesRenderGroup.get().addMeshNode(*meshNode, renderOrder); // we are not adding it, this is ramses way to change render order of already contained element
                }
                else
                {
                    const auto rg = ramses::RamsesUtils::TryConvert<ramses::RenderGroup>(obj);
                    if (!m_ramsesRenderGroup.get().containsRenderGroup(*rg))
                        return LogicNodeRuntimeError{ "Cannot set render order of RenderGroup which is not contained in bound RenderGroup." };
                    status = m_ramsesRenderGroup.get().addRenderGroup(*rg, renderOrder); // we are not adding it, this is ramses way to change render order of already contained element
                }

                if (status != ramses::StatusOK)
                    return LogicNodeRuntimeError{ m_ramsesRenderGroup.get().getStatusMessage(status) };
            }
        }

        return std::nullopt;
    }

    const ramses::RenderGroup& RamsesRenderGroupBindingImpl::getRamsesRenderGroup() const
    {
        return m_ramsesRenderGroup;
    }

    ramses::RenderGroup& RamsesRenderGroupBindingImpl::getRamsesRenderGroup()
    {
        return m_ramsesRenderGroup;
    }

    // Overwrites binding value cache silently (without triggering dirty check) - this code is only executed at initialization,
    // should not overwrite values unless set() or link explicitly called
    void RamsesRenderGroupBindingImpl::ApplyRamsesValuesToInputProperties(RamsesRenderGroupBindingImpl& binding, ramses::RenderGroup& ramsesRenderGroup)
    {
        auto renderOrdersProps = binding.getInputs()->getChild(0u);
        assert(renderOrdersProps && binding.m_elements.size() == renderOrdersProps->getChildCount());

        for (std::size_t i = 0u; i < renderOrdersProps->getChildCount(); ++i)
        {
            int32_t renderOrder = 0;
            const auto& obj = *binding.m_elements[i].second;

            ramses::status_t status = ramses::StatusOK;
            if (obj.isOfType(ramses::ERamsesObjectType_MeshNode))
            {
                const auto meshNode = ramses::RamsesUtils::TryConvert<ramses::MeshNode>(obj);
                assert(meshNode);
                assert(ramsesRenderGroup.containsMeshNode(*meshNode));
                status = ramsesRenderGroup.getMeshNodeOrder(*meshNode, renderOrder);
            }
            else
            {
                const auto rg = ramses::RamsesUtils::TryConvert<ramses::RenderGroup>(obj);
                assert(rg);
                assert(ramsesRenderGroup.containsRenderGroup(*rg));
                status = ramsesRenderGroup.getRenderGroupOrder(*rg, renderOrder);
            }
            assert(status == ramses::StatusOK);
            (void)status;

            auto& elementPropImpl = *renderOrdersProps->getChild(i)->m_impl;
            assert(elementPropImpl.getName() == binding.m_elements[i].first);
            elementPropImpl.initializeBindingInputValue(PropertyValue{ renderOrder });
        }
    }
}
