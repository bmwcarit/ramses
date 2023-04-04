//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <string>
#include <string_view>
#include "ramses-logic/ELuaSavingMode.h"

namespace rlogic
{
    class LuaModule;
}

namespace rlogic::internal
{
    class SaveFileConfigImpl
    {
    public:
        void setMetadataString(std::string_view metadata);
        void setExporterVersion(uint32_t major, uint32_t minor, uint32_t patch, uint32_t fileFormatVersion);
        void setValidationEnabled(bool validationEnabled);
        void setLuaSavingMode(ELuaSavingMode mode);

        [[nodiscard]] const std::string& getMetadataString() const;
        [[nodiscard]] uint32_t getExporterMajorVersion() const;
        [[nodiscard]] uint32_t getExporterMinorVersion() const;
        [[nodiscard]] uint32_t getExporterPatchVersion() const;
        [[nodiscard]] uint32_t getExporterFileFormatVersion() const;
        [[nodiscard]] bool getValidationEnabled() const;
        [[nodiscard]] ELuaSavingMode getLuaSavingMode() const;

    private:
        std::string m_metadata;
        uint32_t m_exporterMajorVersion = 0u;
        uint32_t m_exporterMinorVersion = 0u;
        uint32_t m_exporterPatchVersion = 0u;
        uint32_t m_exporterFileFormatVersion = 0u;
        bool m_validationEnabled = true;
        ELuaSavingMode m_luaSavingMode = ELuaSavingMode::SourceAndByteCode;
    };
}
