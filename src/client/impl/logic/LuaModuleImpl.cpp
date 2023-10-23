//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/LuaModuleImpl.h"

#include "ramses/client/logic/LuaModule.h"

#include "internal/logic/LuaCompilationUtils.h"
#include "internal/logic/flatbuffers/generated/LuaModuleGen.h"
#include "flatbuffers/flatbuffers.h"
#include "internal/logic/SolState.h"
#include "impl/ErrorReporting.h"
#include "internal/logic/DeserializationMap.h"
#include "internal/logic/SerializationMap.h"
#include "internal/logic/EnvironmentProtection.h"
#include "internal/logic/PropertyTypeExtractor.h"
#include <fmt/format.h>

namespace ramses::internal
{
    LuaModuleImpl::LuaModuleImpl(SceneImpl& scene, LuaCompiledModule module, std::string_view name, sceneObjectId_t id)
        : LogicObjectImpl{ scene, name, id }
        , m_sourceCode{ std::move(module.source.sourceCode) }
        , m_byteCode{ std::move(module.source.byteCode) }
        , m_module{ std::move(module.moduleTable) }
        , m_dependencies{ std::move(module.source.userModules) }
        , m_stdModules{ std::move(module.source.stdModules) }
        , m_hasDebugLogFunctions{ module.source.hasDebugLogFunctions }
    {
        assert(m_module != sol::lua_nil);
    }

    const sol::table& LuaModuleImpl::getModule() const
    {
        return m_module;
    }

    flatbuffers::Offset<rlogic_serialization::LuaModule> LuaModuleImpl::Serialize(
        const LuaModuleImpl& module,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap,
        ELuaSavingMode luaSavingMode)
    {
        // serialization with debug logs is forbidden
        assert(!module.hasDebugLogFunctions());

        std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>> modulesFB;
        modulesFB.reserve(module.m_dependencies.size());
        for (const auto& dependency : module.m_dependencies)
        {
            modulesFB.push_back(
                rlogic_serialization::CreateLuaModuleUsage(builder,
                    builder.CreateString(dependency.first),
                    dependency.second->getSceneObjectId().getValue()));
        }

        std::vector<uint8_t> stdModules;
        stdModules.reserve(module.m_stdModules.size());
        for (const EStandardModule stdModule : module.m_stdModules)
        {
            stdModules.push_back(static_cast<uint8_t>(stdModule));
        }

        const auto fbLogicObject = LogicObjectImpl::Serialize(module, builder);
        const auto fbModulesVec = builder.CreateVector(modulesFB);
        const auto fbStdModulesVec = builder.CreateVector(stdModules);

        const bool hasSourceCode = !module.m_sourceCode.empty();
        const bool hasByteCode = !module.m_byteCode.empty();
        assert(hasSourceCode || hasByteCode);
        const bool serializeSourceCode = hasSourceCode && !((luaSavingMode == ELuaSavingMode::ByteCodeOnly) && hasByteCode);
        const bool serializeByteCode = hasByteCode && !((luaSavingMode == ELuaSavingMode::SourceCodeOnly) && hasSourceCode);
        assert(serializeSourceCode || serializeByteCode);

        const auto fbSrcCode = (serializeSourceCode ? builder.CreateString(module.m_sourceCode) : 0);

        flatbuffers::Offset<flatbuffers::Vector<uint8_t>> byteCodeOffset{};
        if (serializeByteCode)
        {
            std::string byteCodeString{ module.m_byteCode.as_string_view() };
            byteCodeOffset = serializationMap.resolveByteCodeOffsetIfFound(byteCodeString);
            if (byteCodeOffset.IsNull())
            {
                std::vector<uint8_t> byteCodeAsVectorUInt8;
                byteCodeAsVectorUInt8.reserve(module.m_byteCode.size());
                std::transform(module.m_byteCode.cbegin(), module.m_byteCode.cend(), std::back_inserter(byteCodeAsVectorUInt8), [](std::byte b) { return uint8_t(b); });

                byteCodeOffset = builder.CreateVector(byteCodeAsVectorUInt8);
                serializationMap.storeByteCodeOffset(std::move(byteCodeString), byteCodeOffset);
            }
        }

        return rlogic_serialization::CreateLuaModule(builder,
            fbLogicObject,
            fbSrcCode,
            fbModulesVec,
            fbStdModulesVec,
            byteCodeOffset);
    }

    std::unique_ptr<LuaModuleImpl> LuaModuleImpl::Deserialize(
        SolState& solState,
        const rlogic_serialization::LuaModule& module,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        std::string name;
        sceneObjectId_t id{};
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(module.base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.set("Fatal error during loading of LuaModule from serialized data: missing name and/or ID!", nullptr);
            return nullptr;
        }

        const bool hasSourceCode = (module.source() != nullptr && module.source()->size() > 0);
        const bool hasBytecode = (module.luaByteCode() != nullptr && module.luaByteCode()->size() > 0);
        if (!hasSourceCode && !hasBytecode)
        {
            errorReporting.set("Fatal error during loading of LuaModule from serialized data: has neither Lua source code nor bytecode!", nullptr);
            return nullptr;
        }

        if (!module.dependencies())
        {
            errorReporting.set("Fatal error during loading of LuaModule from serialized data: missing dependencies!", nullptr);
            return nullptr;
        }

        StandardModules stdModules;
        stdModules.reserve(module.standardModules()->size());
        for (const uint8_t stdModule : *module.standardModules())
        {
            stdModules.push_back(static_cast<EStandardModule>(stdModule));
        }

        ModuleMapping modulesUsed;
        modulesUsed.reserve(module.dependencies()->size());
        for (const auto* mod : *module.dependencies())
        {
            if (!mod->name())
            {
                errorReporting.set(fmt::format("Fatal error during loading of LuaModule '{}' module data: missing name!", name), nullptr);
                return nullptr;
            }
            const auto* moduleUsed = deserializationMap.resolveLogicObject<LuaModuleImpl>(sceneObjectId_t{ mod->moduleId() });
            if (!moduleUsed)
            {
                errorReporting.set(fmt::format("Fatal error during loading of LuaModule '{}' module data: could not resolve dependent module with id={}!", name, mod->moduleId()), nullptr);
                return nullptr;
            }

            modulesUsed.emplace(mod->name()->str(), moduleUsed->getLogicObject().as<LuaModule>());
        }

        std::string source = (hasSourceCode ? module.source()->str() : "");
        sol::bytecode byteCode{};
        if (hasBytecode)
        {
            byteCode.reserve(module.luaByteCode()->size());
            std::transform(module.luaByteCode()->cbegin(), module.luaByteCode()->cend(), std::back_inserter(byteCode), [](uint8_t b) { return std::byte(b); });
        }

        auto compiledModule = LuaCompilationUtils::CompileModuleOrImportPrecompiled(solState, modulesUsed, stdModules, std::move(source), name, errorReporting, std::move(byteCode), false);
        if (!compiledModule)
            return nullptr;

        auto deserialized = std::make_unique<LuaModuleImpl>(
            deserializationMap.getScene(),
            std::move(*compiledModule),
            name, id);

        deserialized->setUserId(userIdHigh, userIdLow);

        return deserialized;
    }

    const ModuleMapping& LuaModuleImpl::getDependencies() const
    {
        return m_dependencies;
    }

    bool LuaModuleImpl::hasDebugLogFunctions() const
    {
        return m_hasDebugLogFunctions;
    }
}
