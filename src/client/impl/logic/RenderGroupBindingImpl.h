//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/logic/RamsesBindingImpl.h"
#include "impl/logic/RenderGroupBindingElementsImpl.h"
#include <memory>

namespace ramses
{
    class RenderGroup;
}

namespace rlogic_serialization
{
    struct RenderGroupBinding;
}

namespace flatbuffers
{
    class FlatBufferBuilder;
    template<typename T> struct Offset;
}

namespace ramses::internal
{
    class IRamsesObjectResolver;
    class ErrorReporting;
    class SerializationMap;
    class DeserializationMap;

    class RenderGroupBindingImpl : public RamsesBindingImpl
    {
    public:
        // Move-able (noexcept); Not copy-able
        explicit RenderGroupBindingImpl(SceneImpl& scene, ramses::RenderGroup& ramsesRenderGroup, const RenderGroupBindingElementsImpl& elements, std::string_view name, sceneObjectId_t id);
        ~RenderGroupBindingImpl() noexcept override = default;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::RenderGroupBinding> Serialize(
            const RenderGroupBindingImpl& renderGroupBinding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<RenderGroupBindingImpl> Deserialize(
            const rlogic_serialization::RenderGroupBinding& renderGroupBinding,
            const IRamsesObjectResolver& ramsesResolver,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        [[nodiscard]] const ramses::RenderGroup& getRamsesRenderGroup() const;
        [[nodiscard]] ramses::RenderGroup& getRamsesRenderGroup();

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        static void ApplyRamsesValuesToInputProperties(RenderGroupBindingImpl& binding, ramses::RenderGroup& ramsesRenderGroup);

        std::reference_wrapper<ramses::RenderGroup> m_ramsesRenderGroup;
        RenderGroupBindingElementsImpl::Elements m_elements;
    };
}
