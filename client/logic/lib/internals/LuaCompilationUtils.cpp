//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/LuaCompilationUtils.h"

#include "ramses-logic/Property.h"
#include "ramses-logic/LuaModule.h"
#include "impl/PropertyImpl.h"
#include "impl/LuaModuleImpl.h"
#include "impl/LoggerImpl.h"
#include "internals/SolState.h"
#include "internals/InterfaceTypeInfo.h"
#include "internals/ErrorReporting.h"
#include "internals/PropertyTypeExtractor.h"
#include "internals/EPropertySemantics.h"
#include "internals/EnvironmentProtection.h"
#include "fmt/format.h"
#include "SolHelper.h"

namespace rlogic::internal
{
    std::optional<LuaCompiledScript> LuaCompilationUtils::CompileScriptOrImportPrecompiled(
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
        bool enableDebugLogFunctions)
    {
        sol::environment env = solState.createEnvironment(stdModules, userModules, enableDebugLogFunctions);
        sol::table internalEnv = EnvironmentProtection::GetProtectedEnvironmentTable(env);

        internalEnv["GLOBAL"] = solState.createTable();

        sol::load_result load_result{};
        sol::protected_function_result main_result{};
        sol::protected_function mainFunction{};
        const std::string debuggingName = (featureLevel == EFeatureLevel_01 ? std::string(name) : "RL_lua_script");

        if (!byteCodeFromPrecompiledScript.empty())
        {
            ScopedEnvironmentProtection p(env, EEnvProtectionFlag::LoadScript);
            main_result = solState.loadScriptByteCode(byteCodeFromPrecompiledScript.as_string_view(), debuggingName, env);
            if (!main_result.valid())
            {
                sol::error error = main_result;
                if (source.empty())
                {
                    errorReporting.add(fmt::format("Fatal error during loading of LuaScript '{}': failed loading pre-compiled byte code and no source available to recompile:\n{}!", name, error.what()),
                        nullptr, EErrorType::BinaryVersionMismatch);
                    return std::nullopt;
                }

                LOG_WARN("Performance warning! Error during loading of LuaScript '{}' from pre-compiled byte code, will try to recompile script from source code. Error:\n{}!", name, error.what());
                byteCodeFromPrecompiledScript.clear();
            }
        }

        if (byteCodeFromPrecompiledScript.empty())
        {
            load_result = solState.loadScript(source, debuggingName);
            if (!load_result.valid())
            {
                sol::error error = load_result;
                errorReporting.add(fmt::format("[{}] Error while loading script. Lua stack trace:\n{}", name, error.what()), nullptr, EErrorType::LuaSyntaxError);
                return std::nullopt;
            }

            if (!CrossCheckDeclaredAndProvidedModules(source, userModules, name, errorReporting))
                return std::nullopt;

            mainFunction = load_result;
            env.set_on(mainFunction);

            {
                ScopedEnvironmentProtection p(env, EEnvProtectionFlag::LoadScript);
                main_result = mainFunction();
            }

            if (!main_result.valid())
            {
                sol::error error = main_result;
                errorReporting.add(error.what(), nullptr, EErrorType::LuaSyntaxError);
                return std::nullopt;
            }
        }

        if (main_result.get_type() != sol::type::none)
        {
            errorReporting.add(fmt::format("[{}] Expected no return value in script source, but a value of type '{}' was returned!",
                name, sol_helper::GetSolTypeName(main_result.get_type())), nullptr, EErrorType::LuaSyntaxError);
            return std::nullopt;
        }

        sol::protected_function init = internalEnv["init"];
        if (init.valid())
        {
            // in order to support interface definitions in globals we need to register the symbols for init section
            sol::protected_function_result initResult {};
            {
                PropertyTypeExtractor::RegisterTypes(internalEnv);
                ScopedEnvironmentProtection p(env, EEnvProtectionFlag::InitFunction);
                initResult = init();
                PropertyTypeExtractor::UnregisterTypes(internalEnv);
            }

            if (!initResult.valid())
            {
                sol::error error = initResult;
                errorReporting.add(fmt::format("[{}] Error while initializing script. Lua stack trace:\n{}", name, error.what()), nullptr, EErrorType::LuaSyntaxError);
                return std::nullopt;
            }
        }

        sol::protected_function run = internalEnv["run"];

        if (!run.valid())
        {
            errorReporting.add(fmt::format("[{}] No 'run' function defined!", name), nullptr, EErrorType::LuaSyntaxError);
            return std::nullopt;
        }

        std::unique_ptr<Property> resultInputs;
        std::unique_ptr<Property> resultOutputs;

        if (inputsFromPrecompiledScript)
        {
            assert(outputsFromPrecompiledScript);
            resultInputs = std::move(inputsFromPrecompiledScript);
            resultOutputs = std::move(outputsFromPrecompiledScript);
        }
        else
        {
            sol::protected_function intf = internalEnv["interface"];
            if (!intf.valid())
            {
                errorReporting.add(fmt::format("[{}] No 'interface' function defined!", name), nullptr, EErrorType::LuaSyntaxError);
                return std::nullopt;
            }

            // Names used only for better error messages!
            PropertyTypeExtractor inputsExtractor("inputs", EPropertyType::Struct);
            PropertyTypeExtractor outputsExtractor("outputs", EPropertyType::Struct);

            sol::environment interfaceEnv = solState.createEnvironment(stdModules, userModules, false);
            sol::table internalInterfaceEnv = EnvironmentProtection::GetProtectedEnvironmentTable(interfaceEnv);
            PropertyTypeExtractor::RegisterTypes(internalInterfaceEnv);
            // Expose globals to interface function
            internalInterfaceEnv["GLOBAL"] = internalEnv["GLOBAL"];

            interfaceEnv.set_on(intf);
            sol::protected_function_result intfResult{};
            {
                ScopedEnvironmentProtection p(interfaceEnv, EEnvProtectionFlag::InterfaceFunctionInScript);
                intfResult = intf(std::ref(inputsExtractor), std::ref(outputsExtractor));
            }

            for (const auto& module : userModules)
                interfaceEnv[module.first] = sol::lua_nil;
            PropertyTypeExtractor::UnregisterTypes(internalInterfaceEnv);

            if (!intfResult.valid())
            {
                sol::error error = intfResult;
                errorReporting.add(fmt::format("[{}] Error while loading script. Lua stack trace:\n{}", name, error.what()), nullptr, EErrorType::LuaSyntaxError);
                return std::nullopt;
            }

            HierarchicalTypeData extractedInputsType = inputsExtractor.getExtractedTypeData();
            HierarchicalTypeData extractedOutputsType = outputsExtractor.getExtractedTypeData();
            // Remove names
            extractedInputsType.typeData.name = "";
            extractedOutputsType.typeData.name = "";

            resultInputs = std::make_unique<Property>(std::make_unique<PropertyImpl>(extractedInputsType, EPropertySemantics::ScriptInput));
            resultOutputs = std::make_unique<Property>(std::make_unique<PropertyImpl>(extractedOutputsType, EPropertySemantics::ScriptOutput));
        }

        sol::bytecode resultByteCode;
        if(featureLevel >= EFeatureLevel_02)
            resultByteCode = (byteCodeFromPrecompiledScript.empty() ? mainFunction.dump() : std::move(byteCodeFromPrecompiledScript));

        EnvironmentProtection::SetEnvironmentProtectionLevel(env, EEnvProtectionFlag::RunFunction);

        return LuaCompiledScript{
            LuaCompiledSource{
                std::move(source),
                std::move(resultByteCode),
                solState,
                stdModules,
                userModules,
                enableDebugLogFunctions
            },
            std::move(run),
            std::move(resultInputs),
            std::move(resultOutputs)
        };
    }

    std::optional<rlogic::internal::LuaCompiledInterface> LuaCompilationUtils::CompileInterface(
        SolState& solState,
        const ModuleMapping& userModules,
        const StandardModules& stdModules,
        bool verifyModules,
        const std::string& source,
        std::string_view name,
        ErrorReporting& errorReporting)
    {
        sol::load_result load_result = solState.loadScript(source, name);
        if (!load_result.valid())
        {
            sol::error error = load_result;
            errorReporting.add(fmt::format("[{}] Error while loading interface. Lua stack trace:\n{}", name, error.what()), nullptr, EErrorType::LuaSyntaxError);
            return std::nullopt;
        }

        if (verifyModules && !CrossCheckDeclaredAndProvidedModules(source, userModules, name, errorReporting))
            return std::nullopt;

        sol::environment env = solState.createEnvironment(stdModules, userModules, false);
        sol::table internalEnv = EnvironmentProtection::GetProtectedEnvironmentTable(env);

        sol::protected_function mainFunction = load_result;
        env.set_on(mainFunction);

        sol::protected_function_result main_result{};
        {
            ScopedEnvironmentProtection p(env, EEnvProtectionFlag::LoadInterface);
            main_result = mainFunction();
        }

        if (!main_result.valid())
        {
            sol::error error = main_result;
            errorReporting.add(error.what(), nullptr, EErrorType::LuaSyntaxError);
            return std::nullopt;
        }

        if (main_result.get_type() != sol::type::none)
        {
            errorReporting.add(fmt::format("[{}] Expected no return value in interface source, but a value of type '{}' was returned!",
                name, sol_helper::GetSolTypeName(main_result.get_type())), nullptr, EErrorType::LuaSyntaxError);
            return std::nullopt;
        }

        sol::protected_function intf = internalEnv["interface"];
        if (!intf.valid())
        {
            errorReporting.add(fmt::format("[{}] No 'interface' function defined!", name), nullptr, EErrorType::LuaSyntaxError);
            return std::nullopt;
        }

        // Name used for better error messages and discarded later
        PropertyTypeExtractor inputsExtractor("inputs", EPropertyType::Struct);

        sol::environment interfaceEnv = solState.createEnvironment(stdModules, userModules, false);
        sol::table internalInterfaceEnv = EnvironmentProtection::GetProtectedEnvironmentTable(interfaceEnv);
        PropertyTypeExtractor::RegisterTypes(internalInterfaceEnv);

        interfaceEnv.set_on(intf);
        sol::protected_function_result interfaceResult{};
        {
            ScopedEnvironmentProtection p(interfaceEnv, EEnvProtectionFlag::InterfaceFunctionInInterface);
            interfaceResult = intf(std::ref(inputsExtractor));
        }

        if (!interfaceResult.valid())
        {
            sol::error error = interfaceResult;
            errorReporting.add(fmt::format("[{}] Error while loading interface. Lua stack trace:\n{}", name, error.what()), nullptr, EErrorType::LuaSyntaxError);
            return std::nullopt;
        }

        PropertyTypeExtractor::UnregisterTypes(internalInterfaceEnv);

        // Discard name
        HierarchicalTypeData extractedInputsType = inputsExtractor.getExtractedTypeData();
        extractedInputsType.typeData.name = "";

        auto rootProperty = std::make_unique<Property>(std::make_unique<PropertyImpl>(extractedInputsType, EPropertySemantics::Interface));

        return LuaCompiledInterface
        {
            std::move(rootProperty)
        };
    }

    std::optional<LuaCompiledModule> LuaCompilationUtils::CompileModuleOrImportPrecompiled(
        SolState& solState,
        const ModuleMapping& userModules,
        const StandardModules& stdModules,
        std::string source,
        std::string_view name,
        ErrorReporting& errorReporting,
        sol::bytecode byteCodeFromPrecompiledModule,
        EFeatureLevel featureLevel,
        bool enableDebugLogFunctions)
    {
        sol::environment env = solState.createEnvironment(stdModules, userModules, enableDebugLogFunctions);
        sol::table internalEnv = EnvironmentProtection::GetProtectedEnvironmentTable(env);
        // interface definitions can be provided within module, in order to be able to extract them
        // when used in LuaScript interface the necessary user types need to be provided
        PropertyTypeExtractor::RegisterTypes(internalEnv);

        sol::load_result load_result{};
        sol::protected_function mainFunction{};
        sol::protected_function_result main_result{};

        const std::string debuggingName = (featureLevel == EFeatureLevel_01 ? std::string(name) : "RL_lua_module");
        if (!byteCodeFromPrecompiledModule.empty())
        {
            ScopedEnvironmentProtection p(env, EEnvProtectionFlag::Module);
            main_result = solState.loadScriptByteCode(byteCodeFromPrecompiledModule.as_string_view(), debuggingName, env);

            if (!main_result.valid())
            {
                sol::error error = main_result;
                if (source.empty())
                {
                    errorReporting.add(fmt::format("Fatal error during loading of LuaModule '{}': failed loading pre-compiled byte code and no source available to recompile:\n{}!", name, error.what()),
                        nullptr, EErrorType::BinaryVersionMismatch);
                    return std::nullopt;
                }

                LOG_WARN("Performance warning! Error during loading of LuaScript '{}' from pre-compiled byte code, will try to recompile script from source code. Error:\n{}!", name, error.what());
                byteCodeFromPrecompiledModule.clear();
            }
        }

        if (byteCodeFromPrecompiledModule.empty())
        {
            load_result = solState.loadScript(source, debuggingName);
            if (!load_result.valid())
            {
                sol::error error = load_result;
                errorReporting.add(fmt::format("[{}] Error while loading module. Lua stack trace:\n{}", name, error.what()), nullptr, EErrorType::LuaSyntaxError);
                return std::nullopt;
            }

            if (!CrossCheckDeclaredAndProvidedModules(source, userModules, name, errorReporting))
                return std::nullopt;

            mainFunction = load_result;
            env.set_on(mainFunction);

            {
                ScopedEnvironmentProtection p(env, EEnvProtectionFlag::Module);
                main_result = mainFunction();
            }

            if (!main_result.valid())
            {
                sol::error error = main_result;
                errorReporting.add(error.what(), nullptr, EErrorType::LuaSyntaxError);
                return std::nullopt;
            }
        }

        PropertyTypeExtractor::UnregisterTypes(internalEnv);

        sol::object resultObj = main_result;
        // TODO Violin check and test for abuse: yield, more than one result
        if (!resultObj.is<sol::table>())
        {
            errorReporting.add(fmt::format("[{}] Error while loading module. Module script must return a table!", name), nullptr, EErrorType::LuaSyntaxError);
            return std::nullopt;
        }

        sol::table moduleTable = resultObj;

        //for serialization
        sol::bytecode resultByteCode;
        if(featureLevel >= EFeatureLevel_02)
            resultByteCode = (byteCodeFromPrecompiledModule.empty() ? mainFunction.dump() : std::move(byteCodeFromPrecompiledModule));

        auto compiledModule = LuaCompiledModule{
            LuaCompiledSource{
                std::move(source),
                std::move(resultByteCode),
                solState,
                stdModules,
                userModules,
                enableDebugLogFunctions
            },
            LuaCompilationUtils::MakeTableReadOnly(solState, moduleTable)
        };

        // Applies environment protection to the module until it's destroyed
        // There is no difference between compilation time and runtime protection rules for modules
        // (the rules are the same)
        EnvironmentProtection::SetEnvironmentProtectionLevel(env, EEnvProtectionFlag::Module);

        return compiledModule;
    }

    // Implements https://www.lua.org/pil/13.4.4.html
    sol::table LuaCompilationUtils::MakeTableReadOnly(SolState& solState, sol::table table)
    {
        auto denyAccess = []() {
            sol_helper::throwSolException("Modifying module data is not allowed!");
        };

        for (auto [childKey, childObject] : table)
        {
            // TODO Violin remove this special case again; currently it's needed because applying read protection
            // to type info objects makes them unusable during interface()
            if (childObject.is<InterfaceTypeInfo>())
            {
                continue;
            }

            if (childObject.is<sol::table>())
            {
                table[childKey] = LuaCompilationUtils::MakeTableReadOnly(solState, childObject);
            }
        }

        // create metatable which denies write access but allows reading
        sol::table metatable = solState.createTable();
        metatable[sol::meta_function::new_index] = denyAccess;
        metatable[sol::meta_function::index] = table;

        // overwrite assigned module with new table and point its metatable to read-only table
        sol::table readOnlyTable = solState.createTable();
        readOnlyTable[sol::metatable_key] = metatable;

        return readOnlyTable;
    }

    bool LuaCompilationUtils::CheckModuleName(std::string_view name)
    {
        if (name.empty())
            return false;

        // Check if any non-alpha-numeric char is contained
        const bool onlyAlphanumChars = std::all_of(name.cbegin(), name.cend(), [](char c) { return c == '_' || (0 != std::isalnum(c)); });
        if (!onlyAlphanumChars)
            return false;

        // First letter is a digit -> error
        if (0 != std::isdigit(name[0]))
            return false;

        return true;
    }

    bool LuaCompilationUtils::CrossCheckDeclaredAndProvidedModules(std::string_view source, const ModuleMapping& modules, std::string_view name, ErrorReporting& errorReporting)
    {
        std::optional<std::vector<std::string>> declaredModules = LuaCompilationUtils::ExtractModuleDependencies(source, errorReporting);
        if (!declaredModules) // failed extraction
            return false;
        if (modules.empty() && declaredModules->empty()) // early out if no modules
            return true;

        std::vector<std::string> providedModules;
        providedModules.reserve(modules.size());
        for (const auto& m : modules)
            providedModules.push_back(m.first);
        std::sort(declaredModules->begin(), declaredModules->end());
        std::sort(providedModules.begin(), providedModules.end());
        if (providedModules != declaredModules)
        {
            std::string errMsg = fmt::format("[{}] Error while loading script/module. Module dependencies declared in source code do not match those provided by LuaConfig.\n", name);
            errMsg += fmt::format("  Module dependencies declared in source code: {}\n", fmt::join(*declaredModules, ", "));
            errMsg += fmt::format("  Module dependencies provided on create API: {}", fmt::join(providedModules, ", "));
            errorReporting.add(errMsg, nullptr, EErrorType::IllegalArgument);
            return false;
        }

        return true;
    }

    std::optional<std::vector<std::string>> LuaCompilationUtils::ExtractModuleDependencies(std::string_view source, ErrorReporting& errorReporting)
    {
        sol::state tempLuaState;

        std::vector<std::string> extractedModules;
        bool success = true;
        int timesCalled = 0;
        tempLuaState.set_function("modules", [&](sol::variadic_args va) {
            ++timesCalled;
            int argIdx = 0;
            for (const auto& v : va)
            {
                if (v.is<std::string>())
                {
                    extractedModules.push_back(v.as<std::string>());
                }
                else
                {
                    const auto argTypeName = sol::type_name(v.lua_state(), v.get_type());
                    errorReporting.add(
                        fmt::format(R"(Error while extracting module dependencies: argument {} is of type '{}', string must be provided: ex. 'modules("moduleA", "moduleB")')",
                        argIdx, argTypeName), nullptr, EErrorType::LuaSyntaxError);
                    success = false;
                }
                ++argIdx;
            }
        });

        const sol::load_result load_result = tempLuaState.load(source, "temp");
        if (!load_result.valid())
        {
            sol::error error = load_result;
            errorReporting.add(fmt::format("Error while extracting module dependencies:\n{}", error.what()), nullptr, EErrorType::LuaSyntaxError);
            return std::nullopt;
        }

        sol::protected_function scriptFunc = load_result;
        const sol::protected_function_result scriptFuncResult = scriptFunc();
        if (!scriptFuncResult.valid())
        {
            const sol::error error = scriptFuncResult;
            LOG_DEBUG("Lua runtime error while extracting module dependencies, this is ignored for the actual extraction but might affect its result:\n{}", error.what());
        }

        if (!success)
            return std::nullopt;

        if (timesCalled > 1)
        {
            errorReporting.add("Error while extracting module dependencies: 'modules' function was executed more than once", nullptr, EErrorType::LuaSyntaxError);
            return std::nullopt;
        }

        auto sortedDependencies = extractedModules;
        std::sort(sortedDependencies.begin(), sortedDependencies.end());
        const auto duplicateIt = std::adjacent_find(sortedDependencies.begin(), sortedDependencies.end());
        if (duplicateIt != sortedDependencies.end())
        {
            errorReporting.add(fmt::format("Error while extracting module dependencies: '{}' appears more than once in dependency list", *duplicateIt), nullptr, EErrorType::LuaSyntaxError);
            return std::nullopt;
        }

        return extractedModules;
    }
}
