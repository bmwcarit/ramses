//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/logic/RamsesBindingImpl.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/UniformInput.h"
#include "ramses/framework/DataTypes.h"
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

namespace ramses::internal
{
    class NodeBindingImpl;
    class AppearanceBindingImpl;
    class ErrorReporting;
    class SerializationMap;
    class DeserializationMap;

    class SkinBindingImpl : public RamsesBindingImpl
    {
    public:
        // Move-able (noexcept); Not copy-able
        explicit SkinBindingImpl(
            SceneImpl& scene,
            std::vector<const NodeBindingImpl*> joints,
            const std::vector<matrix44f>& inverseBindMatrices,
            AppearanceBindingImpl& appearanceBinding,
            const ramses::UniformInput& jointMatInput,
            std::string_view name,
            sceneObjectId_t id);
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

        [[nodiscard]] const std::vector<const NodeBindingImpl*>& getJoints() const;
        [[nodiscard]] const AppearanceBindingImpl& getAppearanceBinding() const;
        [[nodiscard]] const ramses::UniformInput& getAppearanceUniformInput() const;

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        std::vector<const NodeBindingImpl*> m_joints;
        std::vector<matrix44f> m_inverseBindMatrices;
        AppearanceBindingImpl& m_appearanceBinding;
        ramses::UniformInput m_jointMatInput;

        // temp variable used only in update kept as member to avoid reallocs every update call
        std::vector<ramses::matrix44f> m_jointMatricesArray;
    };
}
