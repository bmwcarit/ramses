//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/LogicNodeImpl.h"
#include <memory>

namespace rlogic_serialization
{
    struct AnchorPoint;
}

namespace flatbuffers
{
    class FlatBufferBuilder;
    template<typename T> struct Offset;
}

namespace rlogic::internal
{
    class RamsesNodeBindingImpl;
    class RamsesCameraBindingImpl;
    class ErrorReporting;
    class SerializationMap;
    class DeserializationMap;

    class AnchorPointImpl : public LogicNodeImpl
    {
    public:
        // Move-able (noexcept); Not copy-able
        explicit AnchorPointImpl(RamsesNodeBindingImpl& nodeBinding, RamsesCameraBindingImpl& cameraBinding, std::string_view name, uint64_t id);
        ~AnchorPointImpl() noexcept override = default;
        AnchorPointImpl(const AnchorPointImpl& other) = delete;
        AnchorPointImpl& operator=(const AnchorPointImpl& other) = delete;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::AnchorPoint> Serialize(
            const AnchorPointImpl& anchorPoint,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<AnchorPointImpl> Deserialize(
            const rlogic_serialization::AnchorPoint& anchorPoint,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        [[nodiscard]] RamsesNodeBindingImpl& getRamsesNodeBinding();
        [[nodiscard]] RamsesCameraBindingImpl& getRamsesCameraBinding();

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        RamsesNodeBindingImpl& m_nodeBinding;
        RamsesCameraBindingImpl& m_cameraBinding;
    };
}
