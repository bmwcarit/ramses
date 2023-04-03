//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/RamsesBindingImpl.h"
#include "impl/RamsesRenderGroupBindingElementsImpl.h"
#include "ramses-logic/EFeatureLevel.h"
#include <memory>

namespace ramses
{
    class RenderGroup;
}

namespace rlogic_serialization
{
    struct RamsesRenderGroupBinding;
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

    class RamsesRenderGroupBindingImpl : public RamsesBindingImpl
    {
    public:
        // Move-able (noexcept); Not copy-able
        explicit RamsesRenderGroupBindingImpl(ramses::RenderGroup& ramsesRenderGroup, const RamsesRenderGroupBindingElementsImpl& elements, std::string_view name, uint64_t id);
        ~RamsesRenderGroupBindingImpl() noexcept override = default;
        RamsesRenderGroupBindingImpl(const RamsesRenderGroupBindingImpl& other) = delete;
        RamsesRenderGroupBindingImpl& operator=(const RamsesRenderGroupBindingImpl& other) = delete;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding> Serialize(
            const RamsesRenderGroupBindingImpl& renderGroupBinding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap,
            EFeatureLevel featureLevel);

        [[nodiscard]] static std::unique_ptr<RamsesRenderGroupBindingImpl> Deserialize(
            const rlogic_serialization::RamsesRenderGroupBinding& renderGroupBinding,
            const IRamsesObjectResolver& ramsesResolver,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        [[nodiscard]] const ramses::RenderGroup& getRamsesRenderGroup() const;
        [[nodiscard]] ramses::RenderGroup& getRamsesRenderGroup();

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        static void ApplyRamsesValuesToInputProperties(RamsesRenderGroupBindingImpl& binding, ramses::RenderGroup& ramsesRenderGroup);

        std::reference_wrapper<ramses::RenderGroup> m_ramsesRenderGroup;
        RamsesRenderGroupBindingElementsImpl::Elements m_elements;
    };
}
