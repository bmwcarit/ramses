//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include "ramses-logic/EStandardModule.h"

#include <string>
#include <memory>

namespace ramses::internal
{
    class LuaConfigImpl;
}

namespace ramses
{
    class LuaModule;

    /**
     * Holds configuration settings for Lua script and module creation. Can be default-constructed, moved
     * and copied.
     */
    class LuaConfig
    {
    public:
        RAMSES_API LuaConfig() noexcept;

        /**
         * Adds a #ramses::LuaModule as a dependency to be added when this config is used for script or module
         * creation. The \p aliasName can be any valid Lua label which must obey following rules:
         * - can't use the same label twice in the same #LuaConfig object
         * - can't use standard module names (math, string etc.)
         *
         * The \p moduleInstance provided can be any module. You can't reference modules from
         * different #ramses::LogicEngine instances and the referenced modules must be from the same instance
         * on which the config is used for script creation.
         *
         * @param aliasName the alias name under which the dependency will be mapped into the parent script/module
         * @param moduleInstance the dependency module to map
         * @return true if the dependency was added successfully, false otherwise
         * In case of an error, check the logs.
         */
        RAMSES_API bool addDependency(std::string_view aliasName, const LuaModule& moduleInstance);

        /**
         * Adds a standard module dependency. The module is mapped under a name as documented in #ramses::EStandardModule.
         *
         * @param stdModule the standard module which will be mapped into the parent script/module
         * @return true if the standard module was added successfully, false otherwise
         * In case of an error, check the logs.
         */
        RAMSES_API bool addStandardModuleDependency(EStandardModule stdModule);

        /**
         * Will expose these log functions in Lua:
         *   rl_logInfo
         *   rl_logWarn
         *   rl_logError
         * Each with single string as argument.
         * Call from Lua script or module to any of these will forward the string message to a corresponding internal Ramses Logic logger.
         *
         * IMPORTANT: These functions are meant for debug/prototype purposes only and attempt to save the project to file (#ramses::LogicEngine::saveToFile)
         * with any script or module with these functions enabled will result in failure!
         */
        RAMSES_API void enableDebugLogFunctions();

        /**
         * Destructor of #LuaConfig
         */
        RAMSES_API ~LuaConfig() noexcept;

        /**
         * Copy Constructor of #LuaConfig
         * @param other the other #LuaConfig to copy from
         */
        RAMSES_API LuaConfig(const LuaConfig& other);

        /**
         * Move Constructor of #LuaConfig
         * @param other the other #LuaConfig to move from
         */
        RAMSES_API LuaConfig(LuaConfig&& other) noexcept;

        /**
         * Assignment operator of #LuaConfig
         * @param other the other #LuaConfig to copy from
         * @return self
         */
        RAMSES_API LuaConfig& operator=(const LuaConfig& other);

        /**
         * Move assignment operator of #LuaConfig
         * @param other the other #LuaConfig to move from
         * @return self
         */
        RAMSES_API LuaConfig& operator=(LuaConfig&& other) noexcept;

        /**
         * Implementation detail of #LuaConfig
         */
        std::unique_ptr<internal::LuaConfigImpl> m_impl;
    };
}
