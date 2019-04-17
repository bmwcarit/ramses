//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESVERSION_H
#define RAMSES_RAMSESVERSION_H

#include "Collections/String.h"

namespace ramses_internal
{
    class IOutputStream;
    class IInputStream;

    namespace RamsesVersion
    {
        struct VersionInfo
        {
            String gitHash;
            String versionString;
            UInt32 major;
            UInt32 minor;
        };

        void WriteToStream(IOutputStream& stream, const String& versionString, const String& gitHash);
        bool ReadFromStream(IInputStream& stream, VersionInfo& outVersion);
        bool MatchesMajorMinor(UInt32 currentMajor, UInt32 currentMinor, const VersionInfo& in);
    }
}

#endif
