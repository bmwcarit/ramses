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
    class RenderPass;
}

namespace rlogic_serialization
{
    struct RenderPassBinding;
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

    class RenderPassBindingImpl : public RamsesBindingImpl
    {
    public:
        enum EPropertyIndex
        {
            EPropertyIndex_Enabled = 0,
            EPropertyIndex_RenderOrder,
            EPropertyIndex_ClearColor,
            EPropertyIndex_RenderOnce,

            EPropertyIndex_COUNT
        };

        // Move-able (noexcept); Not copy-able
        explicit RenderPassBindingImpl(SceneImpl& scene, ramses::RenderPass& ramsesRenderPass, std::string_view name, sceneObjectId_t id);
        ~RenderPassBindingImpl() noexcept override = default;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::RenderPassBinding> Serialize(
            const RenderPassBindingImpl& renderPassBinding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<RenderPassBindingImpl> Deserialize(
            const rlogic_serialization::RenderPassBinding& renderPassBinding,
            const IRamsesObjectResolver& ramsesResolver,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        [[nodiscard]] const ramses::RenderPass& getRamsesRenderPass() const;
        [[nodiscard]] ramses::RenderPass& getRamsesRenderPass();

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        static void ApplyRamsesValuesToInputProperties(RenderPassBindingImpl& binding, ramses::RenderPass& ramsesRenderPass);

        std::reference_wrapper<ramses::RenderPass> m_ramsesRenderPass;
    };
}
