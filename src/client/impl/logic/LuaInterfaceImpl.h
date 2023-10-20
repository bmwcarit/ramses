//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/logic/LogicNodeImpl.h"

#include "internal/logic/LuaCompilationUtils.h"
#include "internal/logic/SerializationMap.h"
#include "internal/logic/DeserializationMap.h"
#include "internal/logic/WrappedLuaProperty.h"

#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/LuaInterface.h"

#include <memory>
#include <functional>
#include <string_view>

namespace flatbuffers
{
    template <typename T> struct Offset;
    template<bool> class FlatBufferBuilderImpl;
    using FlatBufferBuilder = FlatBufferBuilderImpl<false>;
}

namespace rlogic_serialization
{
    struct LuaInterface;
}

namespace ramses::internal
{
    class SolState;

    class LuaInterfaceImpl : public LogicNodeImpl
    {
    public:
        explicit LuaInterfaceImpl(SceneImpl& scene, LuaCompiledInterface compiledInterface, std::string_view name, sceneObjectId_t id);
        ~LuaInterfaceImpl() noexcept override = default;
        LuaInterfaceImpl(const LuaInterfaceImpl & other) = delete;
        LuaInterfaceImpl& operator=(const LuaInterfaceImpl & other) = delete;

        [[nodiscard]] Property* getOutputs() override;
        [[nodiscard]] const Property* getOutputs() const override;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::LuaInterface> Serialize(
            const LuaInterfaceImpl& luaInterface,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<LuaInterfaceImpl> Deserialize(
            const rlogic_serialization::LuaInterface& luaInterface,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        std::optional<LogicNodeRuntimeError> update() override;
        void createRootProperties() final;

        [[nodiscard]] std::vector<const Property*> collectUnlinkedProperties() const;
    };
}
