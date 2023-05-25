//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_RAMSESVERSION_H
#define RAMSES_INTERNAL_RAMSESVERSION_H

#include "ramses-framework-api/EFeatureLevel.h"
#include "PlatformAbstraction/PlatformTypes.h"

#include <string>
#include <string_view>

namespace ramses_internal
{
    class IOutputStream;
    class IInputStream;

    namespace RamsesVersion
    {
        struct VersionInfo
        {
            std::string gitHash;
            std::string versionString;
            UInt32 major;
            UInt32 minor;
        };

        void WriteToStream(IOutputStream& stream, std::string_view versionString, std::string_view gitHash, ramses::EFeatureLevel featureLevel);
        bool ReadFromStream(IInputStream& stream, VersionInfo& outVersion, ramses::EFeatureLevel& outFeatureLevel);
        bool MatchesMajorMinor(UInt32 currentMajor, UInt32 currentMinor, const VersionInfo& in);
    }
}

#endif
