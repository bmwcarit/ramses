//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/EFeatureLevel.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace ramses::internal
{
    class IOutputStream;
    class IInputStream;

    namespace RamsesVersion
    {
        struct VersionInfo
        {
            std::string gitHash;
            std::string versionString;
            uint32_t major = 0u;
            uint32_t minor = 0u;
        };

        void WriteToStream(IOutputStream& stream, std::string_view versionString, std::string_view gitHash, EFeatureLevel featureLevel);
        bool ReadFromStream(IInputStream& stream, VersionInfo& outVersion, EFeatureLevel& outFeatureLevel);
    }
}
