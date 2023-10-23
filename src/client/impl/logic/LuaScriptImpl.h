//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/logic/LogicNodeImpl.h"

#include "internal/logic/LuaCompilationUtils.h"
#include "internal/logic/DeserializationMap.h"
#include "internal/logic/WrappedLuaProperty.h"

#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/ELuaSavingMode.h"

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
    struct LuaScript;
}

namespace ramses
{
    class LuaModule;
}

namespace ramses::internal
{
    class SolState;
    class SerializationMap;

    class LuaScriptImpl : public LogicNodeImpl
    {
    public:
        explicit LuaScriptImpl(SceneImpl& scene, LuaCompiledScript compiledScript, std::string_view name, sceneObjectId_t id);
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
            DeserializationMap& deserializationMap);

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
