//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/RamsesBindingImpl.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/DataTypes.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-framework-api/DataTypes.h"
#include <memory>

namespace rlogic_serialization
{
    struct SkinBinding;
}

namespace flatbuffers
{
    class FlatBufferBuilder;
    template<typename T> struct Offset;
}

namespace rlogic::internal
{
    class RamsesNodeBindingImpl;
    class RamsesAppearanceBindingImpl;
    class ErrorReporting;
    class SerializationMap;
    class DeserializationMap;

    class SkinBindingImpl : public RamsesBindingImpl
    {
    public:
        // Move-able (noexcept); Not copy-able
        explicit SkinBindingImpl(
            std::vector<const RamsesNodeBindingImpl*> joints,
            const std::vector<matrix44f>& inverseBindMatrices,
            RamsesAppearanceBindingImpl& appearanceBinding,
            const ramses::UniformInput& jointMatInput,
            std::string_view name,
            uint64_t id);
        ~SkinBindingImpl() noexcept override = default;
        SkinBindingImpl(const SkinBindingImpl& other) = delete;
        SkinBindingImpl& operator=(const SkinBindingImpl& other) = delete;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::SkinBinding> Serialize(
            const SkinBindingImpl& skinBinding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<SkinBindingImpl> Deserialize(
            const rlogic_serialization::SkinBinding& skinBinding,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        [[nodiscard]] const std::vector<const RamsesNodeBindingImpl*>& getJoints() const;
        [[nodiscard]] const RamsesAppearanceBindingImpl& getAppearanceBinding() const;
        [[nodiscard]] const ramses::UniformInput& getAppearanceUniformInput() const;

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        std::vector<const RamsesNodeBindingImpl*> m_joints;
        std::vector<matrix44f> m_inverseBindMatrices;
        RamsesAppearanceBindingImpl& m_appearanceBinding;
        ramses::UniformInput m_jointMatInput;

        // temp variable used only in update kept as member to avoid reallocs every update call
        std::vector<ramses::matrix44f> m_jointMatricesArray;
    };
}
