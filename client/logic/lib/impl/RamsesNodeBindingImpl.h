//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/RamsesBindingImpl.h"
#include "ramses-logic/ERotationType.h"
#include "ramses-logic/EPropertyType.h"
#include "ramses-logic/EFeatureLevel.h"
#include "internals/SerializationMap.h"
#include "internals/DeserializationMap.h"
#include "ramses-client-api/ERotationConvention.h"

#include <memory>

namespace ramses
{
    class Scene;
    class Node;
}

namespace rlogic_serialization
{
    struct RamsesNodeBinding;
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

    enum class ENodePropertyStaticIndex : size_t
    {
        Visibility = 0,
        Rotation = 1,
        Translation = 2,
        Scaling = 3,
        Enabled = 4
    };

    class RamsesNodeBindingImpl : public RamsesBindingImpl
    {
    public:
        // Move-able (noexcept); Not copy-able
        explicit RamsesNodeBindingImpl(ramses::Node& ramsesNode, ERotationType rotationType, std::string_view name, uint64_t id, EFeatureLevel featureLevel);
        ~RamsesNodeBindingImpl() noexcept override = default;
        RamsesNodeBindingImpl(const RamsesNodeBindingImpl& other) = delete;
        RamsesNodeBindingImpl& operator=(const RamsesNodeBindingImpl& other) = delete;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding> Serialize(
            const RamsesNodeBindingImpl& nodeBinding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap,
            EFeatureLevel featureLevel);

        [[nodiscard]] static std::unique_ptr<RamsesNodeBindingImpl> Deserialize(
            const rlogic_serialization::RamsesNodeBinding& nodeBinding,
            const IRamsesObjectResolver& ramsesResolver,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap,
            EFeatureLevel featureLevel);

        [[nodiscard]] ramses::Node& getRamsesNode() const;

        [[nodiscard]] ERotationType getRotationType() const;

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        static void ApplyRamsesValuesToInputProperties(RamsesNodeBindingImpl& binding, ramses::Node& ramsesNode, EFeatureLevel featureLevel);

        std::reference_wrapper<ramses::Node> m_ramsesNode;
        ERotationType m_rotationType;
        EFeatureLevel m_featureLevel;
    };
}
