//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/logic/LogicNodeImpl.h"
#include <memory>

namespace rlogic_serialization
{
    struct AnchorPoint;
}

namespace flatbuffers
{
    template<typename T> struct Offset;
    template<bool> class FlatBufferBuilderImpl;
    using FlatBufferBuilder = FlatBufferBuilderImpl<false>;
}

namespace ramses::internal
{
    class NodeBindingImpl;
    class CameraBindingImpl;
    class ErrorReporting;
    class SerializationMap;
    class DeserializationMap;

    class AnchorPointImpl : public LogicNodeImpl
    {
    public:
        // Move-able (noexcept); Not copy-able
        explicit AnchorPointImpl(SceneImpl& scene, NodeBindingImpl& nodeBinding, CameraBindingImpl& cameraBinding, std::string_view name, sceneObjectId_t id);
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

        [[nodiscard]] NodeBindingImpl& getNodeBinding();
        [[nodiscard]] CameraBindingImpl& getCameraBinding();

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        NodeBindingImpl& m_nodeBinding;
        CameraBindingImpl& m_cameraBinding;
    };
}
