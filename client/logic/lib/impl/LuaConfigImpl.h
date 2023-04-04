//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/EStandardModule.h"

#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>

namespace rlogic
{
    class LuaModule;
}

namespace rlogic::internal
{
    using ModuleMapping = std::unordered_map<std::string, const LuaModule*>;
    using StandardModules = std::vector<EStandardModule>;

    class LuaConfigImpl
    {
    public:
        bool addDependency(std::string_view aliasName, const LuaModule& moduleInstance);
        bool addStandardModuleDependency(EStandardModule stdModule);
        void enableDebugLogFunctions();

        [[nodiscard]] const ModuleMapping& getModuleMapping() const;
        [[nodiscard]] const StandardModules& getStandardModules() const;
        [[nodiscard]] bool hasDebugLogFunctionsEnabled() const;

    private:
        ModuleMapping m_modulesMapping;
        StandardModules m_stdModules;
        bool m_debugLogFunctionsEnabled = false;
    };
}
