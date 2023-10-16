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
#include "ramses/client/logic/ELuaSavingMode.h"
#include "internal/PlatformAbstraction/FmtBase.h"
#include "internal/PlatformAbstraction/Collections/IInputStream.h"
#include "internal/PlatformAbstraction/Collections/IOutputStream.h"

namespace ramses
{
    class LuaModule;
}

namespace ramses::internal
{
    class SaveFileConfigImpl
    {
    public:
        void setMetadataString(std::string_view metadata);
        void setExporterVersion(uint32_t major, uint32_t minor, uint32_t patch, uint32_t fileFormatVersion);
        void setValidationEnabled(bool validationEnabled);
        void setLuaSavingMode(ELuaSavingMode mode);
        void setCompressionEnabled(bool compressionEnabled);

        struct ExporterVersion
        {
            uint32_t major = 0u;
            uint32_t minor = 0u;
            uint32_t patch = 0u;
            uint32_t fileFormat = 0u;
        };

        [[nodiscard]] const std::string& getMetadataString() const;
        [[nodiscard]] const ExporterVersion& getExporterVersion() const;
        [[nodiscard]] bool getValidationEnabled() const;
        [[nodiscard]] ELuaSavingMode getLuaSavingMode() const;
        [[nodiscard]] bool getCompressionEnabled() const;

    private:
        std::string m_metadata;
        ExporterVersion m_exporterVersion;
        bool m_validationEnabled = true;
        bool m_compressionEnabled = false;
        ELuaSavingMode m_luaSavingMode = ELuaSavingMode::SourceAndByteCode;
    };

    inline IOutputStream& operator<<(IOutputStream& os, const SaveFileConfigImpl::ExporterVersion& exporter)
    {
        os << exporter.major << exporter.minor << exporter.patch << exporter.fileFormat;
        return os;
    }

    inline IInputStream& operator>>(IInputStream& is, SaveFileConfigImpl::ExporterVersion& exporter)
    {
        is >> exporter.major >> exporter.minor >> exporter.patch >> exporter.fileFormat;
        return is;
    }
}

template <> struct fmt::formatter<ramses::internal::SaveFileConfigImpl> : public ramses::internal::SimpleFormatterBase
{
    template <typename FormatContext> constexpr auto format(const ramses::internal::SaveFileConfigImpl& c, FormatContext& ctx)
    {
        const auto& exporter = c.getExporterVersion();
        return fmt::format_to(ctx.out(), "'{}' exporter:{}.{}.{}.{} validate:{} compress:{} lua:{}",
                              c.getMetadataString(),
                              exporter.major,
                              exporter.minor,
                              exporter.patch,
                              exporter.fileFormat,
                              c.getValidationEnabled(),
                              c.getCompressionEnabled(),
                              c.getLuaSavingMode()
                              );
    }
};
