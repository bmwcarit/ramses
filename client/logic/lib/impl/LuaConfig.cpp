//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LuaConfig.h"

#include "impl/LuaConfigImpl.h"

namespace rlogic
{
    LuaConfig::LuaConfig() noexcept
        : m_impl(std::make_unique<internal::LuaConfigImpl>())
    {
    }

    void LuaConfig::enableDebugLogFunctions()
    {
        m_impl->enableDebugLogFunctions();
    }

    LuaConfig::~LuaConfig() noexcept = default;

    LuaConfig& LuaConfig::operator=(const LuaConfig& other)
    {
        m_impl = std::make_unique<internal::LuaConfigImpl>(*other.m_impl);
        return *this;
    }

    LuaConfig::LuaConfig(const LuaConfig& other)
    {
        *this = other;
    }

    LuaConfig::LuaConfig(LuaConfig&&) noexcept = default;
    LuaConfig& LuaConfig::operator=(LuaConfig&&) noexcept = default;

    bool LuaConfig::addDependency(std::string_view aliasName, const LuaModule& moduleInstance)
    {
        return m_impl->addDependency(aliasName, moduleInstance);
    }

    bool LuaConfig::addStandardModuleDependency(EStandardModule stdModule)
    {
        return m_impl->addStandardModuleDependency(stdModule);
    }
}
