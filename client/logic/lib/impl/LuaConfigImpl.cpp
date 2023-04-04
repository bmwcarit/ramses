//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/LuaConfigImpl.h"
#include "impl/LoggerImpl.h"

#include "internals/LuaCompilationUtils.h"
#include "internals/SolState.h"

namespace rlogic::internal
{
    bool LuaConfigImpl::addDependency(std::string_view aliasName, const LuaModule& moduleInstance)
    {
        if (!LuaCompilationUtils::CheckModuleName(aliasName))
        {
            LOG_ERROR("Failed to add dependency '{}'! The alias name should be a valid Lua label.", aliasName);
            return false;
        }

        if (SolState::IsReservedModuleName(aliasName))
        {
            LOG_ERROR("Failed to add dependency '{}'! The alias collides with a standard library name!", aliasName);
            return false;
        }

        std::string aliasNameStr {aliasName};

        if (m_modulesMapping.cend() != m_modulesMapping.find(aliasNameStr))
        {
            LOG_ERROR("Module dependencies must be uniquely aliased! Alias '{}' is already used!", aliasName);
            return false;
        }

        m_modulesMapping.emplace(std::move(aliasNameStr), &moduleInstance);
        return true;
    }

    const ModuleMapping& LuaConfigImpl::getModuleMapping() const
    {
        return m_modulesMapping;
    }

    bool LuaConfigImpl::addStandardModuleDependency(EStandardModule stdModule)
    {
        if (std::find(m_stdModules.cbegin(), m_stdModules.cend(), stdModule) != m_stdModules.cend())
        {
            LOG_ERROR("Standard module {} already added, can't add twice!", stdModule);
            return false;
        }

        if (stdModule == EStandardModule::All)
        {
            for (EStandardModule m = EStandardModule::Base; m != EStandardModule::All;)
            {
                m_stdModules.push_back(m);
                m = static_cast<EStandardModule>(static_cast<int8_t>(m) + 1);
            }
        }
        else
        {
            m_stdModules.push_back(stdModule);
        }

        return true;
    }

    const StandardModules& LuaConfigImpl::getStandardModules() const
    {
        return m_stdModules;
    }

    void LuaConfigImpl::enableDebugLogFunctions()
    {
        m_debugLogFunctionsEnabled = true;
    }

    bool LuaConfigImpl::hasDebugLogFunctionsEnabled() const
    {
        return m_debugLogFunctionsEnabled;
    }
}
