//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/SolState.h"

#include "ramses-logic/LuaModule.h"
#include "impl/LuaModuleImpl.h"

#include "internals/LuaCustomizations.h"
#include "internals/WrappedLuaProperty.h"
#include "internals/SolHelper.h"
#include "internals/LuaTypeConversions.h"
#include "internals/EnvironmentProtection.h"

#include <iostream>

namespace rlogic::internal
{
    // NOLINTNEXTLINE(performance-unnecessary-value-param) The signature is forced by SOL. Therefore we have to disable this warning.
    static int solExceptionHandler(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description)
    {
        if (maybe_exception)
        {
            return sol::stack::push(L, description);
        }
        return sol::stack::top(L);
    }

    SolState::SolState()
    {
        m_safeBaselibSymbols = {
            "assert",
            "error",
            "ipairs",
            "next",
            "pairs",
            "print",
            "select",
            "tonumber",
            "tostring",
            "type",
            "unpack",
            "pcall",
            "xpcall",
            "_VERSION",

            // Potentially less safe, but allows for advanced Lua use cases
            "rawequal",
            "rawget",
            "rawset",
            "setmetatable",
            "getmetatable",
        };

        // TODO Violin try to load standard modules on demand
        for (auto solLib : SolLibs)
        {
            m_solState.open_libraries(solLib);
        }

        m_solState.set_exception_handler(&solExceptionHandler);

        // TODO Violin only register wrappers to runtime environments, not in the global environment
        WrappedLuaProperty::RegisterTypes(m_solState);

        LuaCustomizations::RegisterTypes(m_solState);
    }

    sol::load_result SolState::loadScript(std::string_view source, std::string_view scriptName)
    {
        return m_solState.load(source, std::string(scriptName));
    }

    sol::protected_function_result SolState::loadScriptByteCode(std::string_view byteCode, std::string_view scriptName, sol::environment& env)
    {
        return m_solState.safe_script(byteCode, env, sol::script_pass_on_error, std::string(scriptName));
    }

    sol::environment SolState::createEnvironment(const StandardModules& stdModules, const ModuleMapping& userModules, bool exposeDebugLogFunctions)
    {
        sol::environment protectedEnv(m_solState, sol::create);

        protectedEnv["_G"] = protectedEnv;

        mapStandardModules(stdModules, protectedEnv);

        for (const auto& module : userModules)
        {
            assert(!SolState::IsReservedModuleName(module.first));
            protectedEnv[module.first] = module.second->m_impl.getModule();
        }

        // TODO Violin take a closer look at this, should not be needed
        // if using 'modules' call to declare dependencies - resolve it to noop
        auto dummyFunc = [](const std::string& /*unused*/) {};
        protectedEnv["modules"] = dummyFunc;

        LuaCustomizations::MapToEnvironment(m_solState, protectedEnv);

        if (exposeDebugLogFunctions)
            LuaCustomizations::MapDebugLogFunctions(protectedEnv);

        EnvironmentProtection::AddProtectedEnvironmentTable(protectedEnv, m_solState);

        return protectedEnv;
    }

    void SolState::mapStandardModules(const StandardModules& stdModules, sol::environment& env)
    {
        // Copy base libraries' tables into environment to sandbox them, so that scripts cannot affect each other by modifying them
        for (const auto& stdModule : stdModules)
        {
            // The base module needs special handling because it's not in a named table
            if (stdModule == EStandardModule::Base)
            {

                for (const auto& name : m_safeBaselibSymbols)
                {
                    env[name] = m_solState[name];
                }

            }
            else
            {
                std::string_view moduleTableName = *GetStdModuleName(stdModule);
                const sol::table& moduleAsTable = m_solState[moduleTableName];
                copyTableIntoEnvironment(moduleAsTable, moduleTableName, env);
            }
        }
    }

    void SolState::copyTableIntoEnvironment(const sol::table& table, std::string_view name, sol::environment& env)
    {
        sol::table copy(m_solState, sol::create);
        for (const auto& pair : table)
        {
            // first is the name of a function in module, second is the function
            copy[pair.first] = pair.second;
        }
        env[name] = std::move(copy);
    }

    std::optional<std::string_view> SolState::GetStdModuleName(rlogic::EStandardModule m)
    {
        switch (m)
        {
        case EStandardModule::Base:
            // Standard module has no name because it's not loaded in a table but global space
            return std::nullopt;
        case EStandardModule::String:
            return "string";
        case EStandardModule::Table:
            return "table";
        case EStandardModule::Math:
            return "math";
        case EStandardModule::Debug:
            return "debug";
        case EStandardModule::All:
            return std::nullopt;
        }
        return std::nullopt;
    }

    bool SolState::IsReservedModuleName(std::string_view name)
    {
        for (auto m : StdModules)
        {
            std::optional<std::string_view> potentialName = GetStdModuleName(m);
            if (potentialName && *potentialName == name)
            {
                return true;
            }
        }

        return false;
    }

    sol::table SolState::createTable()
    {
        return m_solState.create_table();
    }

    int SolState::getNumElementsInLuaStack() const
    {
        return lua_gettop(m_solState.lua_state());
    }
}
