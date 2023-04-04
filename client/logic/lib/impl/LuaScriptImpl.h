//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/LogicNodeImpl.h"

#include "internals/LuaCompilationUtils.h"
#include "internals/DeserializationMap.h"
#include "internals/WrappedLuaProperty.h"

#include "ramses-logic/Property.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/EFeatureLevel.h"
#include "ramses-logic/ELuaSavingMode.h"

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
    struct LuaScript;
}

namespace rlogic
{
    class LuaModule;
}

namespace rlogic::internal
{
    class SolState;
    class SerializationMap;

    class LuaScriptImpl : public LogicNodeImpl
    {
    public:
        explicit LuaScriptImpl(LuaCompiledScript compiledScript, std::string_view name, uint64_t id);
        ~LuaScriptImpl() noexcept override = default;
        LuaScriptImpl(const LuaScriptImpl & other) = delete;
        LuaScriptImpl& operator=(const LuaScriptImpl & other) = delete;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::LuaScript> Serialize(
            const LuaScriptImpl& luaScript,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap,
            ELuaSavingMode luaSavingMode);

        [[nodiscard]] static std::unique_ptr<LuaScriptImpl> Deserialize(
            SolState& solState,
            const rlogic_serialization::LuaScript& luaScript,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap,
            EFeatureLevel featureLevel);

        std::optional<LogicNodeRuntimeError> update() override;

        [[nodiscard]] const ModuleMapping& getModules() const;
        [[nodiscard]] bool hasDebugLogFunctions() const;

        void createRootProperties() final;

    private:
        std::string             m_source;
        sol::bytecode           m_byteCode;
        WrappedLuaProperty      m_wrappedRootInput;
        WrappedLuaProperty      m_wrappedRootOutput;
        sol::protected_function m_runFunction;
        ModuleMapping           m_modules;
        StandardModules         m_stdModules;
        bool m_hasDebugLogFunctions;
    };
}
