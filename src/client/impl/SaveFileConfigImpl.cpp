//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/SaveFileConfigImpl.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    void SaveFileConfigImpl::setMetadataString(std::string_view metadata)
    {
        m_metadata = metadata;
    }

    void SaveFileConfigImpl::setExporterVersion(uint32_t major, uint32_t minor, uint32_t patch, uint32_t fileFormatVersion)
    {
        m_exporterVersion.major = major;
        m_exporterVersion.minor = minor;
        m_exporterVersion.patch = patch;
        m_exporterVersion.fileFormat = fileFormatVersion;
    }

    const SaveFileConfigImpl::ExporterVersion& SaveFileConfigImpl::getExporterVersion() const
    {
        return m_exporterVersion;
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

    void SaveFileConfigImpl::setCompressionEnabled(bool compressionEnabled)
    {
        m_compressionEnabled = compressionEnabled;
    }

    bool SaveFileConfigImpl::getCompressionEnabled() const
    {
        return m_compressionEnabled;
    }
}
