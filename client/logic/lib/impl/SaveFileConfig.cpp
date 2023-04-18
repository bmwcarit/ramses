//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/SaveFileConfig.h"

#include "impl/SaveFileConfigImpl.h"

namespace rlogic
{
    SaveFileConfig::SaveFileConfig() noexcept
        : m_impl(std::make_unique<internal::SaveFileConfigImpl>())
    {
    }

    SaveFileConfig::~SaveFileConfig() noexcept = default;

    SaveFileConfig& SaveFileConfig::operator=(const SaveFileConfig& other)
    {
        m_impl = std::make_unique<internal::SaveFileConfigImpl>(*other.m_impl);
        return *this;
    }

    SaveFileConfig::SaveFileConfig(const SaveFileConfig& other)
    {
        *this = other;
    }

    SaveFileConfig::SaveFileConfig(SaveFileConfig&&) noexcept = default;
    SaveFileConfig& SaveFileConfig::operator=(SaveFileConfig&&) noexcept = default;

    void SaveFileConfig::setMetadataString(std::string_view metadata)
    {
        m_impl->setMetadataString(metadata);
    }

    void SaveFileConfig::setExporterVersion(uint32_t major, uint32_t minor, uint32_t patch, uint32_t fileFormatVersion)
    {
        m_impl->setExporterVersion(major, minor, patch, fileFormatVersion);
    }

    void SaveFileConfig::setValidationEnabled(bool validationEnabled)
    {
        m_impl->setValidationEnabled(validationEnabled);
    }

    void SaveFileConfig::setLuaSavingMode(ELuaSavingMode mode)
    {
        m_impl->setLuaSavingMode(mode);
    }
}
