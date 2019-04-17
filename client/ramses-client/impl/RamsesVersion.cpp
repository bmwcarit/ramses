//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesVersion.h"
#include "Collections/String.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "Collections/StringOutputStream.h"
#include "Utils/LogMacros.h"
#include "ramses-sdk-build-config.h"

namespace ramses_internal
{
    namespace RamsesVersion
    {
        void WriteToStream(IOutputStream& stream, const String& versionString, const String& gitHash)
        {
            LOG_INFO(CONTEXT_CLIENT, "RamsesVersion::WriteToStream: Version: " << versionString << " Git Hash: " << gitHash);
            StringOutputStream out;
            out << "[RamsesVersion:" << versionString << "]\n[GitHash:" << gitHash << "]\n";
            stream.write(out.c_str(), out.length());
        }

        static bool ReadUntilNewline(IInputStream& stream, UInt32 readLimit, String& out)
        {
            out.reserve(readLimit);
            for (UInt32 i = 0; i < readLimit; ++i)
            {
                Char character = 0;
                stream.read(&character, 1);
                if (stream.getState() != EStatus_RAMSES_OK)
                {
                    break;
                }
                if (character == '\n')
                {
                    return true;
                }
                out += character;
            }
            return false;
        }

        static bool ExpectString(const String& inputString, UInt& matchIndexInOut, const char* expectedString)
        {
            const UInt expectedLength = PlatformStringUtils::StrLen(expectedString);
            if (expectedLength + matchIndexInOut > inputString.getLength())
                return false;

            for (UInt i = 0; i < expectedLength; ++i)
            {
                if (inputString[matchIndexInOut] != expectedString[i])
                    return false;
                ++matchIndexInOut;
            }
            return true;
        }

        static bool ExpectAndGetNumber(const String& inputString, UInt& matchIndexInOut, UInt32& numberOut)
        {
            const char* startptr = inputString.data() + matchIndexInOut;
            char* endptr = nullptr;
            const long readNumber = std::strtol(startptr, &endptr, 10);
            if (startptr == endptr || readNumber < 0)
                return false;
            matchIndexInOut += (endptr - startptr);
            numberOut = static_cast<UInt32>(readNumber);
            return true;
        }

        static bool ExpectHexString(const String& inputString, UInt& matchIndexInOut)
        {
            const UInt startIdx = matchIndexInOut;
            while (matchIndexInOut < inputString.getLength() && std::isxdigit(inputString[matchIndexInOut]))
                ++matchIndexInOut;
            return startIdx != matchIndexInOut;
        }

        static bool ExpectRamsesVersion(const String& versionString, VersionInfo& outVersion)
        {
            UInt idx = 0;
            if (!ExpectString(versionString, idx, "[RamsesVersion:"))
                return false;
            const UInt versionStart = idx;
            if (!ExpectAndGetNumber(versionString, idx, outVersion.major) ||
                !ExpectString(versionString, idx, ".") ||
                !ExpectAndGetNumber(versionString, idx, outVersion.minor) ||
                !ExpectString(versionString, idx, "."))
                return false;

            const UInt trailingVersionStart = idx;
            while (idx < versionString.getLength() && versionString[idx] != ']')
                ++idx;
            if (idx == trailingVersionStart)  // may not be empty
                return false;
            outVersion.versionString = versionString.substr(versionStart, idx - versionStart);

            if (!ExpectString(versionString, idx, "]") ||
                idx != versionString.getLength())
                return false;

            return true;
        }

        static bool ExpectGitHash(const String& gitHashString, VersionInfo& outVersion)
        {
            UInt idx = 0;
            if (!ExpectString(gitHashString, idx, "[GitHash:"))
                return false;
            const UInt hashStart = idx;
            if (!ExpectString(gitHashString, idx, "(unknown)") &&
                !ExpectHexString(gitHashString, idx))
                return false;
            outVersion.gitHash = gitHashString.substr(hashStart, idx - hashStart);

            if (!ExpectString(gitHashString, idx, "]") ||
                idx != gitHashString.getLength())
                return false;

            return true;
        }

        bool ReadFromStream(IInputStream& stream, VersionInfo& outVersion)
        {
            String versionString;
            String gitHashString;
            if (!ReadUntilNewline(stream, 100, versionString) ||
                !ReadUntilNewline(stream, 100, gitHashString))
            {
                LOG_ERROR(CONTEXT_CLIENT, "RamsesVersion::ReadFromStream: Can not read version information, file probably corrupt or invalid");
                return false;
            }

            if (!ExpectRamsesVersion(versionString, outVersion) ||
                !ExpectGitHash(gitHashString, outVersion))
                return false;

            return true;
        }

        bool MatchesMajorMinor(UInt32 currentMajor, UInt32 currentMinor, const VersionInfo& in)
        {
            return (in.major == currentMajor &&
                    in.minor == currentMinor) ||
                (currentMajor == 0 &&
                 currentMinor == 0);
        }
    }
}
