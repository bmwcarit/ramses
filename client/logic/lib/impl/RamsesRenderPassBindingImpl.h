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
    class RenderPass;
}

namespace rlogic_serialization
{
    struct RamsesRenderPassBinding;
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

    class RamsesRenderPassBindingImpl : public RamsesBindingImpl
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
        explicit RamsesRenderPassBindingImpl(ramses::RenderPass& ramsesRenderPass, std::string_view name, uint64_t id);
        ~RamsesRenderPassBindingImpl() noexcept override = default;
        RamsesRenderPassBindingImpl(const RamsesRenderPassBindingImpl& other) = delete;
        RamsesRenderPassBindingImpl& operator=(const RamsesRenderPassBindingImpl& other) = delete;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding> Serialize(
            const RamsesRenderPassBindingImpl& renderPassBinding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap,
            EFeatureLevel featureLevel);

        [[nodiscard]] static std::unique_ptr<RamsesRenderPassBindingImpl> Deserialize(
            const rlogic_serialization::RamsesRenderPassBinding& renderPassBinding,
            const IRamsesObjectResolver& ramsesResolver,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        [[nodiscard]] const ramses::RenderPass& getRamsesRenderPass() const;
        [[nodiscard]] ramses::RenderPass& getRamsesRenderPass();

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        static void ApplyRamsesValuesToInputProperties(RamsesRenderPassBindingImpl& binding, ramses::RenderPass& ramsesRenderPass);

        std::reference_wrapper<ramses::RenderPass> m_ramsesRenderPass;
    };
}
