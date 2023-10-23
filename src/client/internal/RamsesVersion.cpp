//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RamsesVersion.h"
#include "internal/PlatformAbstraction/Collections/IOutputStream.h"
#include "internal/PlatformAbstraction/Collections/IInputStream.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/PlatformAbstraction/PlatformStringUtils.h"
#include "internal/Core/Utils/LogMacros.h"
#include "ramses-sdk-build-config.h"
#include "impl/EFeatureLevelImpl.h"

#include <cctype>

namespace ramses::internal
{
    namespace RamsesVersion
    {
        void WriteToStream(IOutputStream& stream, std::string_view versionString, std::string_view gitHash, EFeatureLevel featureLevel)
        {
            LOG_INFO(CONTEXT_CLIENT, "RamsesVersion::WriteToStream: Version: " << versionString << " Git Hash: " << gitHash << " Feature Level: " << featureLevel);
            StringOutputStream out;
            out << "[RamsesVersion:" << versionString << "]\n[GitHash:" << gitHash << "]\n[FeatureLevel:" << featureLevel << "]\n";
            stream.write(out.c_str(), out.size());
        }

        static bool ReadUntilNewline(IInputStream& stream, uint32_t readLimit, std::string& out)
        {
            out.reserve(readLimit);
            for (uint32_t i = 0; i < readLimit; ++i)
            {
                char character = 0;
                stream.read(&character, 1);
                if (stream.getState() != EStatus::Ok || character == 0)
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

        static bool ExpectString(const std::string& inputString, size_t& matchIndexInOut, const char* expectedString)
        {
            const size_t expectedLength = std::strlen(expectedString);
            if (expectedLength + matchIndexInOut > inputString.size())
                return false;

            for (size_t i = 0; i < expectedLength; ++i)
            {
                if (inputString[matchIndexInOut] != expectedString[i])
                    return false;
                ++matchIndexInOut;
            }
            return true;
        }

        static bool ExpectAndGetNumber(const std::string& inputString, size_t& matchIndexInOut, uint32_t& numberOut)
        {
            const char* startptr = inputString.data() + matchIndexInOut;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg): false positive
            char* endptr = nullptr;
            const auto readNumber = std::strtol(startptr, &endptr, 10);
            if (startptr == endptr || readNumber < 0)
                return false;
            matchIndexInOut += (endptr - startptr);
            numberOut = static_cast<uint32_t>(readNumber);
            return true;
        }

        static bool ExpectHexString(const std::string& inputString, size_t& matchIndexInOut)
        {
            const size_t startIdx = matchIndexInOut;
            while (matchIndexInOut < inputString.size() && (std::isxdigit(inputString[matchIndexInOut]) != 0))
                ++matchIndexInOut;
            return startIdx != matchIndexInOut;
        }

        static bool ExpectRamsesVersion(const std::string& versionString, VersionInfo& outVersion)
        {
            size_t idx = 0;
            if (!ExpectString(versionString, idx, "[RamsesVersion:"))
                return false;
            const size_t versionStart = idx;
            if (!ExpectAndGetNumber(versionString, idx, outVersion.major) ||
                !ExpectString(versionString, idx, ".") ||
                !ExpectAndGetNumber(versionString, idx, outVersion.minor) ||
                !ExpectString(versionString, idx, "."))
                return false;

            const size_t trailingVersionStart = idx;
            while (idx < versionString.size() && versionString[idx] != ']')
                ++idx;
            if (idx == trailingVersionStart)  // may not be empty
                return false;
            outVersion.versionString = versionString.substr(versionStart, idx - versionStart);

            if (!ExpectString(versionString, idx, "]") ||
                idx != versionString.size())
                return false;

            return true;
        }

        static bool ExpectGitHash(const std::string& gitHashString, VersionInfo& outVersion)
        {
            size_t idx = 0;
            if (!ExpectString(gitHashString, idx, "[GitHash:"))
                return false;
            const size_t hashStart = idx;
            if (!ExpectString(gitHashString, idx, "(unknown)") &&
                !ExpectHexString(gitHashString, idx))
                return false;
            outVersion.gitHash = gitHashString.substr(hashStart, idx - hashStart);

            if (!ExpectString(gitHashString, idx, "]") ||
                idx != gitHashString.size())
                return false;

            return true;
        }

        static bool ExpectFeatureLevel(const std::string& featureLevelString, EFeatureLevel& outFeatureLevel)
        {
            size_t idx = 0;
            if (!ExpectString(featureLevelString, idx, "[FeatureLevel:"))
                return false;

            uint32_t num = 0;
            if (!ExpectAndGetNumber(featureLevelString, idx, num))
                return false;

            if (!IsFeatureLevel(num))
            {
                LOG_ERROR(CONTEXT_CLIENT, "RamsesVersion::ReadFromStream: Unknown feature level " << num << " in file, either file corrupt or file exported with future version");
                return false;
            }
            outFeatureLevel = static_cast<EFeatureLevel>(num);

            if (!ExpectString(featureLevelString, idx, "]") || idx != featureLevelString.size())
                return false;

            return true;
        }

        bool ReadFromStream(IInputStream& stream, VersionInfo& outVersion, EFeatureLevel& outFeatureLevel)
        {
            std::string versionString;
            std::string gitHashString;
            std::string featureLevelString;
            if (!ReadUntilNewline(stream, 100, versionString) ||
                !ReadUntilNewline(stream, 100, gitHashString) ||
                !ReadUntilNewline(stream, 100, featureLevelString))
            {
                LOG_ERROR(CONTEXT_CLIENT, "RamsesVersion::ReadFromStream: Can not read version information, file probably corrupt or invalid");
                return false;
            }

            if (!ExpectRamsesVersion(versionString, outVersion) ||
                !ExpectGitHash(gitHashString, outVersion) ||
                !ExpectFeatureLevel(featureLevelString, outFeatureLevel))
                return false;

            return true;
        }

        bool MatchesMajorMinor(uint32_t currentMajor, uint32_t currentMinor, const VersionInfo& in)
        {
            return (in.major == currentMajor &&
                    in.minor == currentMinor) ||
                (currentMajor == 0 &&
                 currentMinor == 0);
        }
    }
}
