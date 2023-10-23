//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/RenderGroupBindingImpl.h"
#include "impl/logic/PropertyImpl.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/ramses-utils.h"
#include "impl/ErrorReporting.h"
#include "internal/logic/RamsesObjectResolver.h"
#include "internal/logic/flatbuffers/generated/RenderGroupBindingGen.h"
#include "fmt/format.h"

namespace ramses::internal
{
    RenderGroupBindingImpl::RenderGroupBindingImpl(SceneImpl& scene, ramses::RenderGroup& ramsesRenderGroup, const RenderGroupBindingElementsImpl& elements, std::string_view name, sceneObjectId_t id)
        : RamsesBindingImpl{ scene, name, id }
        , m_ramsesRenderGroup{ ramsesRenderGroup }
        , m_elements{ elements.getElements() }
    {
        assert(!m_elements.empty());
        assert(std::none_of(m_elements.cbegin(), m_elements.cend(), [](const auto& e) { return e.second == nullptr; }));
    }

    void RenderGroupBindingImpl::createRootProperties()
    {
        HierarchicalTypeData inputsType = MakeStruct("", {
                TypeData{"renderOrders", EPropertyType::Struct}
            });

        auto& elementInputs = inputsType.children.front().children;
        elementInputs.reserve(m_elements.size());
        for (const auto& e : m_elements)
            elementInputs.push_back(MakeType(e.first, EPropertyType::Int32));

        auto inputs = std::make_unique<PropertyImpl>(std::move(inputsType), EPropertySemantics::BindingInput);

        setRootInputs(std::move(inputs));

        ApplyRamsesValuesToInputProperties(*this, m_ramsesRenderGroup);
    }

    flatbuffers::Offset<rlogic_serialization::RenderGroupBinding> RenderGroupBindingImpl::Serialize(
        const RenderGroupBindingImpl& renderGroupBinding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        const auto logicObject = LogicObjectImpl::Serialize(renderGroupBinding, builder);
        const auto fbRamsesRef = RamsesBindingImpl::SerializeRamsesReference(renderGroupBinding.m_ramsesRenderGroup, builder);
        const auto propertyObject = PropertyImpl::Serialize(renderGroupBinding.getInputs()->impl(), builder, serializationMap);
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

        auto fbRenderGroupBinding = rlogic_serialization::CreateRenderGroupBinding(builder, fbRamsesBinding, builder.CreateVector(elementsFB));
        builder.Finish(fbRenderGroupBinding);

        return fbRenderGroupBinding;
    }

    std::unique_ptr<RenderGroupBindingImpl> RenderGroupBindingImpl::Deserialize(
        const rlogic_serialization::RenderGroupBinding& renderGroupBinding,
        const IRamsesObjectResolver& ramsesResolver,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        if (!renderGroupBinding.base())
        {
            errorReporting.set("Fatal error during loading of RenderGroupBinding from serialized data: missing base class info!", nullptr);
            return nullptr;
        }

        std::string name;
        sceneObjectId_t id{};
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(renderGroupBinding.base()->base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.set("Fatal error during loading of RenderGroupBinding from serialized data: missing name and/or ID!", nullptr);
            return nullptr;
        }

        if (!renderGroupBinding.base()->rootInput())
        {
            errorReporting.set("Fatal error during loading of RenderGroupBinding from serialized data: missing root input!", nullptr);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootInput = PropertyImpl::Deserialize(*renderGroupBinding.base()->rootInput(), EPropertySemantics::BindingInput, errorReporting, deserializationMap);
        if (!deserializedRootInput)
            return nullptr;

        if (deserializedRootInput->getType() != EPropertyType::Struct)
        {
            errorReporting.set("Fatal error during loading of RenderGroupBinding from serialized data: root input has unexpected type!", nullptr);
            return nullptr;
        }

        const auto* boundObject = renderGroupBinding.base()->boundRamsesObject();
        if (!boundObject)
        {
            errorReporting.set("Fatal error during loading of RenderGroupBinding from serialized data: missing ramses object reference!", nullptr);
            return nullptr;
        }

        const ramses::sceneObjectId_t objectId{ boundObject->objectId() };
        ramses::RenderGroup* ramsesRenderGroup = ramsesResolver.findRamsesRenderGroupInScene(name, objectId);
        if (!ramsesRenderGroup)
            return nullptr;

        if (ramsesRenderGroup->getType() != static_cast<ramses::ERamsesObjectType>(boundObject->objectType()))
        {
            errorReporting.set("Fatal error during loading of RenderGroupBinding from serialized data: loaded object type does not match referenced object type!", nullptr);
            return nullptr;
        }

        if (!deserializedRootInput->getChild("renderOrders") || deserializedRootInput->getChild("renderOrders")->getChildCount() != renderGroupBinding.elements()->size())
        {
            errorReporting.set("Fatal error during loading of RenderGroupBinding from serialized data: input properties do not match RenderGroup's elements!", nullptr);
            return nullptr;
        }

        RenderGroupBindingElementsImpl elements;
        for (const auto* elementFB : *renderGroupBinding.elements())
        {
            if (!elementFB->name() || !elementFB->ramsesObject())
            {
                errorReporting.set(fmt::format("Fatal error during loading of RenderGroupBinding '{}' elements data: missing name or Ramses reference!", name), nullptr);
                return nullptr;
            }

            auto elementObject = ramsesResolver.findRamsesSceneObjectInScene(name, ramses::sceneObjectId_t{ elementFB->ramsesObject()->objectId() });
            if (!elementObject)
                return nullptr;

            if (elementObject->getType() != static_cast<ramses::ERamsesObjectType>(elementFB->ramsesObject()->objectType()))
            {
                errorReporting.set(fmt::format("Fatal error during loading of RenderGroupBinding '{}' elements data: loaded element object type does not match referenced object type!", name),
                    nullptr);
                return nullptr;
            }

            if (!elements.addElement(*elementObject, elementFB->name()->string_view()))
            {
                errorReporting.set(fmt::format("Fatal error during loading of RenderGroupBinding '{}' elements data '{}' corrupted!", name, elementFB->name()->string_view()), nullptr);
                return nullptr;
            }
        }

        auto binding = std::make_unique<RenderGroupBindingImpl>(deserializationMap.getScene(), *ramsesRenderGroup, elements, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs(std::move(deserializedRootInput));

        ApplyRamsesValuesToInputProperties(*binding, *ramsesRenderGroup);

        return binding;
    }

    std::optional<LogicNodeRuntimeError> RenderGroupBindingImpl::update()
    {
        // The input properties for render order must match the ramses object references stored in m_elements.
        // This is asserted in ApplyRamsesValuesToInputProperties which is always executed once at creation/deserialization,
        // asserting everything here again would be redundant.

        auto renderOrdersProps = getInputs()->getChild(0u);
        for (std::size_t i = 0u; i < renderOrdersProps->getChildCount(); ++i)
        {
            auto& elementPropImpl = renderOrdersProps->getChild(i)->impl();
            if (elementPropImpl.checkForBindingInputNewValueAndReset())
            {
                const int32_t renderOrder = elementPropImpl.getValueAs<int32_t>();
                const auto& obj = *m_elements[i].second;
                assert(obj.isOfType(ramses::ERamsesObjectType::MeshNode) || obj.isOfType(ramses::ERamsesObjectType::RenderGroup));

                bool status = false;
                if (obj.isOfType(ramses::ERamsesObjectType::MeshNode))
                {
                    const auto meshNode = obj.as<MeshNode>();
                    if (!m_ramsesRenderGroup.get().containsMeshNode(*meshNode))
                        return LogicNodeRuntimeError{ "Cannot set render order of MeshNode which is not contained in bound RenderGroup." };
                    status = m_ramsesRenderGroup.get().addMeshNode(*meshNode, renderOrder); // we are not adding it, this is ramses way to change render order of already contained element
                }
                else
                {
                    const auto rg = obj.as<ramses::RenderGroup>();
                    if (!m_ramsesRenderGroup.get().containsRenderGroup(*rg))
                        return LogicNodeRuntimeError{ "Cannot set render order of RenderGroup which is not contained in bound RenderGroup." };
                    status = m_ramsesRenderGroup.get().addRenderGroup(*rg, renderOrder); // we are not adding it, this is ramses way to change render order of already contained element
                }

                if (!status)
                    return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
            }
        }

        return std::nullopt;
    }

    const ramses::RenderGroup& RenderGroupBindingImpl::getRamsesRenderGroup() const
    {
        return m_ramsesRenderGroup;
    }

    ramses::RenderGroup& RenderGroupBindingImpl::getRamsesRenderGroup()
    {
        return m_ramsesRenderGroup;
    }

    // Overwrites binding value cache silently (without triggering dirty check) - this code is only executed at creation or deserialization,
    // should not overwrite values unless set() or link explicitly called
    void RenderGroupBindingImpl::ApplyRamsesValuesToInputProperties(RenderGroupBindingImpl& binding, ramses::RenderGroup& ramsesRenderGroup)
    {
        auto renderOrdersProps = binding.getInputs()->getChild(0u);
        assert(renderOrdersProps && binding.m_elements.size() == renderOrdersProps->getChildCount());

        for (std::size_t i = 0u; i < renderOrdersProps->getChildCount(); ++i)
        {
            int32_t renderOrder = 0;
            const auto& obj = *binding.m_elements[i].second;

            bool status = false;
            if (obj.isOfType(ramses::ERamsesObjectType::MeshNode))
            {
                const auto meshNode = obj.as<MeshNode>();
                assert(meshNode);
                assert(ramsesRenderGroup.containsMeshNode(*meshNode));
                status = ramsesRenderGroup.getMeshNodeOrder(*meshNode, renderOrder);
            }
            else
            {
                const auto rg = obj.as<ramses::RenderGroup>();
                assert(rg);
                assert(ramsesRenderGroup.containsRenderGroup(*rg));
                status = ramsesRenderGroup.getRenderGroupOrder(*rg, renderOrder);
            }
            assert(status);
            (void)status;

            auto& elementPropImpl = renderOrdersProps->getChild(i)->impl();
            assert(elementPropImpl.getName() == binding.m_elements[i].first);
            elementPropImpl.initializeBindingInputValue(PropertyValue{ renderOrder });
        }
    }
}
