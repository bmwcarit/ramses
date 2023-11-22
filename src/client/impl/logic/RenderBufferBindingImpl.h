//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
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
    class RenderBuffer;
}

namespace rlogic_serialization
{
    struct RenderBufferBinding;
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

    class RenderBufferBindingImpl : public RamsesBindingImpl
    {
    public:
        explicit RenderBufferBindingImpl(SceneImpl& scene, ramses::RenderBuffer& renderBuffer, std::string_view name, sceneObjectId_t id);
        ~RenderBufferBindingImpl() noexcept override = default;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::RenderBufferBinding> Serialize(
            const RenderBufferBindingImpl& renderBufferBinding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<RenderBufferBindingImpl> Deserialize(
            const rlogic_serialization::RenderBufferBinding& renderBufferBinding,
            const IRamsesObjectResolver& ramsesResolver,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        [[nodiscard]] const ramses::RenderBuffer& getRenderBuffer() const;
        [[nodiscard]] ramses::RenderBuffer& getRenderBuffer();

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

        enum class EInputProperty
        {
            Width = 0,
            Height,
            SampleCount,

            COUNT
        };

    private:
        static void ApplyValuesToInputProperties(RenderBufferBindingImpl& binding, ramses::RenderBuffer& renderBuffer);

        std::reference_wrapper<ramses::RenderBuffer> m_renderBuffer;
    };
}
