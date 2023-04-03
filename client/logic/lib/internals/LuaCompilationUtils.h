//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/LuaConfigImpl.h"
#include "internals/SolWrapper.h"

#include "ramses-logic/EFeatureLevel.h"

#include <string>
#include <memory>
#include <optional>

namespace rlogic
{
    class Property;
    class LuaModule;
}

namespace rlogic::internal
{
    class SolState;
    class ErrorReporting;

    struct LuaCompiledSource
    {
        // Metadata
        std::string sourceCode;
        sol::bytecode byteCode;
        std::reference_wrapper<SolState> solState;
        StandardModules stdModules;
        ModuleMapping userModules;
        bool hasDebugLogFunctions = false;
    };

    struct LuaCompiledScript
    {
        LuaCompiledSource source;

        // The run() function
        sol::protected_function runFunction;

        // Parsed interface properties
        std::unique_ptr<Property> rootInput;
        std::unique_ptr<Property> rootOutput;
    };

    struct LuaCompiledInterface
    {
        std::unique_ptr<Property> rootProperty;
    };

    struct LuaCompiledModule
    {
        LuaCompiledSource source;
        sol::table moduleTable;
    };

    class LuaCompilationUtils
    {
    public:
        [[nodiscard]] static std::optional<LuaCompiledScript> CompileScriptOrImportPrecompiled(
            SolState& solState,
            const ModuleMapping& userModules,
            const StandardModules& stdModules,
            std::string source,
            std::string_view name,
            ErrorReporting& errorReporting,
            sol::bytecode byteCodeFromPrecompiledScript,
            std::unique_ptr<Property> inputsFromPrecompiledScript,
            std::unique_ptr<Property> outputsFromPrecompiledScript,
            EFeatureLevel featureLevel,
            bool enableDebugLogFunctions);

        [[nodiscard]] static std::optional<LuaCompiledInterface> CompileInterface(
            SolState& solState,
            const ModuleMapping& userModules,
            const StandardModules& stdModules,
            bool verifyModules,
            const std::string& source,
            std::string_view name,
            ErrorReporting& errorReporting);

        [[nodiscard]] static std::optional<LuaCompiledModule> CompileModuleOrImportPrecompiled(
            SolState& solState,
            const ModuleMapping& userModules,
            const StandardModules& stdModules,
            std::string source,
            std::string_view name,
            ErrorReporting& errorReporting,
            sol::bytecode byteCodeFromPrecompiledModule,
            EFeatureLevel featureLevel,
            bool enableDebugLogFunctions);

        [[nodiscard]] static bool CheckModuleName(std::string_view name);

        [[nodiscard]] static std::optional<std::vector<std::string>> ExtractModuleDependencies(
            std::string_view source,
            ErrorReporting& errorReporting);

        [[nodiscard]] static sol::table MakeTableReadOnly(SolState& solState, sol::table table);

    private:
        [[nodiscard]] static bool CrossCheckDeclaredAndProvidedModules(
            std::string_view source,
            const ModuleMapping& modules,
            std::string_view chunkname,
            ErrorReporting& errorReporting);
    };
}
