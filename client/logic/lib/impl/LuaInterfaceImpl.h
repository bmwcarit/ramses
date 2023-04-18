//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/LogicNodeImpl.h"

#include "internals/LuaCompilationUtils.h"
#include "internals/SerializationMap.h"
#include "internals/DeserializationMap.h"
#include "internals/WrappedLuaProperty.h"

#include "ramses-logic/Property.h"
#include "ramses-logic/LuaInterface.h"

#include <memory>
#include <functional>
#include <string_view>

namespace flatbuffers
{
    class FlatBufferBuilder;

    class FlatBufferBuilder;
    template <typename T> struct Offset;
}

namespace rlogic_serialization
{
    struct LuaInterface;
}

namespace rlogic::internal
{
    class SolState;

    class LuaInterfaceImpl : public LogicNodeImpl
    {
    public:
        explicit LuaInterfaceImpl(LuaCompiledInterface compiledInterface, std::string_view name, uint64_t id);
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
