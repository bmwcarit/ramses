//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/LuaScriptImpl.h"

#include "ramses-logic/LuaModule.h"
#include "internals/SolState.h"
#include "impl/PropertyImpl.h"
#include "impl/LuaModuleImpl.h"

#include "internals/WrappedLuaProperty.h"
#include "internals/SolHelper.h"
#include "internals/ErrorReporting.h"
#include "internals/PropertyTypeExtractor.h"
#include "internals/EnvironmentProtection.h"
#include "internals/SerializationMap.h"

#include "generated/LuaScriptGen.h"

#include <iostream>

namespace rlogic::internal
{
    LuaScriptImpl::LuaScriptImpl(LuaCompiledScript compiledScript, std::string_view name, uint64_t id)
        : LogicNodeImpl(name, id)
        , m_source(std::move(compiledScript.source.sourceCode))
        , m_byteCode(std::move(compiledScript.source.byteCode))
        , m_wrappedRootInput(*compiledScript.rootInput->m_impl)
        , m_wrappedRootOutput(*compiledScript.rootOutput->m_impl)
        , m_runFunction(std::move(compiledScript.runFunction))
        , m_modules(std::move(compiledScript.source.userModules))
        , m_stdModules(std::move(compiledScript.source.stdModules))
        , m_hasDebugLogFunctions{ compiledScript.source.hasDebugLogFunctions }
    {
        setRootProperties(std::move(compiledScript.rootInput), std::move(compiledScript.rootOutput));
    }

    void LuaScriptImpl::createRootProperties()
    {
        // unlike other logic objects, luascript properties created outside of it (from script or deserialized)
    }

    flatbuffers::Offset<rlogic_serialization::LuaScript> LuaScriptImpl::Serialize(
        const LuaScriptImpl& luaScript,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap,
        ELuaSavingMode luaSavingMode)
    {
        // serialization with debug logs is forbidden
        assert(!luaScript.hasDebugLogFunctions());

        std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>> userModules;
        userModules.reserve(luaScript.m_modules.size());
        for (const auto& module : luaScript.m_modules)
        {
            userModules.push_back(
                rlogic_serialization::CreateLuaModuleUsage(builder,
                    builder.CreateString(module.first),
                    module.second->getId()));
        }

        std::vector<uint8_t> stdModules;
        stdModules.reserve(luaScript.m_stdModules.size());
        for (const EStandardModule stdModule : luaScript.m_stdModules)
        {
            stdModules.push_back(static_cast<uint8_t>(stdModule));
        }

        const auto fbLogicObject = LogicObjectImpl::Serialize(luaScript, builder);
        const auto fbModulesVec = builder.CreateVector(userModules);
        const auto fbStdModulesVec = builder.CreateVector(stdModules);
        const auto fbInputPropertyObject = PropertyImpl::Serialize(*luaScript.getInputs()->m_impl, builder, serializationMap);
        const auto fbOuputPropertyObject = PropertyImpl::Serialize(*luaScript.getOutputs()->m_impl, builder, serializationMap);

        const bool hasSourceCode = !luaScript.m_source.empty();
        const bool hasByteCode = !luaScript.m_byteCode.empty();
        assert(hasSourceCode || hasByteCode);
        const bool serializeSourceCode = hasSourceCode && !((luaSavingMode == ELuaSavingMode::ByteCodeOnly) && hasByteCode);
        const bool serializeByteCode = hasByteCode && !((luaSavingMode == ELuaSavingMode::SourceCodeOnly) && hasSourceCode);
        assert(serializeSourceCode || serializeByteCode);

        const auto fbSrcCode = (serializeSourceCode ? builder.CreateString(luaScript.m_source) : 0);

        flatbuffers::Offset<flatbuffers::Vector<uint8_t>> byteCodeOffset{};
        if (serializeByteCode)
        {
            std::string byteCodeString{ luaScript.m_byteCode.as_string_view() };
            byteCodeOffset = serializationMap.resolveByteCodeOffsetIfFound(byteCodeString);
            if (byteCodeOffset.IsNull())
            {
                std::vector<uint8_t> byteCodeAsVectorUInt8;
                byteCodeAsVectorUInt8.reserve(luaScript.m_byteCode.size());
                std::transform(luaScript.m_byteCode.cbegin(), luaScript.m_byteCode.cend(), std::back_inserter(byteCodeAsVectorUInt8), [](std::byte b) { return uint8_t(b); });

                byteCodeOffset = builder.CreateVector(byteCodeAsVectorUInt8);
                serializationMap.storeByteCodeOffset(std::move(byteCodeString), byteCodeOffset);
            }
        }

        auto script = rlogic_serialization::CreateLuaScript(builder,
            fbLogicObject,
            fbSrcCode,
            fbModulesVec,
            fbStdModulesVec,
            fbInputPropertyObject,
            fbOuputPropertyObject,
            byteCodeOffset
        );
        builder.Finish(script);

        return script;
    }

    std::unique_ptr<LuaScriptImpl> LuaScriptImpl::Deserialize(
        SolState& solState,
        const rlogic_serialization::LuaScript& luaScript,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap,
        EFeatureLevel featureLevel)
    {
        std::string name;
        uint64_t id = 0u;
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(luaScript.base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.add("Fatal error during loading of LuaScript from serialized data: missing name and/or ID!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const bool hasSourceCode = (luaScript.luaSourceCode() != nullptr && luaScript.luaSourceCode()->size() > 0);
        const bool hasBytecode = (luaScript.luaByteCode() != nullptr && luaScript.luaByteCode()->size() > 0);
        if (!hasSourceCode && !hasBytecode)
        {
            errorReporting.add("Fatal error during loading of LuaScript from serialized data: has neither Lua source code nor bytecode!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!luaScript.rootInput())
        {
            errorReporting.add("Fatal error during loading of LuaScript from serialized data: missing root input!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> rootInput = PropertyImpl::Deserialize(*luaScript.rootInput(), EPropertySemantics::ScriptInput, errorReporting, deserializationMap);
        if (!rootInput)
        {
            return nullptr;
        }

        if (!luaScript.rootOutput())
        {
            errorReporting.add("Fatal error during loading of LuaScript from serialized data: missing root output!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> rootOutput = PropertyImpl::Deserialize(*luaScript.rootOutput(), EPropertySemantics::ScriptOutput, errorReporting, deserializationMap);
        if (!rootOutput)
        {
            return nullptr;
        }

        if (rootInput->getType() != EPropertyType::Struct)
        {
            errorReporting.add("Fatal error during loading of LuaScript from serialized data: root input has unexpected type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (rootOutput->getType() != EPropertyType::Struct)
        {
            errorReporting.add("Fatal error during loading of LuaScript from serialized data: root output has unexpected type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!luaScript.userModules())
        {
            errorReporting.add("Fatal error during loading of LuaScript from serialized data: missing user module dependencies!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }
        ModuleMapping userModules;
        userModules.reserve(luaScript.userModules()->size());
        for (const auto* module : *luaScript.userModules())
        {
            if (!module->name())
            {
                errorReporting.add(fmt::format("Fatal error during loading of LuaScript '{}' module data: missing name!", name), nullptr, EErrorType::BinaryVersionMismatch);
                return nullptr;
            }
            const auto* moduleUsed = deserializationMap.resolveLogicObject<LuaModuleImpl>(module->moduleId());
            if (!moduleUsed)
            {
                errorReporting.add(fmt::format("Fatal error during loading of LuaScript '{}' module data: could not resolve dependent module with id={}!", name, module->moduleId()), nullptr, EErrorType::BinaryVersionMismatch);
                return nullptr;
            }

            userModules.emplace(module->name()->str(), moduleUsed->getLogicObject().as<LuaModule>());
        }

        if (!luaScript.standardModules())
        {
            errorReporting.add("Fatal error during loading of LuaScript from serialized data: missing standard module dependencies!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }
        StandardModules stdModules;
        stdModules.reserve(luaScript.standardModules()->size());
        for (const uint8_t stdModule : *luaScript.standardModules())
            stdModules.push_back(static_cast<EStandardModule>(stdModule));

        auto inputs = std::make_unique<Property>(std::move(rootInput));
        auto outputs = std::make_unique<Property>(std::move(rootOutput));

        std::string sourceCode = (hasSourceCode ? luaScript.luaSourceCode()->str() : "");
        sol::bytecode byteCode;
        if (hasBytecode)
        {
            byteCode.reserve(luaScript.luaByteCode()->size());
            std::transform(luaScript.luaByteCode()->cbegin(), luaScript.luaByteCode()->cend(), std::back_inserter(byteCode), [](uint8_t b) { return std::byte(b); });
        }

        auto compiledScript = LuaCompilationUtils::CompileScriptOrImportPrecompiled(
            solState,
            userModules,
            stdModules,
            std::move(sourceCode),
            name,
            errorReporting,
            std::move(byteCode),
            std::move(inputs),
            std::move(outputs),
            featureLevel,
            false);

        if (!compiledScript)
        {
            errorReporting.add(fmt::format("Fatal error during loading of LuaScript '{}' from serialized data!", name), nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        auto deserialized = std::make_unique<LuaScriptImpl>(
            std::move(*compiledScript),
            name, id);

        deserialized->setUserId(userIdHigh, userIdLow);

        return deserialized;
    }

    std::optional<LogicNodeRuntimeError> LuaScriptImpl::update()
    {
        sol::protected_function_result result = m_runFunction(std::ref(m_wrappedRootInput), std::ref(m_wrappedRootOutput));

        if (!result.valid())
        {
            sol::error error = result;
            return LogicNodeRuntimeError{error.what()};
        }

        return std::nullopt;
    }

    const ModuleMapping& LuaScriptImpl::getModules() const
    {
        return m_modules;
    }

    bool LuaScriptImpl::hasDebugLogFunctions() const
    {
        return m_hasDebugLogFunctions;
    }
}
