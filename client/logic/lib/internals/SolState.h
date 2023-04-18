//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/LuaConfigImpl.h"
#include "internals/SolWrapper.h"

#include <string_view>
#include <utility>

namespace rlogic::internal
{
    constexpr std::array<rlogic::EStandardModule, 5> StdModules = {
        rlogic::EStandardModule::Base,
        rlogic::EStandardModule::String,
        rlogic::EStandardModule::Table,
        rlogic::EStandardModule::Math,
        rlogic::EStandardModule::Debug
    };
    constexpr std::array<sol::lib, 5> SolLibs = {
        sol::lib::base,
        sol::lib::string,
        sol::lib::table,
        sol::lib::math,
        sol::lib::debug
    };

    static_assert(StdModules.size() == SolLibs.size());

    class SolState
    {
    public:
        SolState();

        // Move-able (noexcept); Not copy-able
        ~SolState() noexcept = default;
        // Not move-able because of the dependency of sol environments to the
        // underlying sol state. Enabling move would require to write a custom
        // move code which first moves the dependent objects, then the sol state
        // (in inverse order of creation)
        SolState(SolState&& other) noexcept = delete;
        SolState& operator=(SolState&& other) noexcept = delete;
        SolState(const SolState& other) = delete;
        SolState& operator=(const SolState& other) = delete;

        sol::load_result loadScript(std::string_view source, std::string_view scriptName);
        sol::protected_function_result loadScriptByteCode(std::string_view byteCode, std::string_view scriptName, sol::environment& env);
        sol::environment createEnvironment(const StandardModules& stdModules, const ModuleMapping& userModules, bool exposeDebugLogFunctions);
        void copyTableIntoEnvironment(const sol::table& table, std::string_view name, sol::environment& env);
        sol::table createTable();

        [[nodiscard]] int getNumElementsInLuaStack() const;

        [[nodiscard]] static bool IsReservedModuleName(std::string_view name);

    private:
        sol::state m_solState;
        // Cached to avoid unnecessary heap allocations
        std::vector<std::string> m_safeBaselibSymbols;

        void mapStandardModules(const StandardModules& stdModules, sol::environment& env);
        [[nodiscard]] static std::optional<std::string_view> GetStdModuleName(rlogic::EStandardModule m);
    };
}
