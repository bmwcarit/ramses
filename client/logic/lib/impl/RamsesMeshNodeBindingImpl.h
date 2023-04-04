//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/RamsesBindingImpl.h"
#include "ramses-logic/EFeatureLevel.h"
#include <memory>

namespace ramses
{
    class MeshNode;
}

namespace rlogic_serialization
{
    struct RamsesMeshNodeBinding;
}

namespace flatbuffers
{
    class FlatBufferBuilder;
    template<typename T> struct Offset;
}

namespace rlogic::internal
{
    class IRamsesObjectResolver;
    class ErrorReporting;
    class SerializationMap;
    class DeserializationMap;

    class RamsesMeshNodeBindingImpl : public RamsesBindingImpl
    {
    public:
        // Move-able (noexcept); Not copy-able
        explicit RamsesMeshNodeBindingImpl(ramses::MeshNode& ramsesMeshNode, std::string_view name, uint64_t id);
        ~RamsesMeshNodeBindingImpl() noexcept override = default;
        RamsesMeshNodeBindingImpl(const RamsesMeshNodeBindingImpl& other) = delete;
        RamsesMeshNodeBindingImpl& operator=(const RamsesMeshNodeBindingImpl& other) = delete;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding> Serialize(
            const RamsesMeshNodeBindingImpl& meshNodeBinding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap,
            EFeatureLevel featureLevel);

        [[nodiscard]] static std::unique_ptr<RamsesMeshNodeBindingImpl> Deserialize(
            const rlogic_serialization::RamsesMeshNodeBinding& meshNodeBinding,
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
        static void ApplyRamsesValuesToInputProperties(RamsesMeshNodeBindingImpl& binding, ramses::MeshNode& ramsesMeshNode);

        std::reference_wrapper<ramses::MeshNode> m_ramsesMeshNode;
    };
}
