//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/logic/RamsesBindingImpl.h"
#include "ramses/client/logic/EPropertyType.h"
#include "internal/logic/SerializationMap.h"
#include "internal/logic/DeserializationMap.h"
#include "ramses/framework/ERotationType.h"

#include <memory>

namespace ramses
{
    class Scene;
    class Node;
}

namespace rlogic_serialization
{
    struct NodeBinding;
}

namespace flatbuffers
{
    template<typename T> struct Offset;
    template<bool> class FlatBufferBuilderImpl;
    using FlatBufferBuilder = FlatBufferBuilderImpl<false>;
}

namespace ramses::internal
{
    class IRamsesObjectResolver;
    class ErrorReporting;

    enum class ENodePropertyStaticIndex : size_t
    {
        Visibility = 0,
        Rotation = 1,
        Translation = 2,
        Scaling = 3,
        Enabled = 4
    };

    class NodeBindingImpl : public RamsesBindingImpl
    {
    public:
        // Move-able (noexcept); Not copy-able
        explicit NodeBindingImpl(SceneImpl& scene, ramses::Node& ramsesNode, ramses::ERotationType rotationType, std::string_view name, sceneObjectId_t id);
        ~NodeBindingImpl() noexcept override = default;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::NodeBinding> Serialize(
            const NodeBindingImpl& nodeBinding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<NodeBindingImpl> Deserialize(
            const rlogic_serialization::NodeBinding& nodeBinding,
            const IRamsesObjectResolver& ramsesResolver,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        [[nodiscard]] ramses::Node& getRamsesNode() const;

        [[nodiscard]] ramses::ERotationType getRotationType() const;

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        static void ApplyRamsesValuesToInputProperties(NodeBindingImpl& binding, ramses::Node& ramsesNode);

        std::reference_wrapper<ramses::Node> m_ramsesNode;
        ramses::ERotationType m_rotationType;
    };
}
