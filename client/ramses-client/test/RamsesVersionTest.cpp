//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesVersion.h"
#include "gtest/gtest.h"
#include "Utils/BinaryOutputStream.h"
#include "Utils/BinaryInputStream.h"
#include "Utils/BinaryFileInputStream.h"
#include "Utils/BinaryFileOutputStream.h"
#include "ramses-sdk-build-config.h"

namespace ramses_internal
{
    class ARamsesVersion : public ::testing::Test
    {
    public:
        static BinaryOutputStream CreateOutputStreamFromString(const String& str)
        {
            BinaryOutputStream out;
            out.write(str.data(), static_cast<UInt32>(str.getLength()));
            return out;
        }

        RamsesVersion::VersionInfo info;
    };

    TEST_F(ARamsesVersion, canParseCurrentVersion)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, ::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_STRING, ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH);
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));

        EXPECT_STREQ(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_STRING, info.versionString.c_str());
        EXPECT_STREQ(::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH, info.gitHash.c_str());
        EXPECT_EQ(static_cast<UInt32>(std::atoi(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MAJOR)), info.major);
        EXPECT_EQ(static_cast<UInt32>(std::atoi(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MINOR)), info.minor);
    }

    TEST_F(ARamsesVersion, canParseCurrentVersionFromFile)
    {
        {
            File fOut("ramsesVersionTest");
            BinaryFileOutputStream out(fOut);
            RamsesVersion::WriteToStream(out, ::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_STRING, ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH);
        }
        File fIn("ramsesVersionTest");
        BinaryFileInputStream in(fIn);
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));

        EXPECT_STREQ(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_STRING, info.versionString.c_str());
        EXPECT_STREQ(::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH, info.gitHash.c_str());
        EXPECT_EQ(static_cast<UInt32>(std::atoi(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MAJOR)), info.major);
        EXPECT_EQ(static_cast<UInt32>(std::atoi(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MINOR)), info.minor);
    }

    TEST_F(ARamsesVersion, canParseReleaseVersion)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "17.0.1", "af98afa8e94");
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));

        EXPECT_STREQ("17.0.1", info.versionString.c_str());
        EXPECT_STREQ("af98afa8e94", info.gitHash.c_str());
        EXPECT_EQ(17u, info.major);
        EXPECT_EQ(0u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseMasterVersion)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "0.0.0-devMaster", "af98afa8e94");
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));

        EXPECT_STREQ("0.0.0-devMaster", info.versionString.c_str());
        EXPECT_STREQ("af98afa8e94", info.gitHash.c_str());
        EXPECT_EQ(0u, info.major);
        EXPECT_EQ(0u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseOtherVersionSuffix)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "0.0.0-releaseCandidate", "af98afa8e94");
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));

        EXPECT_STREQ("0.0.0-releaseCandidate", info.versionString.c_str());
        EXPECT_STREQ("af98afa8e94", info.gitHash.c_str());
        EXPECT_EQ(0u, info.major);
        EXPECT_EQ(0u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseVersionNumbersInCorrectOrder)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "1.2.3-devMaster", "af98afa8e94");
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));

        EXPECT_STREQ("1.2.3-devMaster", info.versionString.c_str());
        EXPECT_STREQ("af98afa8e94", info.gitHash.c_str());
        EXPECT_EQ(1u, info.major);
        EXPECT_EQ(2u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseLongVersionNumbers)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "11.22.333", "af98afa8e94");
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));

        EXPECT_STREQ("11.22.333", info.versionString.c_str());
        EXPECT_STREQ("af98afa8e94", info.gitHash.c_str());
        EXPECT_EQ(11u, info.major);
        EXPECT_EQ(22u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseUnknownAsHashValue)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "1.3.2", "(unknown)");
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));

        EXPECT_STREQ("1.3.2", info.versionString.c_str());
        EXPECT_STREQ("(unknown)", info.gitHash.c_str());
        EXPECT_EQ(1u, info.major);
        EXPECT_EQ(3u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseFullHashValue)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "1.3.2", "ce01334641b90dbfc02cbee5d072cec8f9000afe");
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));

        EXPECT_STREQ("1.3.2", info.versionString.c_str());
        EXPECT_STREQ("ce01334641b90dbfc02cbee5d072cec8f9000afe", info.gitHash.c_str());
    }

    TEST_F(ARamsesVersion, canParseMajorMinorPatchTweak)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "1.3.2.5", "ce01334641b90dbfc02cbee5d072cec8f9000afe");
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));

        EXPECT_STREQ("1.3.2.5", info.versionString.c_str());
        EXPECT_STREQ("ce01334641b90dbfc02cbee5d072cec8f9000afe", info.gitHash.c_str());
        EXPECT_EQ(1u, info.major);
        EXPECT_EQ(3u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseDifferentSuffix)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "1.3.2-foobar", "ce01334641b90dbfc02cbee5d072cec8f9000afe");
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));

        EXPECT_STREQ("1.3.2-foobar", info.versionString.c_str());
        EXPECT_STREQ("ce01334641b90dbfc02cbee5d072cec8f9000afe", info.gitHash.c_str());
        EXPECT_EQ(1u, info.major);
        EXPECT_EQ(3u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseRandomStringAfterMajorMinor)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "1.3.a.b.c+hotfix3", "ce01334641b90dbfc02cbee5d072cec8f9000afe");
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));

        EXPECT_STREQ("1.3.a.b.c+hotfix3", info.versionString.c_str());
        EXPECT_STREQ("ce01334641b90dbfc02cbee5d072cec8f9000afe", info.gitHash.c_str());
        EXPECT_EQ(1u, info.major);
        EXPECT_EQ(3u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseHandCraftedVersionInformation)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHash:d89398fd]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info));
    }

    TEST_F(ARamsesVersion, failsWhenHashEntryKeywordWrong)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHashX:d89398fd]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info));
    }

    TEST_F(ARamsesVersion, failsWhenHashEntryCharactersAreWrong)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHash:d8XX9398fd]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info));
    }

    TEST_F(ARamsesVersion, failsWhenHashMissing)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHash:]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info));
    }

    TEST_F(ARamsesVersion, failsWhenVersionMissing)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:]\n[GitHash:d89398fd]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info));
    }

    TEST_F(ARamsesVersion, failsWhenVersionTruncated)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.]\n[GitHash:d89398fd]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info));
    }

    TEST_F(ARamsesVersion, failsWhenVersionNotNumbers)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.a.0]\n[GitHash:d89398fd]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info));
    }

    TEST_F(ARamsesVersion, failsWhenStreamInvalid)
    {
        File f("this_file_should_not_exist");
        BinaryFileInputStream in(f);
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info));
    }

    TEST_F(ARamsesVersion, failsWhenNoNewlineFoundWithinReasonableDistanceFromStart)
    {
        String s;
        for (int i = 0; i < 1000; ++i)
        {
            s += ' ';
        }
        BinaryOutputStream out(CreateOutputStreamFromString(s));
        RamsesVersion::WriteToStream(out, "17.0.1", "af98afa8eeee94");
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info));
    }

    TEST_F(ARamsesVersion, failsWhenStreamNotLongEnoughToMatchVersionIndicator)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersi\n[GitHash:d89398fd]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info));
    }

    TEST_F(ARamsesVersion, failsWhenVersionNumberNegative)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:1.-1.0]\n[GitHash:d89398fd]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info));
    }

    TEST_F(ARamsesVersion, MatchesCurrentMajorMinorMatchesCurrentExactVersion)
    {
        RamsesVersion::VersionInfo vi;
        vi.major = ::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MAJOR_INT;
        vi.minor = ::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MINOR_INT;
        EXPECT_TRUE(RamsesVersion::MatchesMajorMinor(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MAJOR_INT, ::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MINOR_INT, vi));
    }

    TEST_F(ARamsesVersion, AcceptsMasterVersionZeroZeroAlways)
    {
        RamsesVersion::VersionInfo vi;
        vi.major = 0;
        vi.minor = 0;
        EXPECT_TRUE(RamsesVersion::MatchesMajorMinor(0, 0, vi));
    }

    TEST_F(ARamsesVersion, AcceptsAnyFileVersionWithMasterZeroZero)
    {
        RamsesVersion::VersionInfo vi;
        vi.major = 1;
        vi.minor = 2;
        EXPECT_TRUE(RamsesVersion::MatchesMajorMinor(0, 0, vi));
    }

    TEST_F(ARamsesVersion, MatchesCurrentMajorMinorFailsIfMajorDiffers)
    {
        RamsesVersion::VersionInfo vi;
        vi.major = 55 + 1;
        vi.minor = 66;
        EXPECT_FALSE(RamsesVersion::MatchesMajorMinor(55, 66, vi));
    }

    TEST_F(ARamsesVersion, MatchesCurrentMajorMinorFailsIfMinorDiffers)
    {
        RamsesVersion::VersionInfo vi;
        vi.major = 55;
        vi.minor = 66 + 1;
        EXPECT_FALSE(RamsesVersion::MatchesMajorMinor(55, 66, vi));
    }
}
