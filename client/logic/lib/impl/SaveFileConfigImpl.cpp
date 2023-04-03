//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/SaveFileConfigImpl.h"
#include "impl/LoggerImpl.h"

namespace rlogic::internal
{
    void SaveFileConfigImpl::setMetadataString(std::string_view metadata)
    {
        m_metadata = metadata;
    }

    void SaveFileConfigImpl::setExporterVersion(uint32_t major, uint32_t minor, uint32_t patch, uint32_t fileFormatVersion)
    {
        m_exporterMajorVersion = major;
        m_exporterMinorVersion = minor;
        m_exporterPatchVersion = patch;
        m_exporterFileFormatVersion = fileFormatVersion;
    }

    void SaveFileConfigImpl::setValidationEnabled(bool validationEnabled)
    {
        if (validationEnabled == false)
        {
            LOG_INFO("Validation before saving was disabled during save*() calls! Possible content issues will not yield further warnings.");
        }
        m_validationEnabled = validationEnabled;
    }

    bool SaveFileConfigImpl::getValidationEnabled() const
    {
        return m_validationEnabled;
    }

    uint32_t SaveFileConfigImpl::getExporterMajorVersion() const
    {
        return m_exporterMajorVersion;
    }

    uint32_t SaveFileConfigImpl::getExporterMinorVersion() const
    {
        return m_exporterMinorVersion;
    }

    uint32_t SaveFileConfigImpl::getExporterPatchVersion() const
    {
        return m_exporterPatchVersion;
    }

    uint32_t SaveFileConfigImpl::getExporterFileFormatVersion() const
    {
        return m_exporterFileFormatVersion;
    }

    const std::string& SaveFileConfigImpl::getMetadataString() const
    {
        return m_metadata;
    }

    void SaveFileConfigImpl::setLuaSavingMode(ELuaSavingMode mode)
    {
        m_luaSavingMode = mode;
    }

    ELuaSavingMode SaveFileConfigImpl::getLuaSavingMode() const
    {
        return m_luaSavingMode;
    }
}
