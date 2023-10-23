//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/logic/RamsesBindingImpl.h"
#include "internal/logic/SerializationMap.h"
#include "internal/logic/DeserializationMap.h"

#include "ramses/client/logic/EPropertyType.h"
#include "ramses/client/UniformInput.h"

#include <optional>
#include <memory>
#include <unordered_map>

namespace ramses
{
    class Appearance;
    class UniformInput;
}

namespace rlogic_serialization
{
    struct AppearanceBinding;
}

namespace flatbuffers
{
    template <typename T> struct Offset;
    template<bool> class FlatBufferBuilderImpl;
    using FlatBufferBuilder = FlatBufferBuilderImpl<false>;
}

namespace ramses::internal
{
    class PropertyImpl;
    class ErrorReporting;
    class IRamsesObjectResolver;

    class AppearanceBindingImpl : public RamsesBindingImpl
    {
    public:
        explicit AppearanceBindingImpl(SceneImpl& scene, ramses::Appearance& ramsesAppearance, std::string_view name, sceneObjectId_t id);

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::AppearanceBinding> Serialize(
            const AppearanceBindingImpl& binding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<AppearanceBindingImpl> Deserialize(
            const rlogic_serialization::AppearanceBinding& appearanceBinding,
            const IRamsesObjectResolver& ramsesResolver,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        [[nodiscard]] ramses::Appearance& getRamsesAppearance() const;

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        std::reference_wrapper<ramses::Appearance> m_ramsesAppearance;
        std::vector<ramses::UniformInput> m_uniforms;

        void setInputValueToUniform(size_t inputIndex);

        static std::optional<EPropertyType> GetPropertyTypeForUniform(const ramses::UniformInput& uniform);
    };
}
