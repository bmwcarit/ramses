//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/RamsesBindingImpl.h"
#include "internals/SerializationMap.h"
#include "internals/DeserializationMap.h"

#include "ramses-logic/EPropertyType.h"
#include "ramses-logic/EFeatureLevel.h"
#include "ramses-client-api/UniformInput.h"

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
    struct RamsesAppearanceBinding;
}

namespace flatbuffers
{
    class FlatBufferBuilder;
    template <typename T> struct Offset;
}

namespace rlogic::internal
{
    class PropertyImpl;
    class ErrorReporting;
    class IRamsesObjectResolver;

    class RamsesAppearanceBindingImpl : public RamsesBindingImpl
    {
    public:
        explicit RamsesAppearanceBindingImpl(ramses::Appearance& ramsesAppearance, std::string_view name, uint64_t id);

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding> Serialize(
            const RamsesAppearanceBindingImpl& binding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap,
            EFeatureLevel featureLevel);

        [[nodiscard]] static std::unique_ptr<RamsesAppearanceBindingImpl> Deserialize(
            const rlogic_serialization::RamsesAppearanceBinding& appearanceBinding,
            const IRamsesObjectResolver& ramsesResolver,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        [[nodiscard]] ramses::Appearance& getRamsesAppearance() const;

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        std::reference_wrapper<ramses::Appearance> m_ramsesAppearance;
        std::vector<uint32_t> m_uniformIndices;

        void setInputValueToUniform(size_t inputIndex);

        static std::optional<EPropertyType> GetPropertyTypeForUniform(const ramses::UniformInput& uniform);
    };
}
