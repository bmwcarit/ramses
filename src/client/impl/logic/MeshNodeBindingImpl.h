//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/logic/RamsesBindingImpl.h"
#include <memory>

namespace ramses
{
    class MeshNode;
}

namespace rlogic_serialization
{
    struct MeshNodeBinding;
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
    class SerializationMap;
    class DeserializationMap;

    class MeshNodeBindingImpl : public RamsesBindingImpl
    {
    public:
        // Move-able (noexcept); Not copy-able
        explicit MeshNodeBindingImpl(SceneImpl& scene, ramses::MeshNode& ramsesMeshNode, std::string_view name, sceneObjectId_t id);
        ~MeshNodeBindingImpl() noexcept override = default;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::MeshNodeBinding> Serialize(
            const MeshNodeBindingImpl& meshNodeBinding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<MeshNodeBindingImpl> Deserialize(
            const rlogic_serialization::MeshNodeBinding& meshNodeBinding,
            const IRamsesObjectResolver& ramsesResolver,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        [[nodiscard]] const ramses::MeshNode& getRamsesMeshNode() const;
        [[nodiscard]] ramses::MeshNode& getRamsesMeshNode();

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

        enum class EInputProperty
        {
            VertexOffset = 0,
            IndexOffset,
            IndexCount,
            InstanceCount,

            COUNT
        };

    private:
        static void ApplyRamsesValuesToInputProperties(MeshNodeBindingImpl& binding, ramses::MeshNode& ramsesMeshNode);

        std::reference_wrapper<ramses::MeshNode> m_ramsesMeshNode;
    };
}
